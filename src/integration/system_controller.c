
//This file shows the main menu and calls the other modules.

#include <stdio.h>
#include <string.h>
#include "common.h"
#include "system_controller.h"

#define MAX_READINGS 100
#define MAX_ALERTS   20

//data kept by the controller
static SensorReading readings[MAX_READINGS];
static int reading_count = 0;

static Alert alerts[MAX_ALERTS];
static int alert_count = 0;

static double current_score = 0;
static RiskLevel current_risk = RISK_NORMAL;
static int data_loaded = 0;
static int simulation_done = 0;

//1 means the road is blocked
static int bridge_blocked = 0;
static int hill_road_blocked = 0;


//helper functions

//Asks a number from the user and keeps asking until it is correct
int read_int(const char *prompt, int low, int high) {
    char line[50];
    int value;

    while (1) {
        printf("%s", prompt);
        if (fgets(line, sizeof(line), stdin) == NULL) {
            return low;   //no more input=stop
        }
        if (sscanf(line, "%d", &value) == 1 && value >= low && value <= high) {
            return value;
        }
        printf("Please enter a number from %d to %d.\n", low, high);
    }
}

//Gives the name of a risk level as text
const char *risk_name(RiskLevel level) {
    switch (level) {
        case RISK_NORMAL:   return "NORMAL";
        case RISK_WATCH:    return "WATCH";
        case RISK_WARNING:  return "WARNING";
        case RISK_CRITICAL: return "CRITICAL";
        default:            return "UNKNOWN";
    }
}




//functions used in main.c

void system_init(void) {
    reading_count = 0;
    alert_count = 0;
    current_score = 0;
    current_risk = RISK_NORMAL;
    data_loaded = 0;
    simulation_done = 0;
    bridge_blocked = 0;
    hill_road_blocked = 0;

    printf("-\n");
    printf(" JalRakshak - Flood Early Warning\n");
    printf("-\n");
}

void system_run(void) {
    int choice;

    do {
        show_menu();
        choice = read_int("Enter your choice: ", 0, 7);

        switch (choice) {
            case 1: menu_load_data();      break;
            case 2: menu_run_simulation(); break;
            case 3: menu_show_risk();      break;
            case 4: menu_show_alerts();    break;
            case 5: menu_block_road();     break;
            case 6: stub_find_route();     break;
            case 7: menu_save_report();    break;
            case 0: printf("Goodbye!\n");  break;
        }
    } while (choice != 0);
}
