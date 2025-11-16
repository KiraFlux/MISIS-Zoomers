#pragma once

#include <Arduino.h>
#include <kf/units.hpp>
#include <kf/tools/validation.hpp>


namespace zms {

/// @brief ИК Датчик расстояния Sharp
struct Sharp {

    /// @brief Значение выхода АЦП
    using AnalogValue = kf::u16;

    /// @brief Настройки Sharp
    struct Settings : kf::tools::Validable<Settings> {

        /// @brief Аналоговый пин сенсора
        kf::u8 pin;

        /// @brief Разрешение АЦП
        kf::u8 resolution;

        [[nodiscard]] inline AnalogValue maxValue() const {
            return static_cast<AnalogValue>((1u << resolution) - 1u);
        }

        void check(kf::tools::Validator &validator) const {
            kf_Validator_check(validator, resolution > 0);
            kf_Validator_check(validator, resolution <= 16);
        }
    };

    const Settings &settings;

private:
    AnalogValue max_value{0};

public:
    explicit Sharp(const Settings &settings) :
        settings{settings} {}

    [[nodiscard]] bool init() {
        max_value = settings.maxValue();

        pinMode(settings.pin, INPUT);
        analogReadResolution(settings.resolution);

        return true;
    }

    /// @brief Считать значения датчика в величине АЦП 
    [[nodiscard]] inline AnalogValue readRaw() const {
        return analogRead(settings.pin);
    }

    /// @brief Считать расстояние в миллиметрах
    [[nodiscard]] kf::Millimeters read() const {
        // 65535 / analogRead(a)
        long sum = 0;

        const auto n = 4;

        for (int i = 0; i < n; i += 1) {
            sum += readRaw();
            delay(1);
        }

        return (65535.0F * n) / kf::Millimeters(sum);
    }
};

}// namespace zms
