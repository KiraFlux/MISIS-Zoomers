#pragma once

#include <Arduino.h>
#include <bytelang/bridge.hpp>


namespace zms {

/// @brief Протокол ByteLang Моста
struct ByteLangBridgeProtocol final {

    /// @brief Ошибка исполнения инструкции моста
    using BridgeError = bytelang::bridge::Error;

    /// @brief Результат исполнения инструкции моста
    using BridgeResult = kf::Result<void, BridgeError>;

    /// @brief Специализация отправителя
    using Sender = bytelang::bridge::Sender<kf::u8>;

    /// @brief Специализация приёмника
    using Receiver = bytelang::bridge::Receiver<kf::u8, 1>;

private:
    /// @brief Экземпляр отправителя для создания инструкций
    Sender sender;

    /// @brief Экземпляр приёмника для обработки приходящих инструкций
    Receiver receiver;

public:
    // Инструкции отправки

    /// @brief send_millis() -> u32
    bytelang::bridge::Instruction <Sender::Code> send_millis;

    /// @brief (...) -> send_log() -> u8[u8]
    bytelang::bridge::Instruction <Sender::Code, kf::slice<char>> send_log;

    //

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
                        return {BridgeError::InstructionArgumentWriteFail};
                    }

                    return {};
                })
        },
        send_log{
            sender.createInstruction<kf::slice<char>>(
                [](bytelang::core::OutputStream &stream, kf::slice<char> buffer) -> BridgeResult {
                    if (not stream.write(static_cast<kf::u8>(buffer.size))) {
                        return {BridgeError::InstructionArgumentWriteFail};
                    }

                    if (not stream.write(buffer.ptr, buffer.size)) {
                        return {BridgeError::InstructionArgumentWriteFail};
                    }

                    return {};
                })
        } {}

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

            //
        };
    }
};

}// namespace zms
