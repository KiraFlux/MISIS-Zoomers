#pragma once

#include <Arduino.h>
#include <bytelang/bridge.hpp>

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
    using Receiver = bytelang::bridge::Receiver<kf::u8, 2>;

private:
    /// @brief Экземпляр отправителя для создания инструкций
    Sender sender;

    /// @brief Экземпляр приёмника для обработки приходящих инструкций
    Receiver receiver;

public:
    // Инструкции отправки

    /// @brief 0x00 send_millis() -> u32
    bytelang::bridge::Instruction<Sender::Code> send_millis;

    /// @brief 0x01 (...) -> send_log() -> u8[u8]
    bytelang::bridge::Instruction<Sender::Code, const kf::slice<const char> &> send_log;

    /// @brief Публичный конструктор для сервиса
    explicit ByteLangBridgeProtocol() :
        ByteLangBridgeProtocol{Serial} {}

    /// @brief Прокрутка событий (Обработка входящих инструкций)
    void poll() {
        receiver.poll();
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
        send_millis{
            sender.createInstruction(
                [](bytelang::core::OutputStream &stream) -> BridgeResult {
                    if (not stream.write(kf::u32(millis()))) {
                        return {Error::InstructionArgumentWriteFail};
                    }

                    return {};
                })},
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
                })} {}

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
            }

            //
        };
    }
};

}// namespace zms
