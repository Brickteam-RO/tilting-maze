"""Smoke test for the window itself.

Builds the real window against a simulated board and pumps the Tk event loop by hand, so
widget construction, the drain timer, and the message-to-widget path are all executed. It
does not assert anything about appearance - only that a maze arriving on the queue ends
up in the view without raising.

Needs a display. On a Pi that is the desktop session; headless, use xvfb-run.
"""

import gc
import os
import time
import tkinter as tk

import pytest

from pi_comp import protocol, simulator

pytestmark = pytest.mark.skipif(
    not (os.environ.get("DISPLAY") or os.environ.get("WAYLAND_DISPLAY")),
    reason="needs a display; try xvfb-run",
)

#: Generous, because the first maze waits on the oracle subprocess.
READY_TIMEOUT_S = 20.0


@pytest.fixture(autouse=True)
def _collect_tk_garbage_on_the_main_thread():
    """Keep cyclic garbage collection off the simulator thread.

    A Tk object may only be deallocated on the thread that owns the Tcl interpreter, but
    CPython's cyclic collector runs on whichever thread happens to trip the allocation
    threshold. Each test here builds and destroys a whole window, so the simulator thread
    of the *next* test would sooner or later run a collection that frees the previous
    window's widgets - calling into Tcl from the wrong thread, corrupting the heap, and
    aborting the interpreter. Automatic collection is therefore off while a window and
    its worker thread are alive; the explicit collect below runs after the app fixture
    has already joined the worker and destroyed the window, so it is on the main thread
    with nothing else running.
    """
    gc.disable()
    try:
        yield
    finally:
        gc.collect()
        gc.enable()


@pytest.fixture
def app():
    try:
        simulator.find_oracle()
    except simulator.OracleNotFound as exc:
        pytest.skip(str(exc))

    tk = pytest.importorskip("tkinter")
    from pi_comp.gui.app import CompanionApp

    try:
        window = CompanionApp(simulate=True, seed=42)
    except tk.TclError as exc:
        pytest.skip(f"no usable display: {exc}")

    try:
        yield window
    finally:
        window._on_close()


def pump_until(window, predicate, timeout=READY_TIMEOUT_S):
    """Run the Tk event loop until predicate() holds or time runs out."""
    deadline = time.monotonic() + timeout

    while time.monotonic() < deadline:
        window.update()
        if predicate():
            return True
        time.sleep(0.02)

    return False


def test_window_builds_and_receives_a_maze(app):
    assert pump_until(app, lambda: app._last_maze is not None), "no maze reached the window"

    maze = app._last_maze
    assert maze["type"] == protocol.MSG_MAZE
    assert len(maze["walls"]) == protocol.SIDE


def test_maze_view_draws_something(app):
    assert pump_until(app, lambda: app._last_maze is not None)

    # The canvas should hold the grid, the frame, and the start/goal labels.
    items = app.maze_view.find_all()
    assert len(items) >= protocol.CELL_COUNT


def test_status_panel_is_populated(app):
    """The status panel reflects the fields a ``status`` message actually carries."""
    assert pump_until(app, lambda: app._last_maze is not None)
    assert pump_until(app, lambda: app.status._values["state"].cget("text") != "—")
    app.update()

    assert app.status._values["state"].cget("text") in ("solving", "solved", "waiting")
    assert app.status._values["tilt"].cget("text") in ("UP", "DOWN", "LEFT", "RIGHT", "NONE")
    assert app.status._values["button"].cget("text") in ("pressed", "released")
    assert app.status._values["diagnostics"].cget("text") == "OK"


def test_raw_log_receives_lines(app):
    assert pump_until(app, lambda: app._last_maze is not None)
    app.update()

    contents = app.log.text.get("1.0", "end-1c")
    assert '"type":"maze"' in contents


def test_no_lines_are_dropped_from_a_clean_source(app):
    """The simulator emits exactly what the parser expects; nothing should be rejected."""
    assert pump_until(app, lambda: app._last_maze is not None)

    # Let a few status messages through as well.
    pump_until(app, lambda: len(app._recent) > 5, timeout=5.0)

    assert app._dropped == 0


def test_connection_bar_has_no_connect_button_when_simulating(app):
    """--simulate drives itself; a manual CONNECT/DISCONNECT toggle makes no sense here."""
    assert not hasattr(app.connection, "connect_btn")
    assert _find_label_text(app.connection, "SIM FREQ:")


def test_simulated_state_still_reaches_the_state_label(app):
    """Regression test for a real crash: set_state() used to assume connect_btn always
    exists and raised in simulate mode before ever reaching state_label, which would then
    stay stuck on the initial "disconnected" text forever. Tk swallows exceptions raised
    inside callbacks (prints to stderr, does not fail the test), so asserting the label
    actually changed is the only way to catch this - "no exception propagated" is not
    enough.
    """
    assert pump_until(
        app, lambda: "disconnected" not in app.connection.state_label.cget("text")
    ), "state_label never updated - set_state() likely raised before reaching it"

    assert "simulated board" in app.connection.state_label.cget("text")


def _find_label_text(widget: object, text: str) -> bool:
    """Recursively search a widget tree for a Label with this exact text."""
    for child in widget.winfo_children():
        if isinstance(child, tk.Label) and child.cget("text") == text:
            return True
        if _find_label_text(child, text):
            return True
    return False


def test_connection_bar_has_connect_button_and_no_sim_freq_when_not_simulating():
    """Real hardware needs a manual CONNECT/DISCONNECT toggle, but SIM FREQ only makes
    sense when a simulated source is actually driving the ball at a configurable rate.
    """
    tk_mod = pytest.importorskip("tkinter")

    from pi_comp.gui.panels import ConnectionBar

    try:
        root = tk_mod.Tk()
    except tk_mod.TclError as exc:
        pytest.skip(f"no usable display: {exc}")

    try:
        bar = ConnectionBar(root, on_connect=lambda *a: None, on_disconnect=lambda: None)
        assert hasattr(bar, "connect_btn")
        assert not _find_label_text(bar, "SIM FREQ:")
    finally:
        root.destroy()
