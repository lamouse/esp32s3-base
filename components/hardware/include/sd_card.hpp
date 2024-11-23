#pragma once
#ifndef SD_CARD_HPP
#define SD_CARD_HPP
#include "driver/gpio.h"
#include "sdmmc_cmd.h"


#include <string>

namespace hardware{
    class sd_card
    {
    private:
        sdmmc_card_t *card;
    public:
        sd_card(gpio_num_t clk, gpio_num_t cmd, gpio_num_t d0);
        std::string get_mount_point();
        ~sd_card();
    };

}

#endif // SD_CARD_HPP
