#include <thread>
#include <QWidget>
#include <iostream>
#include <chrono>

#include "statsthread.h"
#include "OpenAsusMouseDriver.hpp"

StatsThread::StatsThread (
        QLabel* left_click_label,
        QLabel* right_click_label,
        QLabel* distance_label,
        QPushButton* start_button,
        QPushButton* stop_button,
        AsusMouseDriver* dev
) {
    this->left_click_label = left_click_label;
    this->right_click_label = right_click_label;
    this->distance_label = distance_label;
    this->start_button = start_button;
    this->stop_button = stop_button;
    this->dev = dev;
}

StatsThread::~StatsThread() {
    if (!running) return;

    should_stop = true;
    thread->join();
}

void StatsThread::start() {
    if (running) return;

    left_click_count = 0;
    right_click_count = 0;
    distance_counter = 0;

    should_stop = false;

    dev->enable_key_logging(false, true);
    // flush out old information
    dev->get_key_stats();

    thread = new std::thread(&StatsThread::thread_fn, this);

    start_button->setDisabled(true);
    stop_button->setDisabled(false);

    running = true;
}

void StatsThread::stop() {
    if (!running) return;

    should_stop = true;
    thread->join();

    dev->enable_key_logging(false, false);

    start_button->setDisabled(false);
    stop_button->setDisabled(true);

    running = false;
}

void StatsThread::thread_fn() {
    while (!should_stop) {
        AsusMouseDriver::KeyStats stats = dev->get_key_stats();

        left_click_count += stats.left_button;
        right_click_count += stats.right_button;
        distance_counter += stats.distace_traveled;

        char left_click_text[11];
        char right_click_text[11];
        char distance_text[21];

        snprintf(left_click_text, 11, "%u", left_click_count);
        snprintf(right_click_text, 11, "%u", right_click_count);
        snprintf(distance_text, 11, "%.2f cm", (double) distance_counter / 1600 * 2.54);

        left_click_label->setText(left_click_text);
        right_click_label->setText(right_click_text);;
        distance_label->setText(distance_text);;

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}
