#pragma once

#include <kf/UI.hpp>
#include <kf/aliases.hpp>

#include "zms/drivers/Motor.hpp"
#include "zms/ui/pages/MainPage.hpp"


namespace zms {

/// @brief Страница настройки мотора
struct MotorTunePage final : kf::UI::Page {

    /// @brief Кнопка установки данного значение ШИМ как порог включения
    kf::UI::Button set_current_pwm_as_dead_zone;

    /// @brief Кнопка вызова процедуры инициализации управляемого мотора
    kf::UI::Button init;

    // Управление ШИМ

    using PwmInput = kf::UI::Labeled<kf::UI::SpinBox<Motor::SignedPwm>>;

    /// @brief Ввод ШИМ
    PwmInput pwm_input;

    /// @brief Значение ШИМ для установки
    PwmInput::Impl::Scalar pwm_set{0};

    // Управление значением управления

    using NormalizedInput = kf::UI::Labeled<kf::UI::SpinBox<kf::f32>>;

    /// @brief Ввод нормализованного значения управления мотором
    NormalizedInput normalized_input;

    /// @brief Значение управления для установки
    NormalizedInput::Impl::Scalar normalized_value_set{0.0f};

    // Управление настройками драйвера

    using FrequencyInput = kf::UI::Labeled<kf::UI::SpinBox<Motor::PwmSettings::FrequencyScalar>>;

    /// @brief Ввод частоты ШИМ
    FrequencyInput frequency_input;

    /// @brief Направление движения
    kf::UI::ComboBox<Motor::Direction, 2> direction;

    /// @brief Активная реализация драйвера
    kf::UI::ComboBox<Motor::DriverImpl, 2> driver_impl;

    explicit MotorTunePage(
        const char *motor_name,
        Motor &motor,
        Motor::PwmSettings &pwm_settings,
        Motor::DriverSettings &driver_settings
    ) :
        Page{motor_name},

        set_current_pwm_as_dead_zone{
            *this,
            "Set DeadZone",
            [this, &pwm_settings]() {
                pwm_settings.dead_zone = pwm_set;
            }
        },

        init{
            *this,
            "Re-Init",
            [&motor]() {
                if (not motor.init()) {
                    kf_Logger_fatal("motor init failed!");
                }
            }
        },
        pwm_input{
            *this,
            "PWM",
            PwmInput::Impl{pwm_set}
        },
        normalized_input{
            *this,
            "Norm",
            NormalizedInput::Impl{
                normalized_value_set, 0.1f
            }
        },
        frequency_input{
            *this,
            "Hz",
            FrequencyInput::Impl{
                pwm_settings.ledc_frequency_hz,
                1000,
                FrequencyInput::Impl::Mode::ArithmeticPositiveOnly
            }
        },

        direction{
            *this,
            {
                {
                    {"CW", Motor::Direction::CW},
                    {"CCW", Motor::Direction::CCW},
                }
            },
            driver_settings.direction
        },

        driver_impl{
            *this,
            {
                {
                    {"IArduino", Motor::DriverImpl::IArduino},
                    {"L298N", Motor::DriverImpl::L298nModule},
                }
            },
            driver_settings.impl
        } {
        link(MainPage::instance());
    }
};

}// namespace zms
