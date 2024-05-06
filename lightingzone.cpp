#include <iostream>
#include <string>

#include <QColor>
#include <QColorDialog>
#include <QStyleFactory>

#include "lightingzone.h"
#include "ui_lightingzone.h"

#include "OpenAsusMouseDriver.hpp"

LightingZone::LightingZone(QWidget *parent, AsusMouseDriver* dev, AsusMouseDriver::LightingZones zone_type, AsusMouseDriver::LightingZoneInfo* current_lighting) :
    QWidget(parent),
    ui(new Ui::LightingZone)
{
    ui->setupUi(this);

    this->dev = dev;

    ui->ZoneName->setText(AsusMouseDriver::lighting_zone_names[zone_type].c_str());

    for (auto const& [mode_id, mode] : dev->config.lighting_modes) {
        if(zone_type != AsusMouseDriver::LIGHTING_ZONE_ALL && (
            mode == AsusMouseDriver::LIGHTING_MODE_WAVE ||
            mode == AsusMouseDriver::LIGHTING_MODE_COMET)) continue;

        ui->ModeDropdown->addItem(AsusMouseDriver::lighting_mode_names[mode].c_str());
        ui->ModeDropdown->setItemData(ui->ModeDropdown->count() - 1, mode_id);
    }

    int8_t current_mode_index = ui->ModeDropdown->findData(current_lighting->mode_raw);
    if (current_mode_index == -1) current_mode_index = 0;
    ui->ModeDropdown->setCurrentIndex(current_mode_index);

    connect(ui->ModeDropdown, SIGNAL(currentIndexChanged(int)),
            this, SLOT(mode_changed(int)));

    ui->BrightnessSlider->setMinimum(0);
    ui->BrightnessSlider->setMaximum(dev->config.max_brightness);
    ui->BrightnessSlider->setValue(current_lighting->brightness);
    ui->BrightnessSlider->setSingleStep(1);
    ui->BrightnessSlider->setEnabled(current_lighting->mode == AsusMouseDriver::LIGHTING_MODE_OFF);

    // with windowsvista style the button won't work
    ui->ColorButton->setStyle(QStyleFactory::create("Windows"));
    if (current_lighting->mode == AsusMouseDriver::LIGHTING_MODE_COLOR_CYCLE
        || current_lighting->mode == AsusMouseDriver::LIGHTING_MODE_WAVE
        || current_lighting->mode == AsusMouseDriver::LIGHTING_MODE_BATTERY) {
        ui->ColorButton->setEnabled(0);
        ui->ColorButton->setVisible(0);
    } else {
        current.setRed(current_lighting->red);
        current.setGreen(current_lighting->green);
        current.setBlue(current_lighting->blue);

        ui->ColorButton->setEnabled(1);
        ui->ColorButton->setVisible(1);

        QPalette pal = ui->ColorButton->palette();
        pal.setColor(QPalette::Button, current);
        ui->ColorButton->setPalette(pal);
    }

    connect(ui->ColorButton, SIGNAL(released()),
            this, SLOT(color_button_clicked()));

    // higher speeds are lower value, but qt can't handle high to low, so it needs to be flipped
    bool is_wave = (current_lighting->mode == AsusMouseDriver::LIGHTING_MODE_WAVE);
    if (is_wave) {
        ui->SpeedSlider->setMinimum(dev->config.reasonable_max_speed);
        ui->SpeedSlider->setMaximum(dev->config.reasonable_min_speed);
        ui->SpeedSlider->setValue(dev->config.reasonable_max_speed + dev->config.reasonable_min_speed - current_lighting->speed);
        ui->SpeedSlider->setSingleStep(1);
        ui->SpeedSlider->setEnabled(1);
    } else {
        ui->SpeedSlider->setEnabled(0);
    }

    connect(ui->SpeedSlider, SIGNAL(sliderReleased()),
            this, SLOT(speed_slider_changed()));

    bool is_comet = (current_lighting->mode == AsusMouseDriver::LIGHTING_MODE_COMET);
    ui->RandomCheckbox->setEnabled(is_comet);
    ui->DirectionDropdown->setEnabled(is_comet || is_wave);
}

LightingZone::~LightingZone()
{
    delete ui;
}

void LightingZone::mode_changed(int mode, bool init) {
    int mode_raw = ui->ModeDropdown->itemData(mode).toInt();
    uint8_t mode_id = dev->config.lighting_modes[mode_raw];

    ui->BrightnessSlider->setEnabled(mode_id == AsusMouseDriver::LIGHTING_MODE_OFF);

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
}

void LightingZone::color_button_clicked() {
    current = QColorDialog::getColor(current, this, "Select Color");
    QPalette pal = ui->ColorButton->palette();
    pal.setColor(QPalette::Button, current);
    ui->ColorButton->setPalette(pal);
}

void LightingZone::speed_slider_changed() {
    std::cout << ui->SpeedSlider->value() << std::endl;
}
