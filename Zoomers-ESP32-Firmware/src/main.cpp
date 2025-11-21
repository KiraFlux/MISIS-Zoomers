#include <Arduino.h>
#include <kf/Logger.hpp>

#include "zms/Periphery.hpp"
#include "zms/Service.hpp"


static auto &periphery = zms::Periphery::instance();

static auto &service = zms::Service::instance();


void setup() {
    Serial.begin(115200);

    kf_Logger_setWriter([](const kf::slice<const char> &str) {
        service.bytelang_bridge.send_log(str);
    });

    kf_Logger_info("begin");

    const bool periphery_ok = periphery.init();

    if (not periphery_ok) {
        kf_Logger_fatal("Robot init failed!");
        delay(5000);
        ESP.restart();
    }

    service.init();

    kf_Logger_debug("init isOk");
}

void loop() {
    service.poll();
}
