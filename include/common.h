#ifndef COMMON_H
#define COMMON_H

#define MAX_NAME  32
#define MAX_NODES 50

typedef enum {
    STATUS_OK = 0,
    STATUS_ERROR = -1,
    STATUS_FULL = -2,
    STATUS_EMPTY = -3,
    STATUS_INVALID = -4,
    STATUS_NOT_FOUND = -5,
    STATUS_DUPLICATE = -6
} Status;

typedef enum { RISK_NORMAL = 0, RISK_WATCH, RISK_WARNING, RISK_CRITICAL } RiskLevel;
typedef enum { SENSOR_RAIN = 0, SENSOR_WATER_LEVEL } SensorType;
typedef enum { HAZARD_FLOOD = 0, HAZARD_LANDSLIDE } HazardType;
typedef enum { ROAD_OPEN = 0, ROAD_BLOCKED, ROAD_HAZARDOUS } RoadStatus;

typedef struct {
    int sensor_id;
    SensorType type;
    double value;
    long timestamp;
    int location_id;
} SensorReading;

typedef struct {
    int report_id;
    int location_id;
    HazardType hazard;
    int severity;
    long timestamp;
} CitizenReport;

typedef struct {
    int alert_id;
    int location_id;
    RiskLevel level;
    double confidence;
    int priority;
    long timestamp;
} Alert;

typedef struct {
    double score;
    RiskLevel level;
    int top_factor;
    int anomaly;
} RiskResult;

typedef struct {
    int path[MAX_NODES];
    int length;
    double total_distance;
    Status status;
} Route;

typedef struct {
    int location_id;
    RiskResult result;
} LocationRisk;

#endif
