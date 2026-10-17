/* main.c - starting point of the JalRakshak program (Role 6) */

#include <stdio.h>
#include "common.h"
#include "system_controller.h"

int main(void) {
    system_init();
    system_run();
    printf("Program finished.\n");
    return 0;
}
