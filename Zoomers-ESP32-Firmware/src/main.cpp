#include <Arduino.h>
#include <kf/Logger.hpp>

#include "zms/Periphery.hpp"
#include "zms/Service.hpp"


static auto &periphery = zms::Periphery::instance();

static auto &service = zms::Service::instance();

void setup() {
    Serial.begin(115200);

    kf_Logger_setWriter([](const char *buffer, size_t size) {
        service.bytelang_bridge.send_log(buffer, size);
    });

    kf_Logger_info("begin");

    const bool periphery_ok = periphery.init();
    const bool service_ok = service.init();

    if (not periphery_ok or not service_ok) {
        kf_Logger_fatal("Robot init failed!");
        periphery.storage.erase();
        delay(5000);
        ESP.restart();
    }

    kf_Logger_debug("init ok");
}

void loop() {
    service.poll();
}
