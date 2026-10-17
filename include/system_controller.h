/* system_controller.h - functions of the main controller (Role 6) */

#ifndef SYSTEM_CONTROLLER_H
#define SYSTEM_CONTROLLER_H

/* These two lines let the C++ files use these C functions */
#ifdef __cplusplus
extern "C" {
#endif

void system_init(void);   /* set everything to the starting values */
void system_run(void);    /* show the menu again and again until exit */

#ifdef __cplusplus
}
#endif

#endif
