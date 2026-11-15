#ifndef SYSTEM_CONTROLLER_H
#define SYSTEM_CONTROLLER_H

#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif

void system_init(void);
void system_run(void);
void system_close(void);
int get_summary(double *score, int *level, int *place);
int get_history_count(void);
Alert get_history_item(int index);

#ifdef __cplusplus
}
#endif

#endif
