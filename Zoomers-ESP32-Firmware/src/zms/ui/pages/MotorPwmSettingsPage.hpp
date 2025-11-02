#pragma once

#include <kf/tui.hpp>

#include "zms/drivers/Motor.hpp"
#include "zms/ui/pages/MainPage.hpp"


namespace zms {

/// @brief Страница настроек ШИМ
struct MotorPwmSettingsPage final : kf::tui::Page {
    explicit MotorPwmSettingsPage(Motor::PwmSettings &settings) :
        kf::tui::Page{"Motor PWM"} {
        link(MainPage::instance());
    }
};

}// namespace zms
