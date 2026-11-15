#include <stdio.h>
#include "common.h"
#include "system_controller.h"
#include "glue.h"

#define NUM_PLACES 4
#define MAX_HISTORY 50

static char place_names[NUM_PLACES][20] = {"Riverside", "Colony", "Market", "Temple"};
static int place_ids[NUM_PLACES];
static LocationRisk ranking[NUM_PLACES];

static Alert history[MAX_HISTORY];
static int history_count = 0;

static int ready = 0;
static int data_loaded = 0;
static int simulation_done = 0;
static int scenario_number = 0;
static int bridge_blocked = 0;
static int hill_blocked = 0;
static int report_counter = 0;
static int clock_time = 0;

static int last_place = -1;
static int last_mode = 0;
static int last_route_found = 0;
static Route last_route;

int read_int(const char *prompt, int low, int high) {
    char line[50];
    int value;

    while (1) {
        printf("%s", prompt);
        if (fgets(line, sizeof(line), stdin) == NULL) {
            return low;
        }
        if (sscanf(line, "%d", &value) == 1 && value >= low && value <= high) {
            return value;
        }
        printf("Please enter a number from %d to %d.\n", low, high);
    }
}

const char *risk_name(RiskLevel level) {
    switch (level) {
        case RISK_NORMAL:   return "NORMAL";
        case RISK_WATCH:    return "WATCH";
        case RISK_WARNING:  return "WARNING";
        case RISK_CRITICAL: return "CRITICAL";
        default:            return "UNKNOWN";
    }
}

static const char *scenario_path(int number) {
    if (number == 1) {
        return "data/readings_normal.txt";
    }
    if (number == 2) {
        return "data/readings_flood.txt";
    }
    return "data/readings_critical.txt";
}

static int ask_place(void) {
    int i;

    for (i = 0; i < NUM_PLACES; i++) {
        printf("%d. %s\n", i + 1, place_names[i]);
    }
    return read_int("Choose a place: ", 1, NUM_PLACES) - 1;
}

static void show_menu(void) {
    printf("\n===== JalRakshak Menu =====\n");
    printf("1. Load sensor data\n");
    printf("2. Run simulation (risk and alerts)\n");
    printf("3. Show risk ranking of places\n");
    printf("4. Process alerts (highest priority first)\n");
    printf("5. Show alert history\n");
    printf("6. Submit a citizen report\n");
    printf("7. Block or unblock a road\n");
    printf("8. Find evacuation route\n");
    printf("9. Save report to file\n");
    printf("10. Save state\n");
    printf("11. Load state\n");
    printf("0. Exit\n");
}

static void menu_load_data(void) {
    int choice;
    int count;
    int invalid = 0;

    printf("1. Normal rainfall\n2. Rapid flooding\n3. Critical flooding\n");
    choice = read_int("Choose scenario: ", 1, 3);

    count = glue_load_scenario(scenario_path(choice), &invalid);
    if (count < 0) {
        printf("Error: could not open %s\n", scenario_path(choice));
        data_loaded = 0;
        return;
    }
    if (count == 0) {
        printf("Error: the file has no valid readings.\n");
        data_loaded = 0;
        return;
    }
    printf("Loaded %d readings. Invalid readings skipped: %d\n", count, invalid);
    data_loaded = 1;
    scenario_number = choice;
    simulation_done = 0;
}

static void menu_run_simulation(void) {
    int i;
    int reports;

    if (data_loaded == 0) {
        printf("Please load sensor data first (option 1).\n");
        return;
    }

    glue_clear_alerts();
    for (i = 0; i < NUM_PLACES; i++) {
        reports = glue_verified_reports(place_ids[i]);
        ranking[i].location_id = place_ids[i];
        ranking[i].result = glue_risk(place_ids[i], reports);
    }
    glue_sort(ranking, NUM_PLACES);

    for (i = 0; i < NUM_PLACES; i++) {
        if (ranking[i].result.level >= RISK_WARNING) {
            glue_make_alert(ranking[i].location_id, ranking[i].result, clock_time);
        }
    }
    clock_time = clock_time + 1;
    simulation_done = 1;

    printf("Risk calculated for %d places.\n", NUM_PLACES);
    printf("Highest risk: %s (%s)\n", glue_place_name(ranking[0].location_id),
           risk_name(ranking[0].result.level));
}

static void menu_show_ranking(void) {
    int i;

    if (simulation_done == 0) {
        printf("Please run the simulation first (option 2).\n");
        return;
    }
    printf("\nPlaces from highest to lowest risk:\n");
    for (i = 0; i < NUM_PLACES; i++) {
        printf("%d. %-10s score %5.1f  %s\n", i + 1, glue_place_name(ranking[i].location_id),
               ranking[i].result.score, risk_name(ranking[i].result.level));
    }
    printf("\nWhy %s is the highest:\n", glue_place_name(ranking[0].location_id));
    glue_explain(&ranking[0].result);
}

static void menu_process_alerts(void) {
    Alert a;
    char text[200];
    int count = 0;

    if (simulation_done == 0) {
        printf("Please run the simulation first (option 2).\n");
        return;
    }
    while (glue_next_alert(&a) == 1) {
        printf("\nALERT %d for %s (priority %d)\n", a.alert_id, glue_place_name(a.location_id), a.priority);
        glue_alert_text(&a, text, 200);
        printf("%s\n", text);
        if (history_count < MAX_HISTORY) {
            history[history_count] = a;
            history_count++;
        }
        count++;
    }
    if (count == 0) {
        printf("No alerts are waiting.\n");
    } else {
        printf("\n%d alert(s) processed.\n", count);
    }
}

static void menu_show_history(void) {
    int i;

    if (history_count == 0) {
        printf("No alerts have been processed yet.\n");
        return;
    }
    printf("\nAlert history:\n");
    for (i = 0; i < history_count; i++) {
        printf("%d. Alert %d  %s  %s  priority %d\n", i + 1, history[i].alert_id,
               glue_place_name(history[i].location_id), risk_name(history[i].level),
               history[i].priority);
    }
}

static void menu_submit_report(void) {
    CitizenReport r;
    int place;
    int result;

    place = ask_place();
    r.severity = read_int("Severity (1 to 5): ", 1, 5);

    report_counter = report_counter + 1;
    r.report_id = report_counter;
    r.location_id = place_ids[place];
    r.hazard = HAZARD_FLOOD;
    r.timestamp = clock_time;
    clock_time = clock_time + 5;

    result = glue_submit_report(r);
    if (result == STATUS_OK) {
        printf("Report accepted.\n");
    } else if (result == STATUS_DUPLICATE) {
        printf("This looks like a duplicate report. It was not counted again.\n");
    } else {
        printf("The report could not be saved.\n");
    }
    printf("Similar reports for %s: %d\n", place_names[place], glue_verified_reports(place_ids[place]));
}

static void run_route(int place, int mode) {
    Route r;

    r = glue_route(place_ids[place], mode);
    last_place = place;
    last_mode = mode;
    if (r.status != STATUS_OK) {
        last_route_found = 0;
        printf("No safe route found from %s.\n", place_names[place]);
        printf("Please stay on high ground and wait for help.\n");
        return;
    }
    last_route_found = 1;
    last_route = r;
    printf("Route from %s:\n", place_names[place]);
    glue_print_route(&r);
    printf("Distance: %.1f km\n", r.total_distance);
}

static void menu_find_route(void) {
    int place;
    int mode;

    place = ask_place();
    printf("1. Shortest route\n2. Safest route\n");
    mode = read_int("Choose route type: ", 1, 2) - 1;
    run_route(place, mode);
}

static void menu_block_road(void) {
    int road;
    int old_found;
    double old_distance;

    road = read_int("1 = Bridge, 2 = Hill road: ", 1, 2);
    if (road == 1) {
        if (bridge_blocked == 0) {
            bridge_blocked = 1;
            glue_set_road("Bridge", "Market", ROAD_BLOCKED);
            printf("Bridge is now BLOCKED.\n");
        } else {
            bridge_blocked = 0;
            glue_set_road("Bridge", "Market", ROAD_OPEN);
            printf("Bridge is now OPEN.\n");
        }
    } else {
        if (hill_blocked == 0) {
            hill_blocked = 1;
            glue_set_road("HillRoad", "Temple", ROAD_BLOCKED);
            printf("Hill road is now BLOCKED.\n");
        } else {
            hill_blocked = 0;
            glue_set_road("HillRoad", "Temple", ROAD_OPEN);
            printf("Hill road is now OPEN.\n");
        }
    }

    if (last_place >= 0) {
        old_found = last_route_found;
        old_distance = last_route.total_distance;
        printf("\nRecalculating the last route...\n");
        run_route(last_place, last_mode);
        if (old_found == 1 && last_route_found == 1) {
            printf("Old distance: %.1f km, new distance: %.1f km\n", old_distance,
                   last_route.total_distance);
        }
    }
}

static void menu_save_report(void) {
    FILE *fp;
    int i;

    if (simulation_done == 0) {
        printf("Please run the simulation first (option 2).\n");
        return;
    }
    fp = fopen("data/report.txt", "w");
    if (fp == NULL) {
        printf("Error: could not create data/report.txt\n");
        return;
    }
    fprintf(fp, "JalRakshak Report\n");
    fprintf(fp, "Scenario number: %d\n", scenario_number);
    for (i = 0; i < NUM_PLACES; i++) {
        fprintf(fp, "%d. %s  score %.1f  %s\n", i + 1, glue_place_name(ranking[i].location_id),
                ranking[i].result.score, risk_name(ranking[i].result.level));
    }
    fprintf(fp, "Alerts processed: %d\n", history_count);
    fprintf(fp, "Bridge blocked: %d\n", bridge_blocked);
    fprintf(fp, "Hill road blocked: %d\n", hill_blocked);
    fclose(fp);
    printf("Report saved in data/report.txt\n");
}

static void menu_save_state(void) {
    FILE *fp;

    fp = fopen("data/state.txt", "w");
    if (fp == NULL) {
        printf("Error: could not create data/state.txt\n");
        return;
    }
    fprintf(fp, "%d %d %d %d\n", scenario_number, bridge_blocked, hill_blocked, report_counter);
    fclose(fp);
    glue_save_history("data/alert_history.txt");
    printf("State saved in data/state.txt\n");
}

static void menu_load_state(void) {
    FILE *fp;
    int scenario;
    int bridge;
    int hill;
    int counter;
    int invalid = 0;
    int count;

    fp = fopen("data/state.txt", "r");
    if (fp == NULL) {
        printf("Error: no saved state found.\n");
        return;
    }
    if (fscanf(fp, "%d %d %d %d", &scenario, &bridge, &hill, &counter) != 4) {
        printf("Error: the state file is damaged.\n");
        fclose(fp);
        return;
    }
    fclose(fp);

    if (scenario >= 1 && scenario <= 3) {
        count = glue_load_scenario(scenario_path(scenario), &invalid);
        if (count > 0) {
            data_loaded = 1;
            scenario_number = scenario;
            simulation_done = 0;
        }
    }
    if (bridge == 1) {
        glue_set_road("Bridge", "Market", ROAD_BLOCKED);
    } else {
        glue_set_road("Bridge", "Market", ROAD_OPEN);
    }
    if (hill == 1) {
        glue_set_road("HillRoad", "Temple", ROAD_BLOCKED);
    } else {
        glue_set_road("HillRoad", "Temple", ROAD_OPEN);
    }
    bridge_blocked = bridge;
    hill_blocked = hill;
    report_counter = counter;
    glue_load_history("data/alert_history.txt");
    printf("State loaded. Run the simulation again to see the risk.\n");
}

int get_summary(double *score, int *level, int *place) {
    if (simulation_done == 0) {
        return 0;
    }
    *score = ranking[0].result.score;
    *level = ranking[0].result.level;
    *place = ranking[0].location_id;
    return 1;
}

int get_history_count(void) {
    return history_count;
}

Alert get_history_item(int index) {
    return history[index];
}

void system_init(void) {
    int i;

    history_count = 0;
    ready = 0;
    data_loaded = 0;
    simulation_done = 0;
    scenario_number = 0;
    bridge_blocked = 0;
    hill_blocked = 0;
    report_counter = 0;
    clock_time = 0;
    last_place = -1;
    last_route_found = 0;

    printf("=====================================\n");
    printf(" JalRakshak - Flood Early Warning\n");
    printf("=====================================\n");

    if (glue_start() != 0) {
        printf("Error: could not start the modules. Is data/map.txt there?\n");
        return;
    }
    ready = 1;
    for (i = 0; i < NUM_PLACES; i++) {
        place_ids[i] = glue_find_place(place_names[i]);
        if (place_ids[i] < 0) {
            printf("Error: place %s was not found in the map.\n", place_names[i]);
            ready = 0;
        }
    }
}

void system_run(void) {
    int choice;

    if (ready == 0) {
        printf("The system is not ready, so the program will close.\n");
        return;
    }
    do {
        show_menu();
        choice = read_int("Enter your choice: ", 0, 11);

        switch (choice) {
            case 1:  menu_load_data();       break;
            case 2:  menu_run_simulation();  break;
            case 3:  menu_show_ranking();    break;
            case 4:  menu_process_alerts();  break;
            case 5:  menu_show_history();    break;
            case 6:  menu_submit_report();   break;
            case 7:  menu_block_road();      break;
            case 8:  menu_find_route();      break;
            case 9:  menu_save_report();     break;
            case 10: menu_save_state();      break;
            case 11: menu_load_state();      break;
            case 0:  printf("Goodbye!\n");   break;
        }
    } while (choice != 0);

    glue_end();
}
