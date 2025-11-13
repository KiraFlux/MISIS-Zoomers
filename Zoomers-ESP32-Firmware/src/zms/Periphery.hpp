#pragma once

#include <kf/tools/Storage.hpp>
#include <kf/tools/meta/Singleton.hpp>

#include "zms/drivers/Encoder.hpp"
#include "zms/drivers/EspnowNode.hpp"
#include "zms/drivers/Motor.hpp"
#include "zms/drivers/Sharp.hpp"
#include "zms/drivers/PwmPositionServo.hpp"

/// @brief MISIS-Zoomers
namespace zms {

/// @brief Аппаратное обеспечение робота.
/// Предоставляет доступ к аппаратным модулям робота и их настройкам.
/// Может быть только единственным
struct Periphery final : kf::tools::Singleton<Periphery> {
    friend struct Singleton<Periphery>;

    /// @brief Настройки аппаратного обеспечения
    struct Settings {

        // Драйвер моторов

        /// @brief Настройки ШИМ драйвера моторов
        Motor::PwmSettings motor_pwm;

        /// @brief Настройки драйверов моторов
        Motor::DriverSettings left_motor, right_motor;

        // Servo

        /// @brief Настройки ШИМ сервопривода
        PwmPositionServo::PwmSettings servo_pwm;

        /// @brief Настройки Pulse сервопривода
        PwmPositionServo::PulseSettings servo_generic_pulse_settings;

        /// @brief Настройки сервопривода Захвата
        PwmPositionServo::DriverSettings servo_mg90s;

        /// @brief Настройки сервопривода Захвата
        PwmPositionServo::DriverSettings servo_mg996;

        // Энкодер

        /// @brief Настройки преобразования энкодера
        Encoder::ConversionSettings encoder_conversion;

        /// @brief Настройки подключения энкодеров
        Encoder::PinsSettings left_encoder, right_encoder;

        /// @brief ИК датчики расстояния
        Sharp::Settings left_distance_sensor, right_distance_sensor;

        // Софт

        /// @brief Настройки узла Espnow
        EspnowNode::Settings espnow_node;
    };

    /// @brief Хранилище настроек
    kf::Storage<Settings> storage{"RobotSet", defaultSettings()};

    // Аппаратные компоненты

    // Моторы

    /// @brief Левый мотор
    Motor left_motor{storage.settings.left_motor, storage.settings.motor_pwm};

    /// @brief Правый мотор
    Motor right_motor{storage.settings.right_motor, storage.settings.motor_pwm};

    // Сервоприводы

    /// @brief Сервопривод захвата
    PwmPositionServo servo_mg90s{storage.settings.servo_pwm, storage.settings.servo_mg90s, storage.settings.servo_generic_pulse_settings};

    /// @brief Сервопривод звена
    PwmPositionServo servo_mg996{storage.settings.servo_pwm, storage.settings.servo_mg996, storage.settings.servo_generic_pulse_settings};

    // Энкодеры

    /// @brief Левый Энкодер
    Encoder left_encoder{storage.settings.left_encoder, storage.settings.encoder_conversion};

    /// @brief Правый Энкодер
    Encoder right_encoder{storage.settings.right_encoder, storage.settings.encoder_conversion};

    // Датчики расстояния

    /// @brief Левый датчик расстояния
    Sharp left_distance_sensor{storage.settings.left_distance_sensor};

    /// @brief Правый датчик расстояния
    Sharp right_distance_sensor{storage.settings.right_distance_sensor};

    // Сеть

    /// @brief Узел протокола Espnow
    EspnowNode espnow_node{storage.settings.espnow_node};

    /// @brief Инициализировать всю периферию
    [[nodiscard]] bool init() {
        // Попытка загрузить настройки
        if (not storage.load()) {
            // Не удалось - сохраняем значения по умолчанию
            if (not storage.save()) {
                return false;
            }
        }

        if (not left_motor.init()) { return false; }
        if (not right_motor.init()) { return false; }

        if (not servo_mg90s.init()) { return false; }
        if (not servo_mg996.init()) { return false; }

        if (not left_distance_sensor.init()) { return false; }
        if (not right_distance_sensor.init()) { return false; }

        left_encoder.init();
        right_encoder.init();

        left_encoder.enable();
        right_encoder.enable();

        if (not espnow_node.init()) { return false; }

        return true;
    }

    /// @brief Получить настройки по умолчанию
    /// @return Значения по умолчанию (Из прошивки)
    static const Settings &defaultSettings() {
        static constexpr Settings default_settings{
            .motor_pwm = {
                .ledc_frequency_hz = 20000,
                .dead_zone = 580,// Значение получено экспериментально
                .ledc_resolution_bits = 10,
            },
            .left_motor = {
                .impl = Motor::DriverImpl::IArduino,
                .direction = Motor::Direction::CCW,
                .pin_a = static_cast<rs::u8>(GPIO_NUM_27),
                .pin_b = static_cast<rs::u8>(GPIO_NUM_21),
                .ledc_channel = 0,
            },
            .right_motor = {
                .impl = Motor::DriverImpl::IArduino,
                .direction = Motor::Direction::CW,
                .pin_a = static_cast<rs::u8>(GPIO_NUM_19),
                .pin_b = static_cast<rs::u8>(GPIO_NUM_18),
                .ledc_channel = 1,
            },
            .servo_pwm = {
                .ledc_frequency_hz = 50,
                .ledc_resolution_bits = 10,
            },
            .servo_generic_pulse_settings = {
                .min_position={
                    .pulse=500,
                    .angle=0,
                },
                .max_position={
                    .pulse=2400,
                    .angle=180,
                }
            },
            .servo_mg90s = {
                .signal_pin = 15,
                .ledc_channel = 15,
            },
            .servo_mg996 = {
                .signal_pin = 14,
                .ledc_channel = 14,
            },
            .encoder_conversion = {
                .ticks_in_one_mm = (5000.0f / 2100.0f),
            },
            .left_encoder = {
                .phase_a = static_cast<rs::u8>(GPIO_NUM_32),
                .phase_b = static_cast<rs::u8>(GPIO_NUM_33),
                .edge = Encoder::PinsSettings::Edge::Rising,
            },
            .right_encoder = {
                .phase_a = static_cast<rs::u8>(GPIO_NUM_25),
                .phase_b = static_cast<rs::u8>(GPIO_NUM_26),
                .edge = Encoder::PinsSettings::Edge::Falling,
            },
            .left_distance_sensor = {
                .pin = static_cast<rs::u8>(GPIO_NUM_34),
                .resolution = 10,
            },
            .right_distance_sensor = {
                .pin = static_cast<rs::u8>(GPIO_NUM_35),
                .resolution = 10,
            },
            .espnow_node = {
                .remote_controller_mac = {0x78, 0x1c, 0x3c, 0xa4, 0x96, 0xdc},
            }
        };
        return default_settings;
    }
};

}// namespace zms
