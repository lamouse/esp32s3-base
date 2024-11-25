#pragma once
#include "driver/i2c.h"
#define PCA9557_ADDRESS 0x19

/***********************************************************/
/***************    IO扩展芯片 ↓   *************************/
#define PCA9557_INPUT_PORT              0x00
#define PCA9557_OUTPUT_PORT             0x01
#define PCA9557_POLARITY_INVERSION_PORT 0x02
#define PCA9557_CONFIGURATION_PORT      0x03

#define LCD_CS_GPIO                 BIT(0)    // PCA9557_GPIO_NUM_1
#define PA_EN_GPIO                  BIT(1)    // PCA9557_GPIO_NUM_2
#define DVP_PWDN_GPIO               BIT(2)    // PCA9557_GPIO_NUM_3
/**
 * @brief 使用PCA9557类进行相关操作的时候，有此类保证PCA9557已经初始化完成
 *
 */
namespace hardware{
    class PCA9557
    {
    private:
        /* data */
    public:
        PCA9557(/* args */);
        void set_state(uint8_t gpio_bit, uint8_t level);
        ~PCA9557();
    };

}