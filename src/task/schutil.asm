;; utilities for task scheduler.

    .186
    .model tiny
    .code


    ;; NOTE: (1) should not call from c. (2) should reach here thru near call.
    ;; mostly taken from MINIX 1 `kernel/mpx88.asm`
    public _schutil_save_state
_schutil_save_state:
    PUSH DS
    PUSH CS
    POP DS


