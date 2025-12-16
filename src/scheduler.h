#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

// Görev Yapısı
// src/scheduler.h içindeki struct'ı bununla güncelle:
typedef struct {
    int id;
    int arrivalTime;
    int priority;
    int burstTime;
    int remainingTime;
    int currentPriority;
    const char* color;
    int status; // 0: Bekliyor/Calisiyor, 1: Bitti, 2: Zaman Asimi (YENI)
} TaskBlock;

// Fonksiyon Prototipleri
void vSchedulerInit(void);
void vSchedulerStart(void);
void vTaskSimulator(void *pvParameters); // İşlemciyi simüle eden fonksiyon

#endif