#pragma once

#include <kf/tui.hpp>

#include "zms/drivers/PwmPositionServo.hpp"
#include "zms/ui/pages/MainPage.hpp"


namespace zms {

struct ServoTunePage : kf::tui::Page {
    kf::Degrees angle{0};

    using AngleInput = kf::tui::SpinBox<kf::Degrees>;

    AngleInput angle_spinbox;
    kf::tui::Button set_button, detach_button;

    explicit ServoTunePage(PwmPositionServo &servo, const char *servo_name) :
        Page{servo_name},
        angle_spinbox{
            angle,
            10,
            AngleInput::Mode::Arithmetic
        },
        set_button{
            "set",
            [this, &servo]() { servo.set(angle); }
        },
        detach_button{
            "detach",
            [&servo]() { servo.detach(); }
        } {

        link(MainPage::instance());
        add(angle_spinbox);
        add(set_button);
        add(detach_button);
    }
};

}