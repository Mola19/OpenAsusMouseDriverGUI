#ifndef DIRECTZONE_H
#define DIRECTZONE_H

#include <QWidget>
#include <QPushButton>
#include "OpenAsusMouseDriver.hpp"

namespace Ui {
class DirectZone;
}

class DirectZone : public QWidget
{
    Q_OBJECT

public:
    explicit DirectZone(QWidget *parent = nullptr, AsusMouseDriver* dev = nullptr, uint8_t zone_type = AsusMouseDriver::LIGHTING_ZONE_ALL, const std::vector<uint8_t>* leds = nullptr, std::vector<AsusMouseDriver::RGBColor>* colors = nullptr);
    ~DirectZone();

private slots:
    void color_button_clicked(uint8_t led_id, QPushButton* led_button);

private:
    Ui::DirectZone *ui;
    AsusMouseDriver* dev;
    std::vector<AsusMouseDriver::RGBColor>* colors;
};

#endif // DIRECTZONE_H
