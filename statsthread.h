#ifndef STATSTHREAD_H
#define STATSTHREAD_H

#include <stdint.h>
#include <thread>
#include <QLabel>
#include <QPushButton>

#include "OpenAsusMouseDriver.hpp"

class StatsThread {
    public:
        StatsThread (
                QLabel* left_click_label,
                QLabel* right_click_label,
                QLabel* distance_label,
                QPushButton* start_button,
                QPushButton* stop_button,
                AsusMouseDriver* dev
        );
        ~StatsThread ();
        void start();
        void stop();

    private:
        QLabel* left_click_label;
        QLabel* right_click_label;
        QLabel* distance_label;
        QPushButton* start_button;
        QPushButton* stop_button;

        AsusMouseDriver* dev;

        std::thread* thread;

        bool should_stop = false;
        bool running = false;

        uint32_t left_click_count;
        uint32_t right_click_count;
        uint64_t distance_counter;

        void thread_fn ();
};

#endif // STATSTHREAD_H
