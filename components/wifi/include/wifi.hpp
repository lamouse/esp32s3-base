#ifndef WIFI_HPP
#define WIFI_HPP
#include "string"
namespace device{
    class  wifi
    {
    private:
        ::std::string SSID;
        ::std::string password;
    public:
         wifi(std::string ssid, std::string password);
        ~ wifi();
    };
}

#endif