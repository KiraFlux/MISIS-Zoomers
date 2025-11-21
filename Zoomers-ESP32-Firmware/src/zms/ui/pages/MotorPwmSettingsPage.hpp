#pragma once

#include <kf/UI.hpp>

#include "zms/drivers/Motor.hpp"
#include "zms/ui/pages/MainPage.hpp"


namespace zms {

/// @brief Страница настроек ШИМ
struct MotorPwmSettingsPage final : kf::UI::Page {
    explicit MotorPwmSettingsPage(Motor::PwmSettings &settings) :
        Page{"Motor PWM"} {
        link(MainPage::instance());
    }
};

}// namespace zms
