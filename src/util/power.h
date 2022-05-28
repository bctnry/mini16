#ifndef __MINI16_UTIL_POWER
#define __MINI16_UTIL_POWER

#include "visual_hint.h"

extern void power_bios_reboot();
extern void power_shutdown();
extern FAILURE_AT_ZERO unsigned short power_apm_chk();
extern FAILURE_AT_ZERO char power_apm_connect();
extern FAILURE_AT_NON_ZERO char power_apm_disconnect();
extern FAILURE_AT_NON_ZERO char power_apm_setver(
    // NOTE: if we pass `ver` as two separate args it'll
    // take up two 16-bit reg with openwatcom. might as well
    // use one to simplify stuff, we can directly uses AH
    // and AL in asm here anyway.
    unsigned short ver
);
extern FAILURE_AT_NON_ZERO char power_apm_enable_all();
extern FAILURE_AT_NON_ZERO char power_apm_set_state(
    unsigned char state
);

#define POWER_APM_STATE_ENABLED 0
#define POWER_APM_STATE_STANDBY 1
#define POWER_APM_STATE_SUSPEND 2
#define POWER_APM_STATE_OFF 3

#endif
