#ifndef DEVICEPAGE_H
#define DEVICEPAGE_H

#include <string>

#include <QWidget>
#include <QComboBox>
#include <QPushButton>

#include "OpenAsusMouseDriver.hpp"
#include "statsthread.h"
#include "lightingzone.h"
#include "directzone.h"
#include "cache.h"

namespace Ui {
class DevicePage;
}

class DevicePage : public QWidget
{
    Q_OBJECT

public:
    explicit DevicePage(QWidget *parent = nullptr, AsusMouseDriver* dev = nullptr);
    ~DevicePage();

private slots:
    void profile_changed(int profile);
    void current_profile_reset();
    void all_profile_reset();
    void stats_start();
    void stats_stop();
    void save_profile();
    void load_profile_data();
    void time_to_sleep_changed(int index);
    void warning_at_never_changed(int state);
    void warning_at_value_changed(int value);
    void lighting_type_changed(int index, bool init = false);
    void set_battery_settings();

private:
    Ui::DevicePage* ui;
    AsusMouseDriver* dev;
    StatsThread* stats;
    Cache cache;
    std::vector<LightingZone*> zones;
    std::vector<DirectZone*> direct_zones;
    std::vector<AsusMouseDriver::RGBColor> direct_leds;

    enum Tabs {
        PERFORMANCE,
        LIGHTING,
        KEYS,
        KEY_STATS,
        BATTERY,
        CALIBRATION,
        PROFILE,
        INFORMATION,
    };
};

class ButtonWithData : public QPushButton {
    Q_OBJECT

    public:
        uint8_t data;

        ButtonWithData (std::string str) : QPushButton(str.c_str()) {};
};

#endif // DEVICEPAGE_H
