#pragma once

#include <Arduino.h>
#include <kf/Logger.hpp>
#include <kf/units.hpp>


namespace zms {

struct Servo {
    enum class Direction : kf::u8 {
        CW = 0x00,
        CCW = 0x01,
    };

    struct DriverSettings {
        kf::u8 signal_pin;
        kf::u8 ledc_channel;
        Direction direction;
        kf::u16 min_angle;
        kf::u16 max_angle;
        kf::u16 min_pulse_us;
        kf::u16 max_pulse_us;

        [[nodiscard]] bool isValid() const {
            return !(ledc_channel > 15 || min_angle >= max_angle || min_pulse_us >= max_pulse_us);
        }
    };

    struct PwmSettings {
        kf::u32 ledc_frequency_hz;
        kf::u8 ledc_resolution_bits;

        [[nodiscard]] bool isValid() const {
            return ledc_frequency_hz > 0 && ledc_resolution_bits >= 8 && ledc_resolution_bits <= 16;
        }

        [[nodiscard]] inline kf::u32 maxDuty() const {
            return (1u << ledc_resolution_bits) - 1u;
        }
    };

    const DriverSettings &driver_settings;
    const PwmSettings &pwm_settings;

private:
    kf::u32 max_duty{0};

public:
    explicit constexpr Servo(const DriverSettings &driver_settings, const PwmSettings &pwm_settings) :
        driver_settings{driver_settings}, pwm_settings(pwm_settings) {}

    [[nodiscard]] bool init() {
        if (!driver_settings.isValid() || !pwm_settings.isValid()) {
            kf_Logger_error("Invalid servo settings");
            return false;
        }

        max_duty = pwm_settings.maxDuty();

        if (ledcSetup(driver_settings.ledc_channel, pwm_settings.ledc_frequency_hz, pwm_settings.ledc_resolution_bits) == 0) {
            kf_Logger_error("LEDC setup failed");
            return false;
        }

        ledcAttachPin(driver_settings.signal_pin, driver_settings.ledc_channel);
        setAngle((driver_settings.min_angle + driver_settings.max_angle) / 2);

        return true;
    }

    void setAngle(kf::u16 angle) {
        angle = constrain(angle, driver_settings.min_angle, driver_settings.max_angle);

//        if (driver_settings.direction == Direction::CCW) {
//            angle = driver_settings.max_angle - (angle - driver_settings.min_angle);
//        }

        write(angleToDuty(angle));
    }


    void detach() { ledcWrite(driver_settings.ledc_channel, 0); }

private:
    void write(kf::u32 duty) const {
        ledcWrite(driver_settings.ledc_channel, constrain(duty, 0u, max_duty));
    }

    [[nodiscard]] kf::u32 angleToDuty(kf::u16 angle) const {
        const auto pulse_width = map(
            angle,
            driver_settings.min_angle,
            driver_settings.max_angle,
            driver_settings.min_pulse_us,
            driver_settings.max_pulse_us
        );

        return kf::u32(pulse_width * pwm_settings.ledc_frequency_hz * max_duty) / 1000000;
    }
};

} // namespace zms