#include "scheduler.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "queue.h"
#include "task.h"

// --- RENK TANIMLARI ---
#define C_RESET   "\x1B[0m"

// ID'ye göre renk döndüren fonksiyon
const char* getTaskColor(int id) {
    // 12 farklı renk paleti tanımladık
    const char* colors[] = {
        "\x1B[31m", // Kırmızı
        "\x1B[32m", // Yeşil
        "\x1B[33m", // Sarı
        "\x1B[34m", // Mavi
        "\x1B[35m", // Magenta
        "\x1B[36m", // Cyan
        "\x1B[91m", // Parlak Kırmızı
        "\x1B[92m", // Parlak Yeşil
        "\x1B[93m", // Parlak Sarı
        "\x1B[94m", // Parlak Mavi
        "\x1B[95m", // Parlak Magenta
        "\x1B[96m"  // Parlak Cyan
    };
    // ID mod 12 yaparak renkleri döngüye sokuyoruz
    return colors[id % 12];
}

extern TaskBlock taskList[];
extern int taskCount;

// Kuyruklar
QueueHandle_t realTimeQueue;
QueueHandle_t userQueue1;
QueueHandle_t userQueue2;
QueueHandle_t userQueue3;

// Senkronizasyon
TaskHandle_t xDispatcherHandle = NULL;
TaskHandle_t xProcessorHandle = NULL;

// Global Değişkenler
int globalTime = 0;
int finishedTaskCount = 0;
int currentRunningTaskId = -1; 
int lastActiveTime[100]; // Starvation takibi için

// Yardımcı Fonksiyon
int isQueueNotEmpty(QueueHandle_t q) {
    return uxQueueMessagesWaiting(q) > 0;
}

// -----------------------------------------------------------------------
// DISPATCHER TASK
// -----------------------------------------------------------------------
void vDispatcherTask(void *pvParameters) {
    // Başlangıçta lastActiveTime dizisini temizle
    for(int i=0; i<100; i++) lastActiveTime[i] = -1;

    while(1) {
        // 1. ZAMAN AŞIMI KONTROLÜ (20 sn Kuralı - Starvation)
        for (int i = 0; i < taskCount; i++) {
            if (taskList[i].status != 1 && taskList[i].status != 2 && taskList[i].arrivalTime <= globalTime) {
                
                int referenceTime = (lastActiveTime[i] == -1) ? taskList[i].arrivalTime : lastActiveTime[i];

                if (taskList[i].id != currentRunningTaskId && (globalTime - referenceTime) >= 20) {
                    taskList[i].status = 2; // Timeout
                    finishedTaskCount++;
                    
                    // Rengi ID'ye göre al
                    const char* color = getTaskColor(taskList[i].id);
                    
                    printf("%s%d.0000 sn proses zamanasimi\t\t(id:%04d \toncelik:%d \tkalan sure:%d sn)%s\n", 
                           color, globalTime, taskList[i].id, 
                           taskList[i].currentPriority, taskList[i].remainingTime, C_RESET);
                }
            }
        }

        // 2. YENİ GELEN GÖREVLERİ KUYRUĞA AL
        for (int i = 0; i < taskCount; i++) {
            if (taskList[i].arrivalTime == globalTime) {
                taskList[i].status = 0; // Ready
                taskList[i].currentPriority = taskList[i].priority; 
                lastActiveTime[i] = globalTime; // Giriş saati kaydet

                if (taskList[i].priority == 0) xQueueSend(realTimeQueue, &taskList[i].id, 0);
                else if (taskList[i].priority == 1) xQueueSend(userQueue1, &taskList[i].id, 0);
                else if (taskList[i].priority == 2) xQueueSend(userQueue2, &taskList[i].id, 0);
                else xQueueSend(userQueue3, &taskList[i].id, 0);

                // Rengi ID'ye göre al
                const char* color = getTaskColor(taskList[i].id);

                printf("%s%d.0000 sn proses basladi\t\t(id:%04d \toncelik:%d \tkalan sure:%d sn)%s\n", 
                       color, globalTime, taskList[i].id, 
                       taskList[i].priority, taskList[i].burstTime, C_RESET);
            }
        }

        if (finishedTaskCount >= taskCount) {
             printf("Tum islemler tamamlandi. Simulasyon sonlaniyor...\n");
             vTaskEndScheduler();
             exit(0);
        }

        xTaskNotifyGive(xProcessorHandle);
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    }
}

// -----------------------------------------------------------------------
// PROCESSOR TASK
// -----------------------------------------------------------------------
void vProcessorTask(void *pvParameters) {
    int currentTaskId = -1;
    TaskBlock *currentTaskPtr = NULL;

    while(1) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        
        currentTaskId = -1;
        currentRunningTaskId = -1;
        int foundValidTask = 0;

        // --- GÖREV SEÇİM DÖNGÜSÜ ---
        while (!foundValidTask) {
            currentTaskId = -1;
            
            if (isQueueNotEmpty(realTimeQueue))      xQueuePeek(realTimeQueue, &currentTaskId, 0);
            else if (isQueueNotEmpty(userQueue1))    xQueueReceive(userQueue1, &currentTaskId, 0);
            else if (isQueueNotEmpty(userQueue2))    xQueueReceive(userQueue2, &currentTaskId, 0);
            else if (isQueueNotEmpty(userQueue3))    xQueueReceive(userQueue3, &currentTaskId, 0);

            if (currentTaskId == -1) break; 

            currentTaskPtr = &taskList[currentTaskId];

            if (currentTaskPtr->status == 2) {
                if (currentTaskPtr->priority == 0) { 
                    int temp; xQueueReceive(realTimeQueue, &temp, 0); 
                }
                continue; 
            } else {
                foundValidTask = 1;
            }
        }

        // --- EXECUTION (YÜRÜTME) ---
        if (foundValidTask) {
            currentRunningTaskId = currentTaskPtr->id;
            
            // Rengi ID'ye göre al
            const char* color = getTaskColor(currentTaskPtr->id);

            printf("%s%d.0000 sn proses yurutuluyor\t\t(id:%04d \toncelik:%d \tkalan sure:%d sn)%s\n", 
                   color, globalTime + 1, currentTaskPtr->id, 
                   currentTaskPtr->currentPriority, currentTaskPtr->remainingTime, C_RESET);

            vTaskDelay(pdMS_TO_TICKS(10)); 
            currentTaskPtr->remainingTime--;
            globalTime++; 

            lastActiveTime[currentTaskPtr->id] = globalTime;

            if (currentTaskPtr->remainingTime <= 0) {
                currentTaskPtr->status = 1;
                finishedTaskCount++;
                
                printf("%s%d.0000 sn proses sonlandi\t\t(id:%04d \toncelik:%d \tkalan sure:0 sn)%s\n", 
                       color, globalTime, currentTaskPtr->id, currentTaskPtr->currentPriority, C_RESET);
                
                if (currentTaskPtr->priority == 0) { int temp; xQueueReceive(realTimeQueue, &temp, 0); }
            } 
            else {
                if (currentTaskPtr->priority == 0) {
                    // RealTime
                } else {
                    currentTaskPtr->currentPriority++;
                    
                    printf("%s%d.0000 sn proses askida \t\t(id:%04d \toncelik:%d \tkalan sure:%d sn)%s\n", 
                           color, globalTime, currentTaskPtr->id, 
                           currentTaskPtr->currentPriority, currentTaskPtr->remainingTime, C_RESET);

                    if (currentTaskPtr->currentPriority == 2) xQueueSend(userQueue2, &currentTaskPtr->id, 0);
                    else xQueueSend(userQueue3, &currentTaskPtr->id, 0);
                }
            }
        } else {
            globalTime++;
        }
        
        currentRunningTaskId = -1;
        xTaskNotifyGive(xDispatcherHandle);
    }
}

void vSchedulerInit(void) {
    realTimeQueue = xQueueCreate(100, sizeof(int));
    userQueue1 = xQueueCreate(100, sizeof(int));
    userQueue2 = xQueueCreate(100, sizeof(int));
    userQueue3 = xQueueCreate(100, sizeof(int));
}

void vSchedulerStart(void){
    xTaskCreate(vDispatcherTask, "Dispatcher", configMINIMAL_STACK_SIZE * 4, NULL, 4, &xDispatcherHandle);
    xTaskCreate(vProcessorTask, "Processor", configMINIMAL_STACK_SIZE * 4, NULL, 3, &xProcessorHandle);
}