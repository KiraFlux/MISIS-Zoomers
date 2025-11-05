#pragma once

#include <Arduino.h>
#include <kf/Logger.hpp>
#include <kf/units.hpp>


namespace zms {

struct PwmPositionServo {

    struct PwmSettings {
        kf::u32 ledc_frequency_hz;
        kf::u8 ledc_resolution_bits;

        [[nodiscard]] bool isValid() const {
            return ledc_frequency_hz > 0 and ledc_resolution_bits >= 8 and ledc_resolution_bits <= 16;
        }

        [[nodiscard]] inline kf::u32 maxDuty() const {
            return (1u << ledc_resolution_bits) - 1u;
        }

        [[nodiscard]] kf::u16 dutyFromPulseWidth(kf::Milliseconds pulse_width) const {
            const auto t = kf::u64(pulse_width) * ledc_frequency_hz * maxDuty();
            return kf::u16(t / 1000000u);
        }
    };

    struct DriverSettings {
        kf::u8 signal_pin;
        kf::u8 ledc_channel;

        [[nodiscard]] bool isValid() const {
            return ledc_channel <= 15;
        }
    };

    struct PulseSettings {
        struct Pulse {
            kf::Microseconds pulse;
            kf::Degrees angle;
        };

        Pulse min_position;
        Pulse max_position;

        [[nodiscard]] bool isValid() const {
            if (min_position.pulse >= max_position.pulse) {
                kf_Logger_debug("Pulse min > max");
                return false;
            }

            if (min_position.angle >= max_position.angle) {
                kf_Logger_debug("Angle min > max");
                return false;
            }

            return true;
        }

        [[nodiscard]] kf::Microseconds pulseWidthFromAngle(kf::Degrees angle) const {
            return map(
                constrain(angle, min_position.angle, max_position.angle),
                min_position.angle,
                max_position.angle,
                static_cast<long>(min_position.pulse),
                static_cast<long>(max_position.pulse)
            );
        }
    };

    const PwmSettings &pwm_settings;
    const DriverSettings &driver_settings;
    const PulseSettings &pulse_settings;

public:
    explicit constexpr PwmPositionServo(
        const PwmSettings &pwm_settings,
        const DriverSettings &driver_settings,
        const PulseSettings &pulse_settings
    ) :
        driver_settings{driver_settings}, pwm_settings(pwm_settings), pulse_settings(pulse_settings) {}

    [[nodiscard]] bool init() {
        if (not pulse_settings.isValid()) { return false; }
        if (not pwm_settings.isValid()) { return false; }
        if (not driver_settings.isValid()) { return false; }

        const auto freq = ledcSetup(
            driver_settings.ledc_channel,
            pwm_settings.ledc_frequency_hz,
            pwm_settings.ledc_resolution_bits
        );

        if (freq == 0) {
            kf_Logger_error("LEDC setup failed");
            return false;
        }

        ledcAttachPin(driver_settings.signal_pin, driver_settings.ledc_channel);

        return true;
    }

    void set(kf::Degrees angle) {
        write(pwm_settings.dutyFromPulseWidth(pulse_settings.pulseWidthFromAngle(angle)));
    }

    void detach() { write(0); }

private:
    void write(kf::u16 duty) const {
        ledcWrite(driver_settings.ledc_channel, duty);
    }
};

} // namespace zms