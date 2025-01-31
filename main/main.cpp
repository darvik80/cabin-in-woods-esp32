#include <cstdio>
#include <esp_random.h>
#include <memory>
#include <core/Application.h>
#include <core/system/SystemEvent.h>
#include <core/system/telemetry/TelemetryService.h>
#include <driver/gpio.h>
#include <servo/ServoMotor.h>

extern "C" {
#include <df_player.h>
#include <jq6500_player.h>
}

#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "led/LedStripService.h"

class Cabin : public Application<Cabin> {
public:
    Cabin() = default;

protected:
    void userSetup() override {
        df_player_config_t door_player_config{
            .port = UART_NUM_1,
            .tx_pin = GPIO_NUM_17,
            .rx_pin = GPIO_NUM_18,
            .baud_rate = 9600,

            .busy_pin = GPIO_NUM_42,
            .rx_buffer_size = 256,
            .tx_buffer_size = 256,
        };
        mp3_player_handle_t door_player;
        ESP_ERROR_CHECK(create_df_player(&door_player_config, &door_player));

        jq6500_player_config_t bake_player_config{
            .port = UART_NUM_2,
            .tx_pin = GPIO_NUM_13,
            .rx_pin = GPIO_NUM_15,
            .baud_rate = 9600,

            .busy_pin = GPIO_NUM_39,
            .rx_buffer_size = 256,
            .tx_buffer_size = 256,
        };
        mp3_player_handle_t bake_player;
        ESP_ERROR_CHECK(create_jq6500_player(&bake_player_config, &bake_player));

        auto &door = getRegistry().create<ServoMotor>(ServoMotorOptions{
            .gpio = GPIO_NUM_16
        });

        getRegistry().create<TelemetryService>();
        auto &bakeLed = getRegistry().create<LedStripService<Service_App_LedBake, GPIO_NUM_40, 16> >();
        bakeLed.setColor(0, 15, LedColor{255, 255, 0});
        bakeLed.refresh();

        FreeRTOSTask::execute([&bakeLed, bake_player]() {
            mp3_player_reset(bake_player);
            mp3_player_volume(bake_player, 29);
            mp3_player_play(bake_player, 1);
            while (true) {
                for (int idx = 0; idx < 16; idx++) {
                    uint8_t r = ((uint64_t) 256 * esp_random() >> 32);
                    bakeLed.setColor(idx, idx, LedColor{r, r, 0});
                }
                bakeLed.refresh();
                if (gpio_get_level(GPIO_NUM_39) == 0) {
                    mp3_player_play(bake_player, 1);
                    vTaskDelay(pdMS_TO_TICKS(50));
                } else {
                    vTaskDelay(pdMS_TO_TICKS(50));
                }
            }
        }, "bake-task", 4096);

        FreeRTOSTask::execute([&door, door_player] {
            //xmp3_player_reset(door_player);
            mp3_player_volume(door_player, 10);
            door.move(90);
            while (true) {
                mp3_player_play(door_player, 3);
                for (int angle = 90; angle >= -10; angle -= 1) {
                    door.move(angle);
                    vTaskDelay(pdMS_TO_TICKS(50));
                }

                vTaskDelay(pdMS_TO_TICKS((uint64_t)10000 * esp_random() >> 32) + 500);
                mp3_player_play(door_player, 1);
                for (int angle = -10; angle <= 90; angle += 4) {
                    door.move(angle);
                    vTaskDelay(pdMS_TO_TICKS(50));
                }
                vTaskDelay(pdMS_TO_TICKS((uint64_t)10000 * esp_random() >> 32) + 500);
            }
        }, "door-task", 4096);
    }
};

static std::shared_ptr<Cabin> app;

extern "C" void app_main(void) {
    esp_log_level_set("*", ESP_LOG_INFO);
    const size_t free = heap_caps_get_free_size(MALLOC_CAP_DEFAULT);
    const size_t total = heap_caps_get_total_size(MALLOC_CAP_DEFAULT);
    esp_logi(app, "heap: %zu/%zu", free, total);

    app = std::make_shared<Cabin>();
    app->setup();
}
