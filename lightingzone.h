#ifndef LIGHTINGZONE_H
#define LIGHTINGZONE_H

#include <QWidget>

#include "OpenAsusMouseDriver.hpp"

#include "cache.h"

namespace Ui {
class LightingZone;
}

class LightingZone : public QWidget
{
    Q_OBJECT

public:
    explicit LightingZone(QWidget *parent = nullptr, AsusMouseDriver* dev = nullptr, uint8_t zone_type = AsusMouseDriver::LIGHTING_ZONE_ALL, AsusMouseDriver::LightingZoneInfo* lighting_cache = nullptr);
    ~LightingZone();

private slots:
    void mode_changed(int mode, bool init = false);
    void color_button_clicked();
    void speed_slider_changed(int value);
    void brightnes_slider_changed(int value);
    void random_checkbox_changed(int state);


private:
    Ui::LightingZone *ui;
    AsusMouseDriver* dev;
    QColor current;
    AsusMouseDriver::LightingZoneInfo* lighting_cache;
    uint8_t zone_type;
};

#endif // LIGHTINGZONE_H
