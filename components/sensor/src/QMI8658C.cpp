#include "QMI8658C.hpp"
#include "esp_err.h"
#include "esp_task.h"
#include "esp_log.h"
#include "cmath"
#include "cstring"
#include "i2c.hpp"
#include <mutex>
#define QMI8658C_LOG_TAG "QMI8658C"
#define QMI8658_SENSOR_ADDR 0x6A
namespace sensor
{
    namespace
    {
        enum class qmi8658_reg : uint8_t
        {
            WHO_AM_I = 0,
            REVISION_ID,
            CTRL1,
            CTRL2,
            CTRL3,
            CTRL4,
            CTRL5,
            CTRL6,
            CTRL7,
            CTRL8,
            CTRL9,
            CATL1_L,
            CATL1_H,
            CATL2_L,
            CATL2_H,
            CATL3_L,
            CATL3_H,
            CATL4_L,
            CATL4_H,
            FIFO_WTM_TH,
            FIFO_CTRL,
            FIFO_SMPL_CNT,
            FIFO_STATUS,
            FIFO_DATA,
            I2CM_STATUS = 44,
            STATUSINT,
            STATUS0,
            STATUS1,
            TIMESTAMP_LOW,
            TIMESTAMP_MID,
            TIMESTAMP_HIGH,
            TEMP_L,
            TEMP_H,
            AX_L,
            AX_H,
            AY_L,
            AY_H,
            AZ_L,
            AZ_H,
            GX_L,
            GX_H,
            GY_L,
            GY_H,
            GZ_L,
            GZ_H,
            MX_L,
            MX_H,
            MY_L,
            MY_H,
            MZ_L,
            MZ_H,
            dQW_L = 73,
            dQW_H,
            dQX_L,
            dQX_H,
            dQY_L,
            dQY_H,
            dQZ_L,
            dQZ_H,
            dVX_L,
            dVX_H,
            dVY_L,
            dVY_H,
            dVZ_L,
            dVZ_H,
            AE_REG1,
            AE_REG2,
            RESET = 96
        };

        struct QMI8658C_data
        {
            int16_t acc_y;
            int16_t acc_x;
            int16_t acc_z;
            int16_t gyr_y;
            int16_t gyr_x;
            int16_t gyr_z;
        };

        QMI8658C::angle fetch_angleFromAcc(const QMI8658C_data &data)
        {
            float temp, angleX, angleY, angleZ;

            // 根据寄存器值 计算倾角值 并把弧度转换成角度
            temp = (float)data.acc_x / sqrt(((float)data.acc_y * (float)data.acc_y + (float)data.acc_z * (float)data.acc_z));
            angleX = atan(temp) * 57.29578f; // 180/π=57.29578
            temp = (float)data.acc_y / sqrt(((float)data.acc_x * (float)data.acc_x + (float)data.acc_z * (float)data.acc_z));
            angleY = atan(temp) * 57.29578f; // 180/π=57.29578
            temp = sqrt(((float)data.acc_x * (float)data.acc_x + (float)data.acc_y * (float)data.acc_y)) / (float)data.acc_z;
            angleZ = atan(temp) * 57.29578f; // 180/π=57.29578
            return {angleX, angleY, angleZ};
        }
        std::once_flag init_flag;
    }

    QMI8658C::QMI8658C()
    {
        hardware::i2c_master i2c;
        uint8_t id = 0;
        i2c.register_read(QMI8658_SENSOR_ADDR, static_cast<uint8_t>(qmi8658_reg::WHO_AM_I), &id, 1);
        if (id != 0x05)
        {
            vTaskDelay(pdMS_TO_TICKS(1000));
            i2c.register_read(QMI8658_SENSOR_ADDR, static_cast<uint8_t>(qmi8658_reg::WHO_AM_I), &id, 1);
        }

        ESP_LOGI(QMI8658C_LOG_TAG, "QMI8658C id %d", id);
        std::call_once(init_flag, [&]()
                       {
                           i2c.register_write(QMI8658_SENSOR_ADDR, static_cast<uint8_t>(qmi8658_reg::RESET), 0xb0);
                           vTaskDelay(pdMS_TO_TICKS(20));
                           i2c.register_write(QMI8658_SENSOR_ADDR, static_cast<uint8_t>(qmi8658_reg::CTRL1), 0x40); // CTRL1 设置地址自动增加
                           i2c.register_write(QMI8658_SENSOR_ADDR, static_cast<uint8_t>(qmi8658_reg::CTRL7), 0x03); // CTRL7 允许加速度和陀螺仪
                           i2c.register_write(QMI8658_SENSOR_ADDR, static_cast<uint8_t>(qmi8658_reg::CTRL2), 0x95); // CTRL2 设置ACC 4g 250Hz
                           i2c.register_write(QMI8658_SENSOR_ADDR, static_cast<uint8_t>(qmi8658_reg::CTRL3), 0xd5); // CTRL3 设置GRY 512dps 250Hz
                       });
    }

    QMI8658C::angle QMI8658C::read()
    {
        QMI8658C_data data;
        hardware::i2c_master i2c;
        uint8_t status = 0;
        i2c.register_read(QMI8658_SENSOR_ADDR, static_cast<uint8_t>(qmi8658_reg::STATUS0), &status, 1); // 读状态寄存器
        ESP_LOGI(QMI8658C_LOG_TAG, "QMI8658C status %d", status);

        if (status & 0x03)
        {
            int16_t sensor_covert[6] = {0};
            i2c.register_read(QMI8658_SENSOR_ADDR, static_cast<uint8_t>(qmi8658_reg::AX_L), (uint8_t *)sensor_covert, 12);

            data.acc_x = sensor_covert[0];
            data.acc_y = sensor_covert[1];
            data.acc_z = sensor_covert[2];
            data.gyr_x = sensor_covert[3];
            data.gyr_y = sensor_covert[4];
            data.gyr_z = sensor_covert[5];
            return fetch_angleFromAcc(data);
        }
        else
        {
            throw ReadException{};
        }
    }

    QMI8658C::~QMI8658C()
    {
    }
}