#pragma once

#include <kf/Logger.hpp>
#include <kf/units.hpp>

#include "zms/drivers/PwmPositionServo.hpp"

namespace zms {

/// @brief Двухосевой манипулятор
struct Manipulator2DOF {

    /// @brief Звено манипулятора
    struct Axis {

        /// @brief Настройки звена
        struct Settings {

            /// @brief Настройки сервопривода Звена
            PwmPositionServo::DriverSettings servo;

            /// @brief Минимальный угол оси
            kf::Degrees min_angle;

            /// @brief Максимальный угол оси
            kf::Degrees max_angle;

            /// @brief Значение диапазона валидно
            [[nodiscard]] bool isValid(const PwmPositionServo::PulseSettings &pulse_settings) const {
                kf::Logger::Checker checker{};

                if (min_angle >= max_angle) {
                    checker.error("Max angle < Min angle!");
                }

                if (min_angle < pulse_settings.min_position.angle) {
                    checker.error("min_angle < pulse_settings.min_position.angle");
                }

                if (max_angle < pulse_settings.max_position.angle) {
                    checker.error("max_angle < pulse_settings.max_position.angle");
                }

                return checker.ok();
            }

            [[nodiscard]] kf::Degrees clampAngle(kf::Degrees angle) const {
                return constrain(angle, min_angle, max_angle);
            }
        };

        const Settings &settings;
        PwmPositionServo servo;

        explicit Axis(
            const Settings &axis_settings,
            const PwmPositionServo::PwmSettings &pwm_settings,
            const PwmPositionServo::PulseSettings &pulse_settings) : settings{axis_settings}, servo{pwm_settings, axis_settings.servo, pulse_settings} {}

        [[nodiscard]] bool init() {
            return servo.init();
        }

        void set(kf::Degrees angle) {
            servo.set(settings.clampAngle(angle));
        }
    };

    /// @brief Настройки двухосного манипулятора
    struct Settings {

        /// @brief Настройки ШИМ сервопривода
        PwmPositionServo::PwmSettings servo_pwm;

        /// @brief Настройки Pulse сервопривода
        PwmPositionServo::PulseSettings servo_generic_pulse_settings;

        /// @brief Настройки оси захвата
        Axis::Settings claw_axis;

        /// @brief Настройки оси звена
        Axis::Settings arm_axis;

        /// @brief Настройки манипулятора валидны
        [[nodiscard]] bool isValid() const {
            kf::Logger::Checker checker{};

            if (not arm_axis.isValid(servo_generic_pulse_settings)) {
                checker.error("Invalid: arm range");
            }

            if (not claw_axis.isValid(servo_generic_pulse_settings)) {
                checker.error("Invalid: claw range");
            }

            return checker.ok();
        }
    };

private:
    /// @brief Настройки манипулятора
    const Settings &settings;

    /// @brief Привод оси звена
    Axis arm_axis;

    /// @brief Привод оси захвата
    Axis claw_axis;

public:
    explicit Manipulator2DOF(const Settings &settings) : settings{settings} {}

    /// @brief Инициализировать захват
    [[nodiscard]] bool init() {
        if (not arm_axis.init()) {
            kf_Logger_error("arm servo init fail");
            return false;
        }

        if (not claw_axis.init()) {
            kf_Logger_error("arm servo init fail");
            return false;
        }

        return true;
    }
};

}// namespace zms
