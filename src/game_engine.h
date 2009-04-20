/**
 * Header for code common to FreeDink and FreeDinkedit

 * Copyright (C) 1997, 1998, 1999, 2002, 2003  Seth A. Robinson
 * Copyright (C) 2008, 2009  Sylvain Beucler

 * This file is part of GNU FreeDink

 * GNU FreeDink is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of the
 * License, or (at your option) any later version.

 * GNU FreeDink is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#ifndef _GAME_ENGINE_H
#define _GAME_ENGINE_H

#include "SDL.h"
#include "rect.h"
#include "io_util.h"
#include "dinkc.h"
#include "gfx_tiles.h"
#include "dinkc_sp_custom.h"

#define MAX_SPRITES_AT_ONCE 300


struct sp
{
  int x,moveman;
  int y; 
  int mx,my;
  int lpx[51],lpy[51];
  int speed;
  int brain;
  int seq_orig,dir;
  int seq;
  int frame;
  unsigned long delay;
  int pseq;
  int pframe;
  
  /*BOOL*/int active;
  int attrib;
  unsigned long wait;
  int timer;
  int skip;
  int skiptimer;
  int size;
  int que;
  int base_walk;
  int base_idle;
  int base_attack;
  
  int base_hit;
  int last_sound;
  int hard;
  rect alt;
  int althard;
  int sp_index;
  /*BOOL*/int nocontrol;
  int idle;
  int strength;
  int damage;
  int defense;
  int hitpoints;
  int exp;
  int gold;
  int base_die;
  int kill;
  Uint32 kill_timer;
  int script_num;
  char text[200];
  int owner;
  int script;
  int sound;
  int callback;
  int freeze;
  /*bool*/int move_active;
  int move_script;
  int move_dir;
  int move_num;
  /*BOOL*/int move_nohard;
  int follow;
  int nohit;
  /*BOOL*/int notouch;
  unsigned long notouch_timer;
  /*BOOL*/int flying;
  int touch_damage;
  int brain_parm;
  int brain_parm2;
  /*BOOL*/int noclip;
  /*BOOL*/int reverse;
  /*BOOL*/int disabled;
  int target;
  int attack_wait;
  int move_wait;
  int distance;
  int last_hit;
  /*BOOL*/int live;
  int range;
  int attack_hit_sound;
  int attack_hit_sound_speed;
  int action;
  int nodraw;
  int frame_delay;
  int picfreeze;
  /* v1.08 */
  int bloodseq;
  int bloodnum;
  dinkc_sp_custom custom;
};

struct item_struct
{
  BOOL_1BYTE active;
  char name[10+1];
  int seq;
  int frame;
};

struct mydata
{
  unsigned char type[100];  // DinkC's editor_type(i)
  unsigned short seq[100];  // DinkC's editor_seq(i)
  unsigned char frame[100]; // DinkC's editor_frame(i)
  int last_time; // ticks when type 6, 7 or 8 was set
};

/* Game state. Saved games are generated by dumping this structure in
   to SAVEx.DAT, so DON'T DON'T DON'T change anything here ;)
   otherwise you'll change the saved games format. */
// for storing current tiles in save game
struct player_info_tile
{
  char file[50];
};
#define NB_MITEMS 8
#define NB_ITEMS 16
struct player_info
{
  int version;
  char gameinfo[196];
  int minutes;
  int die, size, defense, dir, pframe, pseq, seq, frame, strength,
    base_walk, base_idle, base_hit, que;
  
  struct item_struct mitem[NB_MITEMS];
  struct item_struct item[NB_ITEMS];
  
  int curitem; // highlighted item in the inventory
  BOOL_1BYTE item_magic; // 1 if it's a magic item, 0 if regular

  int counter;
  BOOL_1BYTE idle;
  struct mydata spmap[769];
  struct varman var[MAX_VARS];
  
  
  BOOL_1BYTE push_active;
  int push_dir;
  unsigned int push_timer;
  int last_talk;
  int mouse; /* vertical position of the mouse when selecting a dialog
		option */
  int last_map;
  int crap;
  int buff[95];
  unsigned int dbuff[20];
  
  int lbuff[10];
  
  /* v1.08: use wasted space for storing file location of map.dat,
     dink.dat, palette, and tiles */
  /* char cbuff[6000];*/
  char mapdat[50];
  char dinkdat[50];
  char palette[50];
  struct player_info_tile tile[GFX_TILES_NB_SETS+1];
  struct global_function func[100];
};

extern struct sp spr[];
extern struct player_info play;
extern int last_sprite_created;

/* Engine variables directly mapped with DinkC variables */
extern int *pvision, *plife, *presult, *pspeed, *ptiming, *plifemax,
  *pexper, *pmap, *pstrength, *pcur_weapon,*pcur_magic, *pdefense,
  *pgold, *pmagic, *plevel, *plast_text, *pmagic_level;
extern int *pupdate_status, *pmissile_target, *penemy_sprite,
  *pmagic_cost, *pmissle_source;


extern int flife, fexp, fstrength, fdefense, fgold, fmagic,
  fmagic_level, flifemax, fraise, last_magic_draw;



/* Sound - BGM */
extern /*bool*/int midi_active;
extern /*bool*/int sound_on;
extern /*bool*/int cd_inserted;


/* dink.dat */
struct map_info
{
  int loc[769];
  int music[769];
  int indoor[769];
};
extern struct map_info map;



/* Joystick */
extern /*BOOL*/int joystick;
/* extern JOYINFOEX jinfo; */
extern SDL_Joystick *jinfo;
struct wait_for_button
{
	int script;
	int button;
	/*bool*/int active;
};
extern struct wait_for_button wait4b;


extern int last_saved_game;

extern char *dversion_string;
extern int dversion;
#define LEN_SAVE_GAME_INFO 200
extern char save_game_info[LEN_SAVE_GAME_INFO];
extern char current_map[50];
extern char current_dat[50];

extern time_t time_start;

extern int smooth_follow;

extern int get_pan(int h);
extern int get_vol(int h);


extern int process_warp;
extern int process_downcycle;
extern int process_upcycle;
extern unsigned long cycle_clock;
extern int cycle_script;

extern unsigned int dink_base_push;

extern void game_init(void);
extern void game_quit(void);

#endif
