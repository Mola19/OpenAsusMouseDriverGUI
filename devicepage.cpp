#include <string>
#include <QComboBox>
#include <QGraphicsOpacityEffect>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QPushButton>
#include <iostream>
#include <chrono>
#include <thread>

#include "devicepage.h"
#include "ui_devicepage.h"
#include "lightingzone.h"

#include "OpenAsusMouseDriver.hpp"

DevicePage::DevicePage(QWidget* parent, AsusMouseDriver* dev) :
    QWidget(parent),
    ui(new Ui::DevicePage)
{
    ui->setupUi(this);

    this->dev = dev;

    AsusMouseDriver::DeviceInfo dev_info = dev->get_device_info();
    cache.info = dev_info;

    ui->VersionText->setText(QString::fromStdString(dev_info.version));

    if (dev->config.is_wireless) {
        ui->DongleVersionText->setText(QString::fromStdString(dev_info.dongle_version));
    } else {
        QGraphicsOpacityEffect *opacity_label = new QGraphicsOpacityEffect(ui->DongleVersionText);
        QGraphicsOpacityEffect *opacity_text = new QGraphicsOpacityEffect(ui->DongleVersionLabel);
        opacity_label->setOpacity(0.5);
        opacity_text->setOpacity(0.5);
        ui->DongleVersionLabel->setGraphicsEffect(opacity_label);
        ui->DongleVersionText->setGraphicsEffect(opacity_text);
    }


    QComboBox* profile_select = new QComboBox();

    for (uint8_t i = 0; i < dev->config.profile_amount; i++) {
        char name[11];
        snprintf(name, 11, "Profile %d", i+1);
        profile_select->addItem(name);
        profile_select->setItemData(i, i);
    }

    profile_select->setCurrentIndex(dev_info.active_profile);

    connect(profile_select, SIGNAL(currentIndexChanged(int)),
                this, SLOT(profile_changed(int)));

    ui->tabWidget->setCornerWidget(profile_select, Qt::BottomRightCorner);

    connect(ui->ResetCurrentProfileButton, SIGNAL(released()),
            this, SLOT(current_profile_reset()));

    connect(ui->ResetAllProfilesButton, SIGNAL(released()),
            this, SLOT(all_profile_reset()));

    stats = new StatsThread(ui->StatsLeftClickText, ui->StatsRightClickText, ui->StatsDistanceText, ui->StatsStartButton, ui->StatsStopButton, dev);

    connect(ui->StatsStartButton, SIGNAL(released()),
            this, SLOT(stats_start()));

    connect(ui->StatsStopButton, SIGNAL(released()),
            this, SLOT(stats_stop()));

    ui->StatsStopButton->setDisabled(true);

    connect(ui->SaveProfileButton, SIGNAL(released()),
            this, SLOT(save_profile()));

    load_profile_data();

    // these are below load_profile_data so they don't get triggered when setting up
    connect(ui->TimeToSleepDropdown, SIGNAL(currentIndexChanged(int)),
            this, SLOT(time_to_sleep_changed(int)));

    connect(ui->WarningAtNeverButton, SIGNAL(stateChanged(int)),
            this, SLOT(warning_at_never_changed(int)));

    connect(ui->WarningAtInput, SIGNAL(valueChanged(int)),
            this, SLOT(warning_at_value_changed(int)));

    connect(ui->LightingTypeDropdown, SIGNAL(currentIndexChanged(int)),
            this, SLOT(lighting_type_changed(int)));
}

DevicePage::~DevicePage()
{
    delete ui;
    delete stats;
}

void DevicePage::profile_changed(int profile) {
    dev->set_profile(profile);

    load_profile_data();
}

void DevicePage::current_profile_reset() {
    dev->reset_current_profile();

    // TODO load stuff if button->data == active profile

}

void DevicePage::all_profile_reset() {
    dev->reset_all_profiles();
    // first profile will be automatically selected
    ((QTabWidget*) ui->tabWidget->cornerWidget())->setCurrentIndex(0);

    load_profile_data();
}

void DevicePage::stats_start() {
    stats->start();
}

void DevicePage::stats_stop() {
    stats->stop();
}

void DevicePage::save_profile() {
    dev->save_current_profile();
}

void DevicePage::time_to_sleep_changed(int /*index*/)  {
    set_battery_settings();
}

void DevicePage::warning_at_never_changed(int state) {
    ui->WarningAtInput->setEnabled((state == 2) ? false : true);
    set_battery_settings();
}

void DevicePage::warning_at_value_changed(int /*value*/) {
    set_battery_settings();
}

void DevicePage::lighting_type_changed(int index, bool init) {
    int value = ui->LightingTypeDropdown->itemData(index).toInt();

    for (uint8_t i = zones.size(); i > 0; i--) {
        delete zones[i - 1];
        zones.pop_back();
    }

    for (uint8_t i = direct_zones.size(); i > 0; i--) {
        delete direct_zones[i - 1];
        direct_zones.pop_back();
    }

    if (value == 1) {
        LightingZone* zone = new LightingZone(ui->LightingTab, dev, AsusMouseDriver::LIGHTING_ZONE_ALL, &cache.lighting_all_zone);
        zone->move(0, 40);
        zone->show();
        zones.push_back(zone);

        if (!init) {
            dev->set_lighting(AsusMouseDriver::LIGHTING_ZONE_ALL, &cache.lighting_all_zone);
        }
    } else if (value == 0) {
        for (uint8_t i = 0; i < dev->config.lighting_zones.size(); i++) {
            LightingZone* zone = new LightingZone(ui->LightingTab, dev, dev->config.lighting_zones[i], &cache.lighting[i]);
            zone->move(i * 180, 40);
            zone->show();
            zones.push_back(zone);

            if (!init) {
                dev->set_lighting(dev->config.lighting_zones[i], &cache.lighting[i]);
            }
        }
    } else {
        int i = 0;
        for (auto const& [zone_type, leds] : dev->config.direct_map) {
            DirectZone* zone = new DirectZone(ui->LightingTab, dev, zone_type, &leds, &direct_leds);
            zone->move(10, 30 + i * 60);
            zone->show();
            direct_zones.push_back(zone);
            i++;
        }

        dev->set_direct_lighting(&direct_leds);
    }

    if (dev->config.has_dock) {
        cache.dock_lighting = dev->get_dock_lighting();
        LightingZone* zone = new LightingZone(ui->LightingTab, dev, AsusMouseDriver::LIGHTING_ZONE_DOCK, &cache.dock_lighting);
        zone->move(zones.size() * 180, 40);
        zone->show();
        zones.push_back(zone);
    }
}

void DevicePage::set_battery_settings() {
    uint8_t time_to_sleep_index = ui->TimeToSleepDropdown->currentIndex();
    uint8_t time_to_sleep =
            (time_to_sleep_index == 0) ?
                AsusMouseDriver::TIME_TO_SLEEP_NEVER :
                time_to_sleep_index - 1;

    Qt::CheckState warning_at_never_state = ui->WarningAtNeverButton->checkState();
    uint8_t warning_at = (warning_at_never_state == Qt::CheckState::Checked) ? 0 : ui->WarningAtInput->value();

    dev->set_battery_settings(time_to_sleep, warning_at);
}

void DevicePage::load_profile_data() {
    if (!dev->config.has_battery) {
        ui->tabWidget->setTabEnabled(Tabs::BATTERY, false);
    } else {
        AsusMouseDriver::BatteryInfo bat_info = dev->get_battery_info();
        cache.battery = bat_info;

        char charge[5];
        snprintf(charge, 5, "%d%%", bat_info.battery_charge);
        ui->BatteryChargeText->setText(charge);

        DevicePage::blockSignals(true);
        Qt::CheckState state = (bat_info.warning_at == 0) ? Qt::CheckState::Checked : Qt::CheckState::Unchecked;
        ui->WarningAtNeverButton->setCheckState(state);
        ui->WarningAtInput->setValue((bat_info.warning_at == 0) ? 5 : bat_info.warning_at);
        ui->WarningAtInput->setEnabled((state == 2) ? false : true);

        uint8_t index = (bat_info.time_to_sleep == 255) ? 0 : bat_info.time_to_sleep + 1;
        ui->TimeToSleepDropdown->setCurrentIndex(index);
        DevicePage::blockSignals(false);
    }

    if (!dev->config.has_lighting) {
        ui->tabWidget->setTabEnabled(Tabs::LIGHTING, false);
    } else {
        std::vector<AsusMouseDriver::LightingZoneInfo> lighting = dev->get_lighting_info();
        cache.lighting = lighting;

        cache.lighting_all_zone._is_ok     = true;
        cache.lighting_all_zone.mode       = cache.lighting[0].mode;
        cache.lighting_all_zone.mode_raw   = cache.lighting[0].mode_raw;
        cache.lighting_all_zone.brightness = cache.lighting[0].brightness;
        cache.lighting_all_zone.red        = cache.lighting[0].red;
        cache.lighting_all_zone.green      = cache.lighting[0].green;
        cache.lighting_all_zone.blue       = cache.lighting[0].blue;
        cache.lighting_all_zone.speed      = cache.lighting[0].speed;
        cache.lighting_all_zone.direction  = cache.lighting[0].direction;
        cache.lighting_all_zone.random     = cache.lighting[0].random;

        DevicePage::blockSignals(true);
        for (uint8_t i = ui->LightingTypeDropdown->count(); i > 0; i--) {
            ui->LightingTypeDropdown->removeItem(0);
        }

        ui->LightingTypeDropdown->addItem("Indivdual Zones");
        ui->LightingTypeDropdown->setItemData(0, 0);

        if (dev->config.lighting_zones.size() > 1) {
            ui->LightingTypeDropdown->addItem("Zones Synced");
            ui->LightingTypeDropdown->setItemData(1, 1);
            bool synced = true;


            if (lighting[0].mode != AsusMouseDriver::LIGHTING_MODE_WAVE
            && lighting[0].mode != AsusMouseDriver::LIGHTING_MODE_COMET) {
                for (uint8_t zone = 1; zone < dev->config.lighting_zones.size(); zone++) {
                    bool mode_has_color = (lighting[0].mode == AsusMouseDriver::LIGHTING_MODE_STATIC
                                       ||  lighting[0].mode == AsusMouseDriver::LIGHTING_MODE_BREATHING
                                       ||  lighting[0].mode == AsusMouseDriver::LIGHTING_MODE_REACTIVE
                                       ||  lighting[0].mode == AsusMouseDriver::LIGHTING_MODE_COMET);

                    if (lighting[0].mode != lighting[zone].mode
                    || lighting[0].brightness != lighting[zone].brightness

                    || (mode_has_color && (
                           lighting[0].red != lighting[zone].red
                        || lighting[0].green != lighting[zone].green
                        || lighting[0].blue != lighting[zone].blue))
                    ) {
                        synced = false;
                        break;
                    }
                }
            }

            ui->LightingTypeDropdown->setCurrentIndex((synced) ? 1 : 0);
            lighting_type_changed((synced) ? 1 : 0, true);
        } else {
            ui->LightingTypeDropdown->setCurrentIndex(0);
            lighting_type_changed(0, true);
        }

        if(dev->config.has_direct) {
            direct_leds = *new std::vector<AsusMouseDriver::RGBColor>(dev->config.led_count);
            ui->LightingTypeDropdown->addItem("Per LED");
            ui->LightingTypeDropdown->setItemData(ui->LightingTypeDropdown->count() - 1, 2);
        }

        DevicePage::blockSignals(false);
    }
}
