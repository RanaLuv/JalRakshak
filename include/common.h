//common.h
 //Nobody else should edit this file.
//Every module includes this file so all of us use the same types.

#ifndef COMMON_H
#define COMMON_H

// Constants 
#define MAX_NAME  32   //longest name for a place or node, including '\0' 
#define MAX_NODES 50   //maximum number of junctions/shelters in the map 

//Status codes
//Functions return one of these to say whether they worked.

typedef enum {
    STATUS_OK        =  0,
    STATUS_ERROR     = -1,
    STATUS_FULL      = -2,
    STATUS_EMPTY     = -3,
    STATUS_INVALID   = -4,
    STATUS_NOT_FOUND = -5,
    STATUS_DUPLICATE = -6
} Status;

//Risk level (output of the Risk Engine)
typedef enum {
    RISK_NORMAL = 0,
    RISK_WATCH,
    RISK_WARNING,
    RISK_CRITICAL

} RiskLevel;

//Kind of sensor
typedef enum {
    SENSOR_RAIN = 0,      /* rainfall, in mm/hour */
    SENSOR_WATER_LEVEL    /* water level, in metres */
} SensorType;

//Kind of hazard
typedef enum {
    HAZARD_FLOOD = 0,
    HAZARD_LANDSLIDE
} HazardType;

// Condition of a road 
typedef enum {
    ROAD_OPEN = 0,
    ROAD_BLOCKED,
    ROAD_HAZARDOUS
} RoadStatus;

//One sensor reading
//Role 1 (Data Acquisition) produces it, Role 2 (Risk Engine) reads it.

typedef struct {
    int        sensor_id;
    SensorType type;
    double     value;        //mm/hour for rain, metres for water level
    long       timestamp;
    int        location_id;
} SensorReading;


//One citizen report
typedef struct {
    int        report_id;
    int        location_id;
    HazardType hazard;
    int        severity;     //1 to 5
    long       timestamp;
} CitizenReport;

// One alert 
typedef struct {
    int       alert_id;
    int       location_id;
    RiskLevel level;
    double    confidence;    //0.0 to 1.0
    int       priority;      //bigger number = handled first
    long      timestamp;
} Alert;

#endif //COMMON_H