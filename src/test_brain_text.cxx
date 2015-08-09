/**
 * Test suite for text "brain"

 * Copyright (C) 2015  Sylvain Beucler

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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "brains.h"
#include "live_sprites_manager.h"
#include "game_engine.h"

class TestBrainText : public CxxTest::TestSuite {
public:
	void setUp() {
		live_sprites_manager_init();
	}
	void tearDown() {
	}
	
	void test_brain_text() {
		TS_ASSERT_EQUALS(add_sprite(0, 0, 0, 0, 0), 1);
		TS_ASSERT_EQUALS(add_sprite(0, 0, 0, 0, 0), 2);
		spr[2].x = 500;
		TS_ASSERT_EQUALS(add_sprite(0, 0, 0, 0, 0), 3);
		spr[3].brain = 8;
		spr[3].damage = -1;
		
		spr[3].owner = 2;
		spr[3].x = 0;
		TS_ASSERT_EQUALS(spr[2].active, 1);
		text_brain(3);
		TS_ASSERT_EQUALS(spr[3].x, 500);

		spr[3].owner = 10;
		spr[3].x = 0;
		TS_ASSERT_EQUALS(spr[10].active, 0);
		text_brain(3);
		TS_ASSERT_EQUALS(spr[3].x, 0);

		spr[3].move_active = 1;
		spr[3].move_dir = 6;
		spr[3].move_num = 1;
		spr[3].speed = 1;
		base_timing = 4;
		
		spr[3].x = 0;
		spr[3].owner = 1000;
		text_brain(3);
		TS_ASSERT_EQUALS(spr[3].x, 1);

		spr[3].x = 0;
		spr[3].owner = 1000;
		text_brain(3);
		TS_ASSERT_EQUALS(spr[3].x, 1); // reproducible
		
		// Shouldn't crash
		spr[3].x = 0;
		spr[3].owner = 1200;
		text_brain(3);
		TS_ASSERT_EQUALS(spr[3].x, 0);

		// Shouldn't crash
		spr[3].x = 0;
		spr[3].owner = 100000;
		text_brain(3);
		TS_ASSERT_EQUALS(spr[3].x, 0);
	}
};