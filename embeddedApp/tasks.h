#pragma once
#include "webserver.h"

// Initialize freeRTOS functions
void taskWebServer(void *parameter);
void taskRecordVideo(void *parameter);
void taskStreamVideo(void *parameter);
void taskWebSocket(void *parameter);

void taskWebServer(void *parameter)
{
    while (true)
    {
        // ******* add function that handles the connection *******
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

void taskRecordVideo(void *parameter)
{
    while (true)
    {
        // ******* add function that handles frame capture *******
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

void taskStreamVideo(void *parameter)
{
    while (true)
    {
        // ******* add function that handles stream frame *******
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

void taskWebSocket(void *parameter)
{
    while (true)
    {
        // Update heartbeat data and broadcast via WebSocket
        detectHB();
        updateIMU();
        // updateUWB(); // <------- ******* ADD WHEN AVAILABLE *******

        String data = "{\"bpm\": " + String(bpm) + ", \"avg_bpm\": " +
        String(bpmAvg) + ", \"ax\": " + String(accX) + ", \"ay\": " + String(accY) + ", \"az\": " + String(accZ) + ", \"gx\": " +
        String(gyroX) + ", \"gy\": " + String(gyroY) + ", \"gz\": " + String(gyroZ) + ", \"temp\": " + String(temperature) + " }";

        webSocket.broadcastTXT(data); // Send heartbeat to all clients
        webSocket.loop();             // Process WebSocket events
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}