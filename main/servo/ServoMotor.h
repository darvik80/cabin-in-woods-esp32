//
// Created by Ivan Kishchenko on 25/3/24.
//

#pragma once

#include "core/Core.h"
#include "config.h"

#include <soc/gpio_num.h>
#include <driver/mcpwm_types.h>

struct ServoMotorOptions {
    gpio_num_t gpio;
};

struct ServoControl {
    int angle{};
};

class ServoMotor : public TService<ServoMotor, Service_App_ServoDoor, Sys_User> {
    int _lastAngle{0};
    ServoMotorOptions _options;

    FreeRTOSMessageBus<ServoControl, 1> _bus;

    mcpwm_timer_handle_t _timer{nullptr};
    mcpwm_oper_handle_t _operator{nullptr};
    mcpwm_cmpr_handle_t _comparator{nullptr};
    mcpwm_gen_handle_t _generator{nullptr};
public:
    ServoMotor() = delete;

    ServoMotor(const ServoMotor &) = delete;

    ServoMotor &operator=(const ServoMotor &) = delete;

    explicit ServoMotor(Registry &registry, const ServoMotorOptions &options);

    [[nodiscard]] std::string_view getServiceName() const override {
        return "servo-motor";
    }

    void setup() override;
    void move(int angle) {
        _bus.overwrite(ServoControl{.angle = angle});
    }
private:
    void handle(const ServoControl &msg);

};
