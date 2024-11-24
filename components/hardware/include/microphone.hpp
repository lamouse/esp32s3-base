#pragma once
#ifndef MICROPHONE_HPP
#define MICROPHONE_HPP
#include "driver/i2s_tdm.h"

namespace hardware{
    class microphone
    {
    private:
        i2s_chan_handle_t _i2s_rx_chan;
    public:
        microphone(/* args */);
        ~microphone();
    };



}


#endif // MICROPHONE_HPP
