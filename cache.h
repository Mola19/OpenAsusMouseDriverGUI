#ifndef CACHE_H
#define CACHE_H

#include "OpenAsusMouseDriver.hpp"

class Cache
{
public:
    Cache() {};
    AsusMouseDriver::DeviceInfo info;
    AsusMouseDriver::BatteryInfo battery;
    std::vector<AsusMouseDriver::LightingZoneInfo> lighting;
    AsusMouseDriver::LightingZoneInfo lighting_all_zone;
    AsusMouseDriver::LightingZoneInfo dock_lighting;
};

#endif // CACHE_H
