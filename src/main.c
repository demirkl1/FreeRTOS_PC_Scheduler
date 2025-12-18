#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "colors.h"
#include "scheduler.h"

// Global Görev Listesi (Basitleştirilmiş)
TaskBlock taskList[100];
int taskCount = 0;

const char* colors[] = {
    C_RED,
    C_ORANGE,
    C_AMBER,
    C_YELLOW,
    C_CHARTREUSE,
    C_GREEN,
    C_SPRING,
    C_CYAN,
    C_AZURE,
    C_BLUE,
    C_INDIGO,
    C_VIOLET,
    C_PURPLE,
    C_MAGENTA,
    C_PINK,
    C_ROSE,
    C_BROWN,
    C_TAN,
    C_OLIVE,
    C_MINT,
    C_TEAL,
    C_NAVY,
    C_GOLDENROD,
    C_CORAL,
    C_AQUAMARINE
};


void readInputFile(const char* filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Dosya acilamadi");
        exit(1);
    }

    int colorCount = sizeof(colors) / sizeof(colors[0]);
    char line[100];
    while (fgets(line, sizeof(line), file)) {
        // Format: varış, öncelik, süre [cite: 74-75]
        // Örn: 12, 0, 1
        int arrival, prio, burst;
        if (sscanf(line, "%d, %d, %d", &arrival, &prio, &burst) == 3) {
            taskList[taskCount].id = taskCount;
            taskList[taskCount].arrivalTime = arrival;
            taskList[taskCount].priority = prio;
            taskList[taskCount].currentPriority = prio;
            taskList[taskCount].burstTime = burst;
            taskList[taskCount].remainingTime = burst;
            taskList[taskCount].color = colors[taskCount % colorCount];
            taskCount++;
        }
    }
    fclose(file);
    printf("Toplam %d gorev okundu.\n", taskCount);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Kullanim: ./freertos_sim giris.txt\n"); // [cite: 107]
        return 1;
    }

    readInputFile(argv[1]);

    // Scheduler'ı başlat
    vSchedulerInit();
    
    // FreeRTOS Kernel'ı başlat (Bu noktadan sonra kontrol FreeRTOS'a geçer)
    vTaskStartScheduler();

    return 0;
}