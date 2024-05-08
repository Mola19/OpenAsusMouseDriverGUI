#include <QWidget>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "devicepage.h"

#include "OpenAsusMouseDriver.hpp"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    driver = new OpenAsusMouseDriver();

    std::vector<AsusMouseDriver*> list = driver->getDevices();

    for (unsigned int i = 0; i < list.size(); i++) {
        bool is_wake = list[i]->get_wake_state();

        QWidget* devicePage;

        if (!is_wake) {
            devicePage = new QWidget();
        } else {
            devicePage = new DevicePage(nullptr, list[i]);
        }

        ui->tabWidget->addTab(devicePage, tr(list[i]->name.c_str()));

        ui->tabWidget->setTabEnabled(i, is_wake);
    }
}

MainWindow::~MainWindow()
{
    delete driver;
    delete ui;
}

