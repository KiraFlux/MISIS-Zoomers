#pragma once

#include <kf/tui.hpp>
#include <rs/aliases.hpp>

#include "zms/drivers/Motor.hpp"
#include "zms/ui/pages/MainPage.hpp"


namespace zms {

/// @brief Страница настройки мотора
struct MotorTunePage final : kf::tui::Page {

    /// @brief Кнопка установки данного значение ШИМ как порог включения
    kf::tui::Button set_current_pwm_as_dead_zone;

    /// @brief Кнопка вызова процедуры инициализации управляемого мотора
    kf::tui::Button init;

    // Управление ШИМ

    using PwmInput = kf::tui::Labeled<kf::tui::SpinBox<Motor::SignedPwm>>;

    /// @brief Ввод ШИМ
    PwmInput pwm_input;

    /// @brief Значение ШИМ для установки
    PwmInput::Content::Scalar pwm_set{0};

    // Управление значением управления

    using NormalizedInput = kf::tui::Labeled<kf::tui::SpinBox<rs::f32>>;

    /// @brief Ввод нормализованного значения управления мотором
    NormalizedInput normalized_input;

    /// @brief Значение управления для установки
    NormalizedInput::Content::Scalar normalized_value_set{0.0f};

    // Управление настройками драйвера

    using FrequencyInput = kf::tui::Labeled<kf::tui::SpinBox<Motor::PwmSettings::FrequencyScalar>>;

    /// @brief Ввод частоты ШИМ
    FrequencyInput frequency_input;

    /// @brief Направление движения
    kf::tui::ComboBox<Motor::Direction, 2> direction;

    /// @brief Активная реализация драйвера
    kf::tui::ComboBox<Motor::DriverImpl, 2> driver_impl;

    explicit MotorTunePage(
        const char *motor_name,
        Motor &motor,
        Motor::PwmSettings &pwm_settings,
        Motor::DriverSettings &driver_settings
    ) :
        Page{motor_name},

        set_current_pwm_as_dead_zone{
            "Set DeadZone",
            [this, &pwm_settings]() {
                pwm_settings.dead_zone = pwm_set;
            }
        },

        init{
            "Re-Init",
            [&motor]() {
                if (not motor.init()) {
                    kf_Logger_fatal("motor init failed!");
                }
            }
        },
        pwm_input{"PWM", PwmInput::Content{pwm_set}},
        normalized_input{"Norm", NormalizedInput::Content{normalized_value_set, 0.1f}},
        frequency_input{"Hz", FrequencyInput::Content{pwm_settings.ledc_frequency_hz, 1000, FrequencyInput::Content::Mode::ArithmeticPositiveOnly}},

        direction{
            {
                {
                    {"CW", Motor::Direction::CW},
                    {"CCW", Motor::Direction::CCW},
                }
            },
            driver_settings.direction
        },

        driver_impl{
            {
                {
                    {"IArduino", Motor::DriverImpl::IArduino},
                    {"L298N", Motor::DriverImpl::L298nModule},
                }
            },
            driver_settings.impl
        } {
        link(MainPage::instance());

        add(pwm_input);
        add(set_current_pwm_as_dead_zone);

        add(normalized_input);
        add(direction);

        add(init);
        add(frequency_input);
        add(driver_impl);
    }
};

}// namespace zms
