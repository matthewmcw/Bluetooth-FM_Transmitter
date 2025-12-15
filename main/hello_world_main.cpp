/*
 * 
 *
 * 
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "si4713.h"

extern "C" void app_main(void)
{
        // --- SETUP CODE GOES HERE ---
    printf("Setup: Initializing...\n");
    // gpio_set_direction(GPIO_NUM_2, GPIO_MODE_OUTPUT);



    for (int i = 10; i >= 0; i--) {
        printf("Restarting in %d seconds...\n", i);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    printf("Restarting now.\n");
    fflush(stdout);
    esp_restart();
}
