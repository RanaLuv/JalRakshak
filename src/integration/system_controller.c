#include <stdio.h>
#include "common.h"
#include "system_controller.h"
#include "glue.h"

#define MAX_PLACES 7

static int place_ids[MAX_PLACES];
static int place_count = 0;
static LocationRisk ranking[MAX_PLACES];

static int blocked[MAX_NODES][MAX_NODES];

static int ready = 0;
static int data_loaded = 0;
static int simulation_done = 0;
static int scenario_number = 0;
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

    for (i = 0; i < place_count; i++) {
        printf("%d. %s\n", i + 1, glue_place_name(place_ids[i]));
    }
    return read_int("Choose a place: ", 1, place_count) - 1;
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
    for (i = 0; i < place_count; i++) {
        reports = glue_verified_reports(place_ids[i]);
        ranking[i].location_id = place_ids[i];
        ranking[i].result = glue_risk(place_ids[i], reports);
    }
    glue_sort(ranking, place_count);

    for (i = 0; i < place_count; i++) {
        if (ranking[i].result.level >= RISK_WARNING) {
            glue_make_alert(ranking[i].location_id, ranking[i].result, clock_time);
        }
    }
    clock_time = clock_time + 1;
    simulation_done = 1;

    printf("Risk calculated for %d places.\n", place_count);
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
    for (i = 0; i < place_count; i++) {
        printf("%d. %-14s score %5.1f  %s\n", i + 1, glue_place_name(ranking[i].location_id),
               ranking[i].result.score, risk_name(ranking[i].result.level));
    }
    printf("\nWhy %s is the highest:\n", glue_place_name(ranking[0].location_id));
    glue_explain(&ranking[0].result);
}

static void menu_process_alerts(void) {
    Alert a;
    char text[512];
    int count = 0;

    if (simulation_done == 0) {
        printf("Please run the simulation first (option 2).\n");
        return;
    }
    while (glue_next_alert(&a) == 1) {
        printf("\nALERT %d for %s (priority %d)\n", a.alert_id, glue_place_name(a.location_id), a.priority);
        glue_alert_text(&a, text, 512);
        printf("%s\n", text);
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
    int total;
    Alert a;

    total = glue_history_count();
    if (total == 0) {
        printf("No alerts have been processed yet.\n");
        return;
    }
    printf("\nAlert history:\n");
    for (i = 0; i < total; i++) {
        glue_history_get(i, &a);
        printf("%d. Alert %d  %s  %s  priority %d\n", i + 1, a.alert_id,
               glue_place_name(a.location_id), risk_name(a.level), a.priority);
    }
}

static void menu_submit_report(void) {
    CitizenReport r;
    int place;
    int minutes;
    int result;

    place = ask_place();
    r.severity = read_int("Severity (1 to 5): ", 1, 5);
    minutes = read_int("Minutes since the previous report (0 = right away): ", 0, 120);

    report_counter = report_counter + 1;
    clock_time = clock_time + minutes * 60;
    r.report_id = report_counter;
    r.location_id = place_ids[place];
    r.hazard = HAZARD_FLOOD;
    r.timestamp = clock_time;

    result = glue_submit_report(r);
    if (result == STATUS_OK) {
        printf("Report accepted.\n");
    } else if (result == STATUS_DUPLICATE) {
        printf("This looks like a duplicate report. It was not counted again.\n");
    } else {
        printf("The report could not be saved.\n");
    }
    printf("Reports counted for %s: %d\n", glue_place_name(place_ids[place]),
           glue_verified_reports(place_ids[place]));
}

static void run_route(int place, int mode) {
    Route r;

    r = glue_route(place_ids[place], mode);
    last_place = place;
    last_mode = mode;
    if (r.status != STATUS_OK) {
        last_route_found = 0;
        printf("No safe route found from %s.\n", glue_place_name(place_ids[place]));
        printf("Please stay on high ground and wait for help.\n");
        return;
    }
    last_route_found = 1;
    last_route = r;
    printf("Route from %s:\n", glue_place_name(place_ids[place]));
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
    int total;
    int i;
    int from;
    int to;
    int old_found;
    double old_distance;

    total = glue_node_count();
    for (i = 0; i < total; i++) {
        printf("%d. %s\n", i + 1, glue_place_name(i));
    }
    from = read_int("Road starts at number: ", 1, total) - 1;
    to = read_int("Road ends at number: ", 1, total) - 1;
    if (from == to) {
        printf("Please choose two different places.\n");
        return;
    }

    if (blocked[from][to] == 0) {
        if (glue_set_road(from, to, ROAD_BLOCKED) != STATUS_OK) {
            printf("There is no road between these two places.\n");
            return;
        }
        blocked[from][to] = 1;
        blocked[to][from] = 1;
        printf("Road %s - %s is now BLOCKED.\n", glue_place_name(from), glue_place_name(to));
    } else {
        glue_set_road(from, to, ROAD_OPEN);
        blocked[from][to] = 0;
        blocked[to][from] = 0;
        printf("Road %s - %s is now OPEN.\n", glue_place_name(from), glue_place_name(to));
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

static int count_blocked_roads(void) {
    int i;
    int j;
    int count = 0;

    for (i = 0; i < MAX_NODES; i++) {
        for (j = i + 1; j < MAX_NODES; j++) {
            if (blocked[i][j] == 1) {
                count++;
            }
        }
    }
    return count;
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
    for (i = 0; i < place_count; i++) {
        fprintf(fp, "%d. %s  score %.1f  %s\n", i + 1, glue_place_name(ranking[i].location_id),
                ranking[i].result.score, risk_name(ranking[i].result.level));
    }
    fprintf(fp, "Alerts processed: %d\n", glue_history_count());
    fprintf(fp, "Blocked roads: %d\n", count_blocked_roads());
    fclose(fp);
    printf("Report saved in data/report.txt\n");
}

static void menu_save_state(void) {
    FILE *fp;
    int i;
    int j;

    fp = fopen("data/state.txt", "w");
    if (fp == NULL) {
        printf("Error: could not create data/state.txt\n");
        return;
    }
    fprintf(fp, "%d %d %d\n", scenario_number, report_counter, count_blocked_roads());
    for (i = 0; i < MAX_NODES; i++) {
        for (j = i + 1; j < MAX_NODES; j++) {
            if (blocked[i][j] == 1) {
                fprintf(fp, "%d %d\n", i, j);
            }
        }
    }
    fclose(fp);

    if (glue_history_save("data/alert_history.txt") != STATUS_OK) {
        printf("Error: could not save data/alert_history.txt\n");
        return;
    }
    printf("State saved in data/state.txt\n");
}

static void menu_load_state(void) {
    FILE *fp;
    int scenario;
    int counter;
    int pairs;
    int i;
    int j;
    int u;
    int v;
    int invalid = 0;
    int total;

    fp = fopen("data/state.txt", "r");
    if (fp == NULL) {
        printf("Error: no saved state found.\n");
        return;
    }
    if (fscanf(fp, "%d %d %d", &scenario, &counter, &pairs) != 3) {
        printf("Error: the state file is damaged.\n");
        fclose(fp);
        return;
    }

    total = glue_node_count();
    for (i = 0; i < MAX_NODES; i++) {
        for (j = 0; j < MAX_NODES; j++) {
            if (blocked[i][j] == 1) {
                glue_set_road(i, j, ROAD_OPEN);
                blocked[i][j] = 0;
            }
        }
    }
    for (i = 0; i < pairs; i++) {
        if (fscanf(fp, "%d %d", &u, &v) != 2) {
            break;
        }
        if (u >= 0 && u < total && v >= 0 && v < total) {
            if (glue_set_road(u, v, ROAD_BLOCKED) == STATUS_OK) {
                blocked[u][v] = 1;
                blocked[v][u] = 1;
            }
        }
    }
    fclose(fp);

    if (scenario >= 1 && scenario <= 3) {
        if (glue_load_scenario(scenario_path(scenario), &invalid) > 0) {
            data_loaded = 1;
            scenario_number = scenario;
            simulation_done = 0;
        }
    }
    report_counter = counter;

    if (glue_history_load("data/alert_history.txt") != STATUS_OK) {
        printf("Note: no alert history was loaded.\n");
    }
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
    return glue_history_count();
}

Alert get_history_item(int index) {
    Alert a;

    a.alert_id = 0;
    a.location_id = 0;
    a.level = RISK_NORMAL;
    a.confidence = 0;
    a.priority = 0;
    a.timestamp = 0;
    glue_history_get(index, &a);
    return a;
}

void system_init(void) {
    int i;
    int j;
    int total;

    ready = 0;
    data_loaded = 0;
    simulation_done = 0;
    scenario_number = 0;
    report_counter = 0;
    clock_time = 0;
    last_place = -1;
    last_route_found = 0;
    place_count = 0;
    for (i = 0; i < MAX_NODES; i++) {
        for (j = 0; j < MAX_NODES; j++) {
            blocked[i][j] = 0;
        }
    }

    printf("=====================================\n");
    printf(" JalRakshak - Flood Early Warning\n");
    printf("=====================================\n");

    if (glue_start() != 0) {
        printf("Error: could not start the modules.\n");
        return;
    }
    total = glue_node_count();
    for (i = 0; i < total && place_count < MAX_PLACES; i++) {
        if (glue_is_shelter(i) == 0) {
            place_ids[place_count] = i;
            place_count++;
        }
    }
    if (place_count == 0) {
        printf("Error: the map has no places to watch.\n");
        return;
    }
    ready = 1;
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
}

void system_close(void) {
    glue_end();
}
