#pragma once

#include <Arduino.h>
#include <kf/tools/validation.hpp>
#include <kf/units.hpp>


namespace zms {

/// @brief Драйвер мотора (IArduino Motor Shield)
struct Motor {

    /// @brief Псевдоним типа для значения ШИМ
    using SignedPwm = kf::i16;

    /// @brief Тип драйвера мотора
    enum class DriverImpl : kf::u8 {
        /// @brief Реализация мотор-шилда IArduino
        IArduino = 0x00,

        /// @brief Реализация драйвера моторов на H-Мосте L298nModule
        L298nModule = 0x01,
    };

    /// @brief Определяет направление положительного вращения
    enum class Direction : kf::u8 {
        /// @brief Положительное вращение - по часовой
        CW = 0x00,

        /// @brief Положительное вращение - против часовой
        CCW = 0x01
    };

    /// @brief Настройки драйвера
    struct DriverSettings : kf::tools::Validable<DriverSettings> {

        /// @brief Выбранный драйвер
        DriverImpl impl;

        /// @brief Определение положительного направления вращения
        Direction direction;

        /// @brief IArduino Motor Shield: Пин направления (H-bridge)
        /// @brief L293N Module: IN1 / IN3
        kf::u8 pin_a;

        /// @brief IArduino Motor Shield: Пин скорости (Enable)
        /// @brief L293N Module: IN2 / IN4
        kf::u8 pin_b;

        /// @brief Канал (0 .. 15)
        kf::u8 ledc_channel;

        void check(kf::tools::Validator &validator) const {
            kf_Validator_check(validator, ledc_channel <= 15);
        }
    };

    /// @brief Настройки ШИМ
    struct PwmSettings : kf::tools::Validable<PwmSettings> {
        /// @brief Скаляр для частоты ШИМ
        using FrequencyScalar = kf::u16;

        /// @brief Частота ШИМ Гц
        FrequencyScalar ledc_frequency_hz;

        /// @brief Мёртвая зона ШИМ
        SignedPwm dead_zone;

        /// @brief Разрешение (8 .. 12)
        kf::u8 ledc_resolution_bits;

        /// @brief Рассчитать актуальное максимальное значение ШИМ
        [[nodiscard]] inline SignedPwm maxPwm() const {
            return static_cast<SignedPwm>((1u << ledc_resolution_bits) - 1u);
        }

        void check(kf::tools::Validator &validator) const {
            kf_Validator_check(validator, dead_zone >= 0);
            kf_Validator_check(validator, ledc_resolution_bits >= 8);
            kf_Validator_check(validator, ledc_resolution_bits <= 12);
        }
    };

    /// @brief Настройки драйвера
    const DriverSettings &driver_settings;

    /// @brief Настройки ШИМ
    const PwmSettings &pwm_settings;

private:
    /// @brief Максимальное значение ШИМ
    SignedPwm max_pwm{0};

public:
    explicit constexpr Motor(const DriverSettings &driver_settings, const PwmSettings &pwm_settings) :
        driver_settings{driver_settings}, pwm_settings{pwm_settings} {}

    [[nodiscard]] bool init() {
        max_pwm = pwm_settings.maxPwm();

        pinMode(driver_settings.pin_a, OUTPUT);
        pinMode(driver_settings.pin_b, OUTPUT);

        switch (driver_settings.impl) {
            case DriverImpl::IArduino: {
                kf_Logger_debug("IArduino mode");

                const auto current_frequency = ledcSetup(
                    driver_settings.ledc_channel,
                    pwm_settings.ledc_frequency_hz,
                    pwm_settings.ledc_resolution_bits);

                if (current_frequency == 0) {
                    kf_Logger_error("LEDC setup failed!");
                    return false;
                }

                ledcAttachPin(driver_settings.pin_b, driver_settings.ledc_channel);
            }
                break;

            case DriverImpl::L298nModule: {
                kf_Logger_debug("L293n mode");

                analogWriteFrequency(pwm_settings.ledc_frequency_hz);
                analogWriteResolution(pwm_settings.ledc_resolution_bits);
            }
                break;
        }

        stop();

        kf_Logger_debug("isOk");
        return true;
    }

    /// @brief Установить значение в нормализованной величине
    void set(float value) const {
        write(fromNormalized(value));
    }

    /// @brief Остановить мотор
    inline void stop() const {
        write(0);
    }

    /// @brief Установить значение ШИМ + направление
    /// @param pwm Значение - ШИМ, Знак - направление
    void write(SignedPwm pwm) const {
        pwm = constrain(pwm, -max_pwm, max_pwm);

        switch (driver_settings.impl) {

            case DriverImpl::IArduino: {
                digitalWrite(driver_settings.pin_a, matchDirection(pwm));
                ledcWrite(driver_settings.ledc_channel, std::abs(pwm));
            }
                return;

            case DriverImpl::L298nModule: {
                const bool positive_direction = matchDirection(pwm);
                if (positive_direction) {
                    analogWrite(driver_settings.pin_a, std::abs(pwm));
                    analogWrite(driver_settings.pin_b, 0);
                } else {
                    analogWrite(driver_settings.pin_a, 0);
                    analogWrite(driver_settings.pin_b, std::abs(pwm));
                }
            }
                return;
        }
    }

private:
    [[nodiscard]] inline bool matchDirection(SignedPwm pwm) const {
        const bool positive = pwm > 0;
        return driver_settings.direction == Direction::CW == positive;
    }

    [[nodiscard]] SignedPwm fromNormalized(float value) const {
        constexpr auto normalized_dead_zone = 1e-2f;

        if (std::isnan(value)) { return 0; }

        const auto abs_value = std::abs(constrain(value, -1.0f, +1.0f));
        if (abs_value < normalized_dead_zone) { return 0; }

        const auto ret = int(abs_value * float(max_pwm - pwm_settings.dead_zone)) + pwm_settings.dead_zone;
        return static_cast<SignedPwm>((value > 0.0f) ? ret : -ret);
    }
};

}// namespace zms
