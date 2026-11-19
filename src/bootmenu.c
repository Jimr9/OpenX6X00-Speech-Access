/*
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 *
 *  OpenX6X00 - the multi-boot card's start-up settings (bootmenu.h)
 */

#include "bootmenu.h"

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <unistd.h>

#define BOOT_DEV  "/dev/mmcblk0p1"
#define CARD_DEV  "/dev/mmcblk0"
#define MNT       "/tmp/bootmenu"
#define ONCE_LBA  2                 /* the card's sector the boot script looks at */

static bool mnt(void) {
    mkdir(MNT, 0755);
    umount(MNT);
    return mount(BOOT_DEV, MNT, "vfat", MS_NOATIME, NULL) == 0;
}

static void unmnt(void) {
    sync();
    umount(MNT);
}

/* A "key=value" file's value, -1 when not there */
static int file_int(const char *path, const char *key) {
    FILE  *f = fopen(path, "r");
    char   line[256];
    size_t kl = strlen(key);
    int    v  = -1;

    if (!f) return -1;
    while (fgets(line, sizeof(line), f)) {
        if (!strncmp(line, key, kl) && line[kl] == '=') {
            v = atoi(line + kl + 1);
        }
    }
    fclose(f);
    return v;
}

/* key=value set in the file, the other lines kept (CR LF or LF, as they were) */
static bool file_set(const char *path, const char *key, int value) {
    char   out[4096] = "", line[256];
    size_t kl = strlen(key), o = 0;
    bool   done = false;
    FILE  *f = fopen(path, "r");

    if (f) {
        while (fgets(line, sizeof(line), f)) {
            if (!strncmp(line, key, kl) && line[kl] == '=') {
                const char *eol = strchr(line, '\r') ? "\r\n" : "\n";
                o += snprintf(out + o, sizeof(out) - o, "%s=%i%s", key, value, eol);
                done = true;
            } else {
                o += snprintf(out + o, sizeof(out) - o, "%s", line);
            }
            if (o >= sizeof(out) - 1) {
                fclose(f);
                return false;
            }
        }
        fclose(f);
    }
    if (!done) {
        if (o && out[o - 1] != '\n') o += snprintf(out + o, sizeof(out) - o, "\n");
        o += snprintf(out + o, sizeof(out) - o, "%s=%i\n", key, value);
    }
    f = fopen(path, "w");
    if (!f) return false;
    fwrite(out, 1, o, f);
    fflush(f);
    fsync(fileno(f));
    fclose(f);
    return true;
}

/* This system's root partition, from the kernel's command line */
static int own_partition(void) {
    FILE *f = fopen("/proc/cmdline", "r");
    char  line[1024] = "";
    char *p;

    if (!f) return 0;
    if (!fgets(line, sizeof(line), f)) line[0] = '\0';
    fclose(f);
    p = strstr(line, "root=/dev/mmcblk0p");
    return p ? atoi(p + strlen("root=/dev/mmcblk0p")) : 0;
}

bool bootmenu_read(bootmenu_t *b) {
    char  line[256];
    FILE *f;
    int   own = own_partition();

    memset(b, 0, sizeof(*b));
    if (!mnt()) return false;
    /* menu.txt: "F2  OpenX6X00 speech  (SYS2, root /dev/mmcblk0p5)" */
    f = fopen(MNT "/menu.txt", "r");
    if (f && access(MNT "/bootmenu.env", F_OK) == 0) {
        while (fgets(line, sizeof(line), f)) {
            int   key;
            char *name, *end, *root;

            if (line[0] != 'F' || !isdigit((unsigned char)line[1])) continue;
            key = line[1] - '0';
            if (key < 1 || key > BOOTMENU_MAX) continue;
            name = line + 2;
            while (*name == ' ') name++;
            end = strstr(name, "  (");
            if (!end) end = name + strcspn(name, "\r\n");
            snprintf(b->name[key - 1], sizeof(b->name[0]), "%.*s", (int)(end - name), name);
            root = strstr(line, "root /dev/mmcblk0p");
            if (root && own && atoi(root + strlen("root /dev/mmcblk0p")) == own) b->self = key;
            if (!strncasecmp(b->name[key - 1], "Talking menu", 12)) b->talk = key;
            if (key > b->n) b->n = key;
        }
    }
    if (f) fclose(f);
    if (b->n) {
        b->screen_default = file_int(MNT "/bootmenu.env", "bm_default");
        b->screen_secs    = file_int(MNT "/bootmenu.env", "bm_secs");
        b->talk_default   = file_int(MNT "/talkmenu.txt", "default");
        b->talk_wait      = file_int(MNT "/talkmenu.txt", "wait");
        if (b->talk_wait < 0) b->talk_wait = 0;
        if (b->talk_default < 1 || b->talk_default > b->n) b->talk_default = 1;
    }
    unmnt();
    return b->n > 0;
}

bool bootmenu_set_screen(int default_key, int secs) {
    bool ok;

    if (!mnt()) return false;
    ok = true;
    if (default_key >= 1) ok = file_set(MNT "/bootmenu.env", "bm_default", default_key) && ok;
    if (secs >= 0) ok = file_set(MNT "/bootmenu.env", "bm_secs", secs) && ok;
    unmnt();
    return ok;
}

bool bootmenu_set_talk(int default_key, int wait_s) {
    bool ok;

    if (!mnt()) return false;
    ok = true;
    if (default_key >= 1) ok = file_set(MNT "/talkmenu.txt", "default", default_key) && ok;
    if (wait_s >= 0) ok = file_set(MNT "/talkmenu.txt", "wait", wait_s) && ok;
    unmnt();
    return ok;
}

bool bootmenu_restart_into(int key) {
    unsigned char sec[512] = {0};
    int           fd = open(CARD_DEV, O_RDWR | O_SYNC);
    bool          ok;

    if (fd < 0 || key < 1 || key > BOOTMENU_MAX) {
        if (fd >= 0) close(fd);
        return false;
    }
    sec[0] = 'B';
    sec[1] = 'M';
    sec[2] = (unsigned char)('0' + key);
    ok = pwrite(fd, sec, sizeof(sec), ONCE_LBA * 512) == (ssize_t)sizeof(sec);
    fsync(fd);
    close(fd);
    if (ok) {
        sync();
        return system("reboot") == 0;
    }
    return false;
}
