#pragma once

#include <Arduino.h>
#include <kf/units.hpp>
#include <kf/tools/validation.hpp>


/// @brief Обработчик прерывания на основной фазе
static void IRAM_ATTR encoderInterruptHandler(void *);

namespace zms {

/// @brief Энкодер инкрементальный с двумя фазами
struct Encoder {

    /// @brief Псевдоним типа для положения энкодера в отсчётах (ticks)
    using Ticks = kf::i32;

    /// @brief Настройки преобразований
    struct ConversionSettings : kf::tools::Validable<ConversionSettings> {
        /// @brief Сколько отсчётов в одном миллиметре (Должно быть положительным!)
        kf::f32 ticks_in_one_mm;

        /// @brief Перевести из отсчётов в мм
        [[nodiscard]] kf::Millimeters toMillimeters(Ticks ticks) const {
            return kf::Millimeters(ticks) / ticks_in_one_mm;
        }

        /// @brief Перевести из мм в отсчёты
        [[nodiscard]] Ticks toTicks(kf::Millimeters mm) const {
            return Ticks(mm * ticks_in_one_mm);
        }

        void check(kf::tools::Validator &validator) const {
            kf_Validator_check(validator, ticks_in_one_mm > 0);
        }
    };

    /// @brief Настройки пинов
    struct PinsSettings {

        /// @brief Режим вызова прерывания
        enum class Edge : kf::u8 {

            /// @brief Прерывание по нарастанию (LOW -> HIGH)
            Rising = RISING,

            /// @brief Прерывание по спаду (HIGH -> LOW)
            Falling = FALLING
        };

        /// @brief Пин основного сигнала (источник прерывания)
        kf::u8 phase_a;

        /// @brief Пин вторичной фазы (для определения направления)
        kf::u8 phase_b;

        /// @brief Фронт срабатывания прерывания
        Edge edge;
    };

    /// @brief Настройки подключения
    const PinsSettings &pins;

    /// @brief Настройки преобразования
    const ConversionSettings &conversion;

    /// @brief Текущее положение энкодера в отсчётах
    Ticks position{0};

    explicit Encoder(const PinsSettings &pins_settings, const ConversionSettings &conversion_settings) :
        pins{pins_settings}, conversion{conversion_settings} {}

    /// @brief Инициализировать пины энкодера
    void init() {
        pinMode(pins.phase_a, INPUT);
        pinMode(pins.phase_b, INPUT);

        enable();
    }

    /// @brief Разрешить (Подключить) обработку прерываний с основной фазы
    void enable() {
        attachInterruptArg(
            pins.phase_a,
            encoderInterruptHandler,
            static_cast<void *>(this),
            static_cast<int>(pins.edge));
    }

    /// @brief Отключить обработку прерываний
    void disable() const {
        detachInterrupt(pins.phase_a);
    }

    /// @brief Положение энкодера в отчётах
    [[nodiscard]] inline Ticks getPositionTicks() const {
        return position;
    }

    /// @brief Установить положение энкодера в отсчётах
    void setPositionTicks(Ticks new_position) {
        position = new_position;
    }

    /// @brief Положение энкодера в мм
    [[nodiscard]] inline kf::Millimeters getPositionMillimeters() const {
        return conversion.toMillimeters(position);
    }

    /// @brief Установить положение энкодера в мм
    void setPositionMillimeters(kf::Millimeters new_position) {
        position = conversion.toTicks(new_position);
    }
};

}// namespace zms

void encoderInterruptHandler(void *instance) {
    auto &encoder = *static_cast<zms::Encoder *>(instance);

    if (digitalRead(encoder.pins.phase_b)) {
        encoder.position += 1;
    } else {
        encoder.position -= 1;
    }
}