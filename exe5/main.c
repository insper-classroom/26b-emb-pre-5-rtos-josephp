
#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <queue.h>

#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"
#include "hardware/irq.h"

const int BTN_PIN_R = 28;
const int BTN_PIN_Y = 21;

const int LED_PIN_R = 5;
const int LED_PIN_Y = 10;

SemaphoreHandle_t xSemaphoreR;
SemaphoreHandle_t xSemaphoreY;

QueueHandle_t xQueueR;
QueueHandle_t xQueueY;


void btn_callback(uint gpio, uint32_t events) {
    if (events == 0x4) {
        if (gpio == BTN_PIN_R) {
            xSemaphoreGiveFromISR(xSemaphoreR, 0);
        }
        else {
            xSemaphoreGiveFromISR(xSemaphoreY, 0);
        }
    }
}


void btn_task_1(void* p) {
    gpio_init(BTN_PIN_R);
    gpio_set_dir(BTN_PIN_R, GPIO_IN);
    gpio_pull_up(BTN_PIN_R);

    gpio_set_irq_enabled_with_callback(
        BTN_PIN_R, GPIO_IRQ_EDGE_FALL, true, &btn_callback
    );

    int estado = 0;

    while (true) {
        if (xSemaphoreTake(xSemaphoreR, pdMS_TO_TICKS(500)) == pdTRUE) {
            estado = !estado;
            xQueueSend(xQueueR, &estado, 0);
        }
    }
}


// TASK DO LED VERMELHO
void led_task_1(void* p) {
    gpio_init(LED_PIN_R);
    gpio_set_dir(LED_PIN_R, GPIO_OUT);

    int estado = 0;
    int led_ligado = 0;

    while (true) {

        // Verifica se chegou um novo estado
        if (xQueueReceive(xQueueR, &estado, 0) == pdTRUE) {
            if (estado == 0) {
                led_ligado = 0;
                gpio_put(LED_PIN_R, 0);
            }
        }

        // Se estado = 1, fica piscando
        if (estado == 1) {
            led_ligado = !led_ligado;
            gpio_put(LED_PIN_R, led_ligado);

            vTaskDelay(pdMS_TO_TICKS(100));
        }
        else {
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }
}


// TASK DO BOTÃO AMARELO
void btn_task_2(void* p) {
    gpio_init(BTN_PIN_Y);
    gpio_set_dir(BTN_PIN_Y, GPIO_IN);
    gpio_pull_up(BTN_PIN_Y);

    gpio_set_irq_enabled(
        BTN_PIN_Y, GPIO_IRQ_EDGE_FALL, true
    );

    int estado = 0;

    while (true) {
        if (xSemaphoreTake(xSemaphoreY, pdMS_TO_TICKS(500)) == pdTRUE) {
            estado = !estado;
            xQueueSend(xQueueY, &estado, 0);
        }
    }
}


// TASK DO LED AMARELO
void led_task_2(void* p) {
    gpio_init(LED_PIN_Y);
    gpio_set_dir(LED_PIN_Y, GPIO_OUT);

    int estado = 0;
    int led_ligado = 0;

    while (true) {

        // Verifica se chegou um novo estado
        if (xQueueReceive(xQueueY, &estado, 0) == pdTRUE) {
            if (estado == 0) {
                led_ligado = 0;
                gpio_put(LED_PIN_Y, 0);
            }
        }

        // Se estado = 1, fica piscando
        if (estado == 1) {
            led_ligado = !led_ligado;
            gpio_put(LED_PIN_Y, led_ligado);

            vTaskDelay(pdMS_TO_TICKS(100));
        }
        else {
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }
}


// MAIN
int main() {
    stdio_init_all();

    xTaskCreate(btn_task_1, "BTN_Task 1", 256, NULL, 1, NULL);
    xTaskCreate(btn_task_2, "BTN_Task 2", 256, NULL, 1, NULL);
    xTaskCreate(led_task_1, "LED_Task 1", 256, NULL, 1, NULL);
    xTaskCreate(led_task_2, "LED_Task 2", 256, NULL, 1, NULL);

    xSemaphoreR = xSemaphoreCreateBinary();
    xSemaphoreY = xSemaphoreCreateBinary();

    xQueueR = xQueueCreate(10, sizeof(int));
    xQueueY = xQueueCreate(10, sizeof(int));

    vTaskStartScheduler();

    while (1) {}

    return 0;
}
