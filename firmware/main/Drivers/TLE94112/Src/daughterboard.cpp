#include "daughterboard.h"

DaughterBoard::DaughterBoard(uint8_t id) : board_id(id) {}

DaughterBoard::~DaughterBoard() {}

void DaughterBoard::delay_us(uint32_t us) {
    uint32_t count = us * (SystemCoreClock / 1000000) / 4;
    while(count--) { __NOP(); }
}

void DaughterBoard::select_board() {
    if(board_id > 4) return;

    HAL_GPIO_WritePin(SEL_0_PORT, SEL_0_PIN, (board_id & 0x01) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(SEL_1_PORT, SEL_1_PIN, (board_id & 0x02) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(SEL_2_PORT, SEL_2_PIN, (board_id & 0x04) ? GPIO_PIN_SET : GPIO_PIN_RESET);

    delay_us(5);
}

void DaughterBoard::transmit_daisy_chain(uint8_t reg_addr) {
    uint16_t spi_frame[7];

    for(int i = 0; i < 7; i++) {
        uint8_t target_ic_index = 6 - i;
        uint8_t data_8bit = drivers[target_ic_index].get_register_data(reg_addr);

        spi_frame[i] = (reg_addr << 8) | data_8bit;
    }

    select_board();

    HAL_GPIO_WritePin(SPI_CS_PORT, SPI_CS_PIN, GPIO_PIN_RESET);
    HAL_SPI_Transmit(&hspi1, (uint8_t*)spi_frame, 7, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(SPI_CS_PORT, SPI_CS_PIN, GPIO_PIN_SET);

    delay_us(2);
}

void DaughterBoard::transmit_daisy_chain(uint8_t reg_addr, uint8_t fixed_data) {
    uint16_t spi_frame[7];
    
    for(int i = 0; i < 7; i++) {
        spi_frame[i] = (reg_addr << 8) | fixed_data;
    }

    select_board();

    HAL_GPIO_WritePin(SPI_CS_PORT, SPI_CS_PIN, GPIO_PIN_RESET);
    HAL_SPI_Transmit(&hspi1, (uint8_t*)spi_frame, 7, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(SPI_CS_PORT, SPI_CS_PIN, GPIO_PIN_SET);

    delay_us(5);
}

void DaughterBoard::init() {
    stop();

    transmit_daisy_chain(PWM_CH_FREQ_CTRL_ADDR, CLEAR_REG_DATA);
    
    transmit_daisy_chain(FW_OL_CTRL_ADDR, FW_OL_CTRL_DATA);
    transmit_daisy_chain(FW_CTRL_ADDR, FW_CTRL_DATA);

    transmit_daisy_chain(PWM1_DC_CTRL_ADDR, PWM1_DC_CTRL_DATA);
    transmit_daisy_chain(PWM2_DC_CTRL_ADDR, PWM2_DC_CTRL_DATA);
    transmit_daisy_chain(PWM3_DC_CTRL_ADDR, PWM3_DC_CTRL_DATA);
}

void DaughterBoard::start() {
    transmit_daisy_chain(PWM_CH_FREQ_CTRL_ADDR, 0b00000011);
    delay_us(2500);
    transmit_daisy_chain(PWM_CH_FREQ_CTRL_ADDR, 0b00001111);
    delay_us(2500);
    transmit_daisy_chain(PWM_CH_FREQ_CTRL_ADDR, 0b00111111);
}

void DaughterBoard::stop() {
    transmit_daisy_chain(PWM_CH_FREQ_CTRL_ADDR, CLEAR_REG_DATA);

    transmit_daisy_chain(HB_MODE_1_CTRL_ADDR, CLEAR_REG_DATA);
    transmit_daisy_chain(HB_MODE_2_CTRL_ADDR, CLEAR_REG_DATA);
    transmit_daisy_chain(HB_MODE_3_CTRL_ADDR, CLEAR_REG_DATA);

    for(int i = 0; i < 7; i++) {
        drivers[i].reset_state();
    }
}

void DaughterBoard::step_motors(uint32_t num_steps, uint32_t speed_delay_ms) {
    start();

    for(uint32_t step_count = 0; step_count < num_steps; step_count++) {
        for(int i = 0; i < 7; i++) {
            drivers[i].calculate_step_data();
        }

        transmit_daisy_chain(HB_ACT_1_CTRL_ADDR);
        transmit_daisy_chain(HB_ACT_2_CTRL_ADDR);
        transmit_daisy_chain(HB_ACT_3_CTRL_ADDR);

        transmit_daisy_chain(HB_MODE_1_CTRL_ADDR);
        transmit_daisy_chain(HB_MODE_2_CTRL_ADDR);
        transmit_daisy_chain(HB_MODE_3_CTRL_ADDR);
        
        HAL_Delay(speed_delay_ms);
    }

    stop();
}