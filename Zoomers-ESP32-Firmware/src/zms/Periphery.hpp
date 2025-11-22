#pragma once

#include <kf/tools/validation.hpp>
#include <kf/tools/meta/Singleton.hpp>
#include <kf/tools/Storage.hpp>
#include <kf/EspNow.hpp>
#include <kf/Option.hpp>

#include "zms/drivers/Encoder.hpp"
#include "zms/drivers/Motor.hpp"
#include "zms/drivers/Sharp.hpp"
#include "zms/drivers/Manipulator2DOF.hpp"

/// @brief MISIS-Zoomers
namespace zms {

/// @brief Аппаратное обеспечение робота.
/// Предоставляет доступ к аппаратным модулям робота и их настройкам.
/// Может быть только единственным
struct Periphery final : kf::tools::Singleton<Periphery> {
    friend struct Singleton<Periphery>;

    /// @brief Настройки аппаратного обеспечения
    struct Settings : kf::tools::Validable<Settings> {

        // Драйвер моторов

        /// @brief Настройки ШИМ драйвера моторов
        Motor::PwmSettings motor_pwm;

        /// @brief Настройки драйверов моторов
        Motor::DriverSettings left_motor, right_motor;

        // 
        /// @brief Настройки двухосного манипулятора
        Manipulator2DOF::Settings manipulator;

        // Энкодер

        /// @brief Настройки преобразования энкодера
        Encoder::ConversionSettings encoder_conversion;

        /// @brief Настройки подключения энкодеров
        Encoder::PinsSettings left_encoder, right_encoder;

        /// @brief ИК датчики расстояния
        Sharp::Settings left_distance_sensor, right_distance_sensor;

        // Софт

        /// @brief Настройки узла Espnow
        kf::EspNow::Mac espnow_mac;

        void check(kf::tools::Validator &validator) const {
            // motors
            kf_Validator_check(validator, motor_pwm.isValid());
            kf_Validator_check(validator, left_motor.isValid());
            kf_Validator_check(validator, right_motor.isValid());

            //
            kf_Validator_check(validator, manipulator.isValid());

            // encoders
            kf_Validator_check(validator, encoder_conversion.isValid());

            // distance sensors
            kf_Validator_check(validator, left_distance_sensor.isValid());
            kf_Validator_check(validator, right_distance_sensor.isValid());
        }
    };

    /// @brief Хранилище настроек
    kf::Storage<Settings> storage{"RobotSet", defaultSettings()};

    // Аппаратные компоненты

    // Моторы

    /// @brief Левый мотор
    Motor left_motor{storage.settings.left_motor, storage.settings.motor_pwm};

    /// @brief Правый мотор
    Motor right_motor{storage.settings.right_motor, storage.settings.motor_pwm};

    // Манипуляторы

    /// @brief Манипулятор
    Manipulator2DOF manipulator{storage.settings.manipulator};

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

    //

    /// @brief Узел протокола Espnow
    kf::Option<kf::EspNow::Peer> espnow_peer{};

    /// @brief Инициализировать всю периферию
    [[nodiscard]] bool init() {
        // Попытка загрузить настройки
        if (not storage.load()) {
            // Не удалось - сохраняем значения по умолчанию
            if (not storage.save()) {
                storage.erase();
                return false;
            }
        }

        if (not storage.settings.isValid()) {
            storage.erase();
            return false;
        }

        if (not manipulator.init()) { return false; }

        if (not left_motor.init()) { return false; }
        if (not right_motor.init()) { return false; }

        if (not left_distance_sensor.init()) { return false; }
        if (not right_distance_sensor.init()) { return false; }

        left_encoder.init();
        right_encoder.init();

        auto peer_init = initEspnowPeer();
        if (peer_init.hasValue()) {
            kf_Logger_error(kf::EspNow::stringFromError(peer_init.value()));
            return false;
        }

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
                .pin_a = static_cast<kf::u8>(GPIO_NUM_27),
                .pin_b = static_cast<kf::u8>(GPIO_NUM_21),
                .ledc_channel = 0,
            },
            .right_motor = {
                .impl = Motor::DriverImpl::IArduino,
                .direction = Motor::Direction::CW,
                .pin_a = static_cast<kf::u8>(GPIO_NUM_19),
                .pin_b = static_cast<kf::u8>(GPIO_NUM_18),
                .ledc_channel = 1,
            },
            .manipulator = {
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
                .claw_axis = {
                    .signal_pin = 15,
                    .ledc_channel = 15,
                    .min_angle = 0,
                    .max_angle = 180,
                },
                .arm_axis = {
                    .signal_pin = 14,
                    .ledc_channel = 14,
                    .min_angle = 90,
                    .max_angle = 180,
                },
            },
            .encoder_conversion = {
                .ticks_in_one_mm = (5000.0f / 2100.0f),
            },
            .left_encoder = {
                .phase_a = static_cast<kf::u8>(GPIO_NUM_32),
                .phase_b = static_cast<kf::u8>(GPIO_NUM_33),
                .edge = Encoder::PinsSettings::Edge::Falling,
            },
            .right_encoder = {
                .phase_a = static_cast<kf::u8>(GPIO_NUM_25),
                .phase_b = static_cast<kf::u8>(GPIO_NUM_26),
                .edge = Encoder::PinsSettings::Edge::Falling,
            },
            .left_distance_sensor = {
                .pin = static_cast<kf::u8>(GPIO_NUM_35),
                .resolution = 10,
            },
            .right_distance_sensor = {
                .pin = static_cast<kf::u8>(GPIO_NUM_34),
                .resolution = 10,
            },
            .espnow_mac = {
                {0x78, 0x1c, 0x3c, 0xa4, 0x96, 0xdc},
            }
        };
        return default_settings;
    }

private:

    [[nodiscard]] kf::Option<kf::EspNow::Error> initEspnowPeer() {
        const auto init_result = kf::EspNow::init();
        if (not init_result.isOk()) { return init_result.error(); }

        const auto peer_result = kf::EspNow::Peer::add(storage.settings.espnow_mac);
        if (not peer_result.isOk()) { return peer_result.error(); }

        espnow_peer = peer_result.ok();

        return {};
    }
};

}// namespace zms
