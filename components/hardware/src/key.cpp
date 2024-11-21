#include "key.hpp"
#include "driver/gptimer.h"
namespace hardware
{

        void gpio_isr_handle(void *arg)
        {
            auto k = static_cast<hardware::key *>(arg);
            gpio_num_t io_num = k->_gpio;
            auto key_level = gpio_get_level(io_num);
            if(!key_level && !k->_put_down){
                k->_put_down = true;
            }
            if(key_level && k->_put_down && !k->_put_up){
                k->_put_up = true;
            }
        }
        bool gptimer_alarm_cb(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_ctx){
            gpio_isr_handle(user_ctx);
            return true;
        }

    key::key(gpio_num_t gpio) : _gpio(gpio), _put_down(false),_put_up(false)
    {
        gpio_config_t conf = {
            .pin_bit_mask = 1ULL << _gpio,
            .mode = GPIO_MODE_INPUT,
            .pull_up_en = GPIO_PULLUP_ENABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_DISABLE};

        gpio_config(&conf);
        gptimer_config_t timer_conf{
            .clk_src = GPTIMER_CLK_SRC_DEFAULT,
            .direction = GPTIMER_COUNT_UP,
            .resolution_hz = 1000000
        };
        gptimer_handle_t timer_handle{};
        gptimer_new_timer(&timer_conf, &timer_handle);
        gptimer_alarm_config_t timer_alarm_conf{
            .alarm_count = 1000,
            .reload_count = 0,
            .flags {.auto_reload_on_alarm = true}
        };
        gptimer_set_alarm_action(timer_handle, &timer_alarm_conf);
        gptimer_event_callbacks_t timer_callback{
            .on_alarm = gptimer_alarm_cb
        };

        gptimer_register_event_callbacks(timer_handle, &timer_callback,this);
        gptimer_enable(timer_handle);
        gptimer_start(timer_handle);
    }
    key_stat key::get_state(){
        if(_put_down && !_put_up){
            return key_stat::put_down;
        }
        if(_put_up && _put_down){
            _put_down = false;
            _put_up = false;
            return key_stat::put_up;
        }
        return key_stat::no_action;
    }
    key::~key()
    {
    }

}