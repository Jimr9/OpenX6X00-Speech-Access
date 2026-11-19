/*
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 *
 *  OpenX6X00 - the multi-boot card's start-up settings (TomekR1's Xiegu6x00-Multi-Boot with the talking menu)
 *
 *  The card's BOOT partition (/dev/mmcblk0p1, FAT) holds menu.txt (the systems: key, name, root partition),
 *  bootmenu.env (the screen menu: bm_default, bm_secs) and talkmenu.txt (the talking menu: default, wait).
 *  A one-time start is a mark in the card's sector 2 ("BM" and the key's digit): the boot script starts that
 *  system and clears the mark.
 */

#pragma once

#include <stdbool.h>

#define BOOTMENU_MAX 5

typedef struct {
    int  n;                         /* entries on the card (F1..Fn), 0: not a multi-boot card */
    char name[BOOTMENU_MAX][40];    /* "F2 OpenX6X00 speech" as menu.txt has it, without the key */
    int  self;                      /* this system's key (1..n), 0 unknown */
    int  talk;                      /* the talking menu's key, 0 none */
    int  screen_default;            /* bm_default */
    int  screen_secs;               /* bm_secs */
    int  talk_default;              /* talkmenu.txt default= (1..n) */
    int  talk_wait;                 /* talkmenu.txt wait= seconds, 0: waits for a key */
} bootmenu_t;

#ifdef __cplusplus
extern "C" {
#endif

/* Read the card (mounted for the moment): false when it is not a multi-boot card */
bool bootmenu_read(bootmenu_t *b);

bool bootmenu_set_screen(int default_key, int secs);       /* bootmenu.env; -1 leaves a value as it is */
bool bootmenu_set_talk(int default_key, int wait_s);       /* talkmenu.txt; -1 leaves a value as it is */

/* The next start only: that key's system, then the default again. Restarts the radio */
bool bootmenu_restart_into(int key);

#ifdef __cplusplus
}
#endif
