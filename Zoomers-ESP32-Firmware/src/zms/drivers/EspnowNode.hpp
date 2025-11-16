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
    std::function<void(kf::slice<const void>)> on_receive{nullptr};

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
        if (not init_result.isOk()) {
            kf_Logger_error(kf::espnow::stringFromError(init_result.error().value()));
            return false;
        }

        const auto peer_result = kf::espnow::Peer::add(settings.remote_controller_mac);
        if (not peer_result.isOk()) {
            kf_Logger_error(kf::espnow::stringFromError(init_result.error().value()));
            return false;
        }

        auto receive_handler = [this](const kf::espnow::Mac &mac, kf::slice<const void> source) {
            if (on_receive and mac == settings.remote_controller_mac) {
                on_receive(source);
            }
        };

        const auto handler_result = kf::espnow::Protocol::instance().setReceiveHandler(receive_handler);
        if (not handler_result.isOk()) {
            kf_Logger_error(kf::espnow::stringFromError(init_result.error().value()));
            return false;
        }

        kf_Logger_debug("success");
        return true;
    }

    /// @brief Отправить пакет данных на пульт
    template<typename T> [[nodiscard]] inline kf::Result<void, kf::espnow::Error> send(const T &value) {
        return kf::espnow::Protocol::send(settings.remote_controller_mac, value);
    }

    /// @brief Отправить буфер на пульт
    [[nodiscard]] inline kf::Result<void, kf::espnow::Error> send(kf::slice<const void> source) {
        return kf::espnow::Protocol::send(settings.remote_controller_mac, source);
    }
};

}// namespace zms
