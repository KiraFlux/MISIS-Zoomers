#pragma once

#include <WiFi.h>
#include <kf/Logger.hpp>
#include <kf/espnow.hpp>


namespace zms {

/// @brief Узел Espnow
struct EspnowNode {

public:
    /// @brief Настройки узла
    struct Settings {
        /// @brief MAC-Адрес пульта
        kf::espnow::Mac remote_controller_mac;
    };

private:
    /// @brief Настройки узла
    const Settings &settings;

public:
    /// @brief Обработчик входящего пакета от пульта
    std::function<void(const void *, rs::u8)> on_receive{nullptr};

    explicit EspnowNode(const Settings &settings) :
        settings{settings} {}

    /// @brief Инициализировать протокол
    /// @returns <code>true</code> - Успешная инициализация
    [[nodiscard]] bool init() const {
        kf_Logger_info("init");

        const bool wifi_ok = WiFiClass::mode(WIFI_MODE_STA);
        if (not wifi_ok) {
            kf_Logger_error("WIFI fail");
            return false;
        }

        const auto init_result = kf::espnow::Protocol::init();
        if (init_result.fail()) {
            kf_Logger_error(rs::toString(init_result.error));
            return false;
        }

        const auto peer_result = kf::espnow::Peer::add(settings.remote_controller_mac);
        if (peer_result.fail()) {
            kf_Logger_error(rs::toString(peer_result.error));
            return false;
        }

        auto receive_handler = [this](const kf::espnow::Mac &mac, const void *data, rs::u8 len) {
            if (this->on_receive and mac == this->settings.remote_controller_mac) { this->on_receive(data, len); }
        };
        const auto handler_result = kf::espnow::Protocol::instance().setReceiveHandler(receive_handler);
        if (handler_result.fail()) {
            kf_Logger_error(rs::toString(handler_result.error));
            return false;
        }

        kf_Logger_debug("success");
        return true;
    }

    /// @brief Отправить пакет данных на пульт
    template<typename T> [[nodiscard]] inline rs::Result<void, kf::espnow::Error> send(const T &value) {
        return kf::espnow::Protocol::send(settings.remote_controller_mac, value);
    }

    /// @brief Отправить буфер на пульт
    [[nodiscard]] inline rs::Result<void, kf::espnow::Error> send(const void *data, rs::u8 size) {
        return kf::espnow::Protocol::send(settings.remote_controller_mac, data, size);
    }
};

}// namespace zms
