#include <QPushButton>
#include <QStyleFactory>
#include <QColorDialog>
#include <iostream>
#include "directzone.h"
#include "ui_directzone.h"

#include "OpenAsusMouseDriver.hpp"

DirectZone::DirectZone(QWidget *parent, AsusMouseDriver* dev, uint8_t zone_type, const std::vector<uint8_t>* leds, std::vector<AsusMouseDriver::RGBColor>* colors) :
    QWidget(parent),
    ui(new Ui::DirectZone)
{
    ui->setupUi(this);

    this->dev = dev;
    this->colors = colors;

    ui->ZoneName->setText(AsusMouseDriver::lighting_zone_names[zone_type].c_str());

    uint8_t i = 0;
    for (uint8_t led : *leds) {
        QPushButton* led_button = new QPushButton(this);

        led_button->setStyle(QStyleFactory::create("Windows"));
        AsusMouseDriver::RGBColor curr = colors->at(led);

        QPalette pal = led_button->palette();
        pal.setColor(QPalette::Button, *new QColor(curr.red, curr.green, curr.blue));
        led_button->setPalette(pal);

        led_button->resize(30, 30);
        led_button->move(10 + i * 40, 30);
        led_button->show();

        connect(led_button, &QPushButton::clicked, [=] {
                    color_button_clicked(led, led_button);
        });
        i++;
    }
}


DirectZone::~DirectZone()
{
    delete ui;
}

void DirectZone::color_button_clicked(uint8_t led_id, QPushButton* led_button) {
    AsusMouseDriver::RGBColor curr = colors->at(led_id);
    QColor current = QColorDialog::getColor(*new QColor(curr.red, curr.green, curr.blue), this, "Select Color");

    QPalette pal = led_button->palette();
    pal.setColor(QPalette::Button, current);
    led_button->setPalette(pal);

    colors->at(led_id) = { (uint8_t) current.red(), (uint8_t) current.green(), (uint8_t) current.blue() };


    dev->set_direct_lighting(colors);
}
