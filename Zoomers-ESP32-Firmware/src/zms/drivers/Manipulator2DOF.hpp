#pragma once

#include <kf/tools/validation.hpp>
#include <kf/units.hpp>

#include "zms/drivers/PwmPositionServo.hpp"

namespace zms {

/// @brief Двухосевой манипулятор
struct Manipulator2DOF {

    /// @brief Настройки двухосного манипулятора
    struct Settings : kf::tools::Validable<Settings> {

        /// @brief Настройки ШИМ сервопривода
        PwmPositionServo::PwmSettings servo_pwm;

        /// @brief Настройки Pulse сервопривода
        PwmPositionServo::PulseSettings servo_generic_pulse_settings;

        /// @brief Настройки оси захвата
        PwmPositionServo::DriverSettings claw_axis;

        /// @brief Настройки оси звена
        PwmPositionServo::DriverSettings arm_axis;

        void check(kf::tools::Validator &validator) const {
            kf_Validator_check(validator, arm_axis.isValid());
            kf_Validator_check(validator, claw_axis.isValid());
            kf_Validator_check(validator, servo_pwm.isValid());
            kf_Validator_check(validator, servo_generic_pulse_settings.isValid());
        }
    };

private:
    /// @brief Настройки манипулятора
    const Settings &settings;

    /// @brief Привод оси звена
    PwmPositionServo arm_axis;

    /// @brief Привод оси захвата
    PwmPositionServo claw_axis;

public:
    explicit Manipulator2DOF(const Settings &settings) :
        settings{settings},
        arm_axis{settings.servo_pwm, settings.arm_axis, settings.servo_generic_pulse_settings},
        claw_axis{settings.servo_pwm, settings.claw_axis, settings.servo_generic_pulse_settings} {}

    /// @brief Инициализировать захват
    [[nodiscard]] bool init() {
        if (not arm_axis.init()) {
            kf_Logger_error("arm axis fail");
            return false;
        }

        if (not claw_axis.init()) {
            kf_Logger_error("claw axis fail");
            return false;
        }

        return true;
    }

    inline void setArm(kf::Degrees angle) { arm_axis.set(angle); }

    inline void setClaw(kf::Degrees angle) { claw_axis.set(angle); }

    inline void disableArm() { arm_axis.disable(); }

    inline void disableClaw() { claw_axis.disable(); }
};

}// namespace zms
