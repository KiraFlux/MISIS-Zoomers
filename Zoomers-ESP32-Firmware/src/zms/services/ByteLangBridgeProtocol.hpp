#pragma once

#include <Arduino.h>
#include <bytelang/bridge.hpp>
#include <kf/tools/time/Timer.hpp>

namespace zms {

/// @brief Протокол ByteLang Моста
struct ByteLangBridgeProtocol final {

    /// @brief Ошибка исполнения инструкции моста
    using Error = bytelang::bridge::Error;

    /// @brief Результат исполнения инструкции моста
    using BridgeResult = kf::Result<void, Error>;

    /// @brief Специализация отправителя
    using Sender = bytelang::bridge::Sender<kf::u8>;

    /// @brief Специализация приёмника
    using Receiver = bytelang::bridge::Receiver<kf::u8, 4>;

private:
    /// @brief Экземпляр отправителя для создания инструкций
    Sender sender;

    /// @brief Экземпляр приёмника для обработки приходящих инструкций
    Receiver receiver;

    // / @brief Таймер периода отправки значений энкодеров
    // kf::tools::Timer encoders_diffs_timer{static_cast<kf::Hertz>(5)};

public:
    // Инструкции отправки

    /// @brief 0x00 send_millis() -> u32
    bytelang::bridge::Instruction<Sender::Code> send_millis;

    /// @brief 0x01 (...) -> send_log() -> u8[u8]
    bytelang::bridge::Instruction<Sender::Code, const kf::slice<const char> &> send_log;

    /// @brief 0x02 send_dist_sensors() -> { left: u16, right: u16 }
    bytelang::bridge::Instruction<Sender::Code> send_distances;

    /// @brief 0x03 send_encoder_diff() -> { left: i8, right: i8 }
    bytelang::bridge::Instruction<Sender::Code> send_encoders_diffs;

    /// @brief Публичный конструктор для сервиса
    explicit ByteLangBridgeProtocol() :
        ByteLangBridgeProtocol{Serial} {}

    /// @brief Прокрутка событий (Обработка входящих инструкций)
    void poll() {
        receiver.poll();

        // if (encoders_diffs_timer.ready()) {
        //     send_encoders_diffs();
        // }
    }

private:
    /// @brief Приватный конструктор
    /// @param arduino_stream
    explicit ByteLangBridgeProtocol(Stream &arduino_stream) :
        sender{bytelang::core::OutputStream{arduino_stream}},
        receiver{
            .in = bytelang::core::InputStream{arduino_stream},
            .instructions = getInstructions(),
        },

        //

        send_millis{
            sender.createInstruction(
                [](bytelang::core::OutputStream &stream) -> BridgeResult {
                    if (not stream.write(kf::u32(millis()))) {
                        return {Error::InstructionArgumentWriteFail};
                    }

                    return {};
                })},

        //

        send_log{
            sender.createInstruction<const kf::slice<const char> &>(
                [](bytelang::core::OutputStream &stream, const kf::slice<const char> &buffer) -> BridgeResult {
                    if (not stream.write(static_cast<kf::u8>(buffer.size))) {
                        return {Error::InstructionArgumentWriteFail};
                    }

                    if (not stream.write(buffer.ptr, buffer.size)) {
                        return {Error::InstructionArgumentWriteFail};
                    }

                    return {};
                })},

        //

        send_distances{
            sender.createInstruction(
                [](bytelang::core::OutputStream &stream) -> BridgeResult {
                    auto &periphery = Periphery::instance();

                    const auto left = kf::u16(periphery.left_distance_sensor.read());
                    if (not stream.write(left)) { return {Error::InstructionArgumentWriteFail}; }

                    const auto right = kf::u16(periphery.right_distance_sensor.read());
                    if (not stream.write(right)) { return {Error::InstructionArgumentWriteFail}; }

                    return {};
                })},

        //

        send_encoders_diffs{
            sender.createInstruction(
                [](bytelang::core::OutputStream &stream) -> BridgeResult {
                    auto &periphery = Periphery::instance();

                    const auto max_delta = 100;

                    static auto last_left{periphery.left_encoder.getPositionTicks()};
                    static auto last_right{periphery.right_encoder.getPositionTicks()};

                    auto delta_left = periphery.left_encoder.getPositionTicks() - last_left;
                    last_left = periphery.left_encoder.getPositionTicks();
                    delta_left = constrain(delta_left, -max_delta, max_delta);

                    auto delta_right = periphery.right_encoder.getPositionTicks() - last_right;
                    last_right = periphery.right_encoder.getPositionTicks();
                    delta_right = constrain(delta_right, -max_delta, max_delta);

                    if (not stream.write(static_cast<kf::i8>(delta_left))) {
                        return {Error::InstructionArgumentWriteFail};
                    }

                    if (not stream.write(static_cast<kf::i8>(delta_right))) {
                        return {Error::InstructionArgumentWriteFail};
                    }

                    return {};
                })}
    //
    {}

    /// @brief Получить таблицу инструкций приёма
    /// @return Таблица инструкций на приём
    Receiver::InstructionTable getInstructions() {
        return {
            // 0x00
            // get_millis()
            // Вызывает процедуру отправки бортового времени в миллисекундах
            [this](bytelang::core::InputStream &stream) -> BridgeResult {
                return send_millis();
            },

            // 0x01
            // set_manipulator(arm: u8, claw: u8)
            // Устанавливает манипулятор в положение
            // Значение 0xff выключает ось
            [](bytelang::core::InputStream &stream) -> BridgeResult {
                auto arm = stream.readByte();
                if (not arm.hasValue()) { return Error::InstructionArgumentReadFail; }

                auto claw = stream.readByte();
                if (not claw.hasValue()) { return Error::InstructionArgumentReadFail; }

                const auto disabled = 0xFF;

                auto &manipulator = Periphery::instance().manipulator;

                if (disabled == arm.value()) {
                    manipulator.disableArm();
                } else {
                    manipulator.setArm(arm.value());
                }

                if (disabled == claw.value()) {
                    manipulator.disableClaw();
                } else {
                    manipulator.setClaw(claw.value());
                }

                return {};
            },

            // 0x02
            // get_distances()
            // Запросить расстояния с датчиков
            [this](bytelang::core::InputStream &stream) -> BridgeResult {
                return send_distances();
            },

            // 0x03
            // set_motors(left: i16, right: i16)
            // Установить значения моторов.
            // left, right [-1000, 1000]
            [](bytelang::core::InputStream &stream) -> BridgeResult {
                const auto max_value = 1000.0f;

                auto left_op = stream.read<kf::i16>();
                if (not left_op.hasValue()) { return Error::InstructionArgumentReadFail; }

                auto right_op = stream.read<kf::i16>();
                if (not right_op.hasValue()) { return Error::InstructionArgumentReadFail; }

                auto &periphery = Periphery::instance();
                periphery.left_motor.set(kf::f32(constrain(left_op.value(), -max_value, max_value)) / max_value);
                periphery.right_motor.set(kf::f32(constrain(right_op.value(), -max_value, max_value)) / max_value);

                return {};
            },

            //
        };
    }
};

}// namespace zms
