#include <iostream>
#include <string>

#include <QColor>
#include <QColorDialog>
#include <QStyleFactory>

#include "lightingzone.h"
#include "ui_lightingzone.h"

#include "OpenAsusMouseDriver.hpp"

LightingZone::LightingZone(QWidget *parent, AsusMouseDriver* dev, uint8_t zone_type, AsusMouseDriver::LightingZoneInfo* lighting_cache) :
    QWidget(parent),
    ui(new Ui::LightingZone)
{
    ui->setupUi(this);

    this->dev = dev;
    this->lighting_cache = lighting_cache;
    this->zone_type = zone_type;

    ui->ZoneName->setText(AsusMouseDriver::lighting_zone_names[zone_type].c_str());

    for (auto const& [mode_id, mode] : dev->config.lighting_modes) {
        if(zone_type != AsusMouseDriver::LIGHTING_ZONE_ALL && (
            mode == AsusMouseDriver::LIGHTING_MODE_WAVE ||
            mode == AsusMouseDriver::LIGHTING_MODE_COMET ||
            mode == AsusMouseDriver::LIGHTING_MODE_OFF)) continue;

        ui->ModeDropdown->addItem(AsusMouseDriver::lighting_mode_names[mode].c_str());
        ui->ModeDropdown->setItemData(ui->ModeDropdown->count() - 1, mode_id);
    }

    int8_t current_mode_index = ui->ModeDropdown->findData(lighting_cache->mode_raw);
    if (current_mode_index == -1) current_mode_index = 0;
    ui->ModeDropdown->setCurrentIndex(current_mode_index);

    connect(ui->ModeDropdown, SIGNAL(currentIndexChanged(int)),
            this, SLOT(mode_changed(int)));

    ui->BrightnessSlider->setTracking(false);
    ui->BrightnessSlider->setMinimum(0);
    ui->BrightnessSlider->setMaximum(dev->config.max_brightness);
    ui->BrightnessSlider->setValue(lighting_cache->brightness);
    ui->BrightnessSlider->setSingleStep(1);
    ui->BrightnessSlider->setEnabled(lighting_cache->mode != AsusMouseDriver::LIGHTING_MODE_OFF);

    // with windowsvista style the button won't work
    ui->ColorButton->setStyle(QStyleFactory::create("Windows"));
    if (lighting_cache->mode == AsusMouseDriver::LIGHTING_MODE_COLOR_CYCLE
        || lighting_cache->mode == AsusMouseDriver::LIGHTING_MODE_WAVE
        || lighting_cache->mode == AsusMouseDriver::LIGHTING_MODE_BATTERY) {
        ui->ColorButton->setEnabled(0);
        ui->ColorButton->setVisible(0);
    } else {
        current.setRed(lighting_cache->red);
        current.setGreen(lighting_cache->green);
        current.setBlue(lighting_cache->blue);

        ui->ColorButton->setEnabled(1);
        ui->ColorButton->setVisible(1);

        QPalette pal = ui->ColorButton->palette();
        pal.setColor(QPalette::Button, current);
        ui->ColorButton->setPalette(pal);
    }

    connect(ui->ColorButton, SIGNAL(released()),
            this, SLOT(color_button_clicked()));

    // higher speeds are lower value, but qt can't handle high to low, so it needs to be flipped
    bool is_wave = (lighting_cache->mode == AsusMouseDriver::LIGHTING_MODE_WAVE);
    if (is_wave) {
        ui->SpeedSlider->setTracking(false);
        ui->SpeedSlider->setMinimum(dev->config.reasonable_max_speed);
        ui->SpeedSlider->setMaximum(dev->config.reasonable_min_speed);
        ui->SpeedSlider->setValue(dev->config.reasonable_max_speed + dev->config.reasonable_min_speed - lighting_cache->speed);
        ui->SpeedSlider->setSingleStep(1);
        ui->SpeedSlider->setEnabled(1);
    } else {
        ui->SpeedSlider->setEnabled(0);
    }

    connect(ui->BrightnessSlider, SIGNAL(valueChanged(int)),
            this, SLOT(brightnes_slider_changed(int)));
    connect(ui->SpeedSlider, SIGNAL(valueChanged(int)),
            this, SLOT(speed_slider_changed(int)));

    bool is_comet = (lighting_cache->mode == AsusMouseDriver::LIGHTING_MODE_COMET);
    ui->RandomCheckbox->setEnabled(is_comet);
    ui->DirectionDropdown->setEnabled(is_comet || is_wave);

    connect(ui->RandomCheckbox, SIGNAL(stateChanged(int)),
            this, SLOT(random_checkbox_changed(int)));
}

LightingZone::~LightingZone()
{
    delete ui;
}

void LightingZone::mode_changed(int mode, bool init) {
    int mode_raw = ui->ModeDropdown->itemData(mode).toInt();
    uint8_t mode_id = dev->config.lighting_modes[mode_raw];

    lighting_cache->mode_raw = mode_raw;
    lighting_cache->mode = mode_id;

    ui->BrightnessSlider->setEnabled(mode_id != AsusMouseDriver::LIGHTING_MODE_OFF);

    bool has_color = (mode_id == AsusMouseDriver::LIGHTING_MODE_STATIC
        || mode_id == AsusMouseDriver::LIGHTING_MODE_BREATHING
        || mode_id == AsusMouseDriver::LIGHTING_MODE_REACTIVE
        || mode_id == AsusMouseDriver::LIGHTING_MODE_COMET);

    ui->ColorButton->setEnabled(has_color);
    ui->ColorButton->setVisible(has_color);

    bool is_wave = (mode_id == AsusMouseDriver::LIGHTING_MODE_WAVE);
    if (is_wave) {
        ui->SpeedSlider->setMinimum(dev->config.reasonable_max_speed);
        ui->SpeedSlider->setMaximum(dev->config.reasonable_min_speed);
        ui->SpeedSlider->setValue(dev->config.reasonable_max_speed + dev->config.reasonable_min_speed - dev->config.default_speed);
        ui->SpeedSlider->setSingleStep(1);
        ui->SpeedSlider->setEnabled(1);
    } else {
        ui->SpeedSlider->setEnabled(0);
    }

    bool is_comet = (mode_id == AsusMouseDriver::LIGHTING_MODE_COMET);
    ui->RandomCheckbox->setEnabled(is_comet);
    ui->DirectionDropdown->setEnabled(is_comet || is_wave);

    if(!init) {
        dev->set_lighting(zone_type, lighting_cache);
    }
}

void LightingZone::color_button_clicked() {
    current = QColorDialog::getColor(current, this, "Select Color");
    QPalette pal = ui->ColorButton->palette();
    pal.setColor(QPalette::Button, current);
    ui->ColorButton->setPalette(pal);

    lighting_cache->red = current.red();
    lighting_cache->green = current.green();
    lighting_cache->blue = current.blue();

    dev->set_lighting(zone_type, lighting_cache);
}

void LightingZone::brightnes_slider_changed(int value) {
    lighting_cache->brightness = value;

    dev->set_lighting(zone_type, lighting_cache);
}

void LightingZone::speed_slider_changed(int value) {
    lighting_cache->speed = dev->config.reasonable_max_speed + dev->config.reasonable_min_speed - value;

    dev->set_lighting(zone_type, lighting_cache);
}

void LightingZone::random_checkbox_changed(int state) {
    lighting_cache->random = (state == 2);

    dev->set_lighting(zone_type, lighting_cache);
}
