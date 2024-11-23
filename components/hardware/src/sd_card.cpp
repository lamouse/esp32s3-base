#include "sd_card.hpp"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_vfs_fat.h"
#include "driver/sdmmc_host.h"
#include "stdio.h"
#include "string.h"

static const char *SD_CARD_TAG = "sd card";
#define MOUNT_POINT "/sdcard"
namespace hardware
{
    sd_card::sd_card(gpio_num_t clk, gpio_num_t cmd, gpio_num_t d0)
    {
        esp_vfs_fat_sdmmc_mount_config_t mount_config = {
            .format_if_mount_failed = true,
            .max_files = 5,
            .allocation_unit_size = 16 * 1024};

        const char mount_point[] = MOUNT_POINT;
        ESP_LOGI(SD_CARD_TAG, "Initializing SD card");
        sdmmc_host_t host = SDMMC_HOST_DEFAULT();
        sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();
        slot_config.width = 1;
        slot_config.clk = clk;
        slot_config.cmd = cmd;
        slot_config.d0 = d0;
        slot_config.flags |= SDMMC_SLOT_FLAG_INTERNAL_PULLUP;
        ESP_LOGI(SD_CARD_TAG, "Mounting filesystem");
        ESP_ERROR_CHECK(esp_vfs_fat_sdmmc_mount(mount_point, &host, &slot_config, &mount_config, &card));
        // Card has been initialized, print its properties
        sdmmc_card_print_info(stdout, card);
    }

    std::string sd_card::get_mount_point()
    {
        return MOUNT_POINT;
    }

    sd_card::~sd_card()
    {
        esp_vfs_fat_sdcard_unmount(MOUNT_POINT, card);
    }

}