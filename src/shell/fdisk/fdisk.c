#include "../../term/vga.h"
#include "../../term/term.h"
#include "../../term/cursor.h"
#include "../../kb/kb.h"
#include "../../cstdlib/string.h"
#include "../../util/debug.h"
#include "../../disk/disk.h"

#include "fdisk.h"

void fdisk_shell() {
    vga_flush_up(0, 25, 1, 1);
}
