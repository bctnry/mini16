#include "int8.h"
#include "../term/vga.h"




/*
    NOTE: only a dummy task_scheduler here. currently used to display time.
    This should be fixed after disk drivers & memory management is completed.
    It's okay we pause the progress of this part for now; the order of things
    getting done is much more important. For big projects we must not do things
    that looks cool but will hinder our progress in the future.
*/
unsigned short i = 0x30;
void task_scheduler() {
    vga_write_ch_c(i, 0x00|0x07, 0, 0);
    i++;
    if (i > 0x39) {
        i = 0x30;
    }
}

void init_task_scheduler() {
    // term_echo_str("Initializing task scheduler...");
    int8_load_handler_near((void*)task_scheduler);
    int8_register_near(65535);
}
