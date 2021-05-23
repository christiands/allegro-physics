/*
	Allegro Physics (alp)
	A simulation of a ball bouncing around in Allegro
*/

#define GRAV_C 9.8f /* Gravitational constant */
#define REFR_R 60.0f /* Refresh rate */
#define FRIC_C 2.1f /* Friction coefficient */

#define WIN_H 720 /* Window height */
#define WIN_L 1280 /* Window length */

#define BDR_L 50 /* Left border offset */
#define BDR_R 50 /* Right border offset */
#define BDR_T 20 /* Top border offset */
#define BDR_B 20 /* Bottom border offset*/

#define BLL_L 128 /* Ball length */
#define BLL_W 128 /* Ball width */
#define BLL_SX 100 /* Ball starting point */
#define BLL_SY 100

#include <stdio.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>

typedef struct
{
	float x;
	float y;
} COORD;

ALLEGRO_TIMER *ti;
ALLEGRO_EVENT_QUEUE *qe;
ALLEGRO_DISPLAY *ds;
ALLEGRO_KEYBOARD_STATE kb;

ALLEGRO_BITMAP *bg;
ALLEGRO_BITMAP *bl;

ALLEGRO_FONT *fn;

int scroll = 0;

// COORD boun_calc(float accel, float time_fl, float frict, int x_dir, int y_dir);
COORD coll_chk(float x, float y); /* Returns a correction if true */
int init_a(); /* Initialize add-ons */
int init_b(); /* Initialize bitmaps */
int dest_a(); /* Destroy add-ons */
int dest_b(); /* Destroy bitmaps */

COORD accel_calc(float x, float y, float accel_con, float drop_t)
{
	COORD t;
	
	y = y + (accel_con * drop_t);

	t.x = x;
	if(y > WIN_H && scroll)
		t.y = (float)((int)y % WIN_H);
	else
		t.y = y;
	return t;
}

COORD coll_chk(float x, float y)
{
	COORD t;

	if(x < BDR_L)
		x = BDR_L;

	if(x > WIN_L - BDR_R)
		x = WIN_L - BDR_R;

	if(y < BDR_T)
		y = BDR_T;

	if(y > WIN_H - BDR_B)
		y = WIN_H - BDR_B;

	t.x = x;
	t.y = y;
	return t;
}

int init_a()
{
	al_init();

	al_install_keyboard();
	al_init_image_addon();

	ti = al_create_timer(1.0 / REFR_R);
	if(!ti)
	{
		fprintf(stderr, "Could not initialize ALLEGRO_TIMER\n");
		return 1;
	}

	qe = al_create_event_queue();
	if(!qe)
	{
		fprintf(stderr, "Could not initialize ALLEGRO_EVENT_QUEUE\n");
		return 1;
	}

	ds = al_create_display(WIN_L, WIN_H);
	if(!ds)
	{
		fprintf(stderr, "Could not initialize ALLEGRO_DISPLAY\n");
		return 1;
	}

	fn = al_create_builtin_font();
	if(!fn)
	{
		fprintf(stderr, "Could not initialize ALLEGRO_FONT\n");
		return 1;
	}

	al_register_event_source(qe, al_get_keyboard_event_source());
	al_register_event_source(qe, al_get_timer_event_source(ti));
	al_register_event_source(qe, al_get_display_event_source(ds));
	
	return 0;
}

int init_b()
{
	bg = al_load_bitmap("assets/border.png");

	if(!bg)
	{
		fprintf(stderr, "Could not load 'background.png'\n");
		return 1;
	}

	bl = al_load_bitmap("assets/ball.png");
	
	if(!bl)
	{
		fprintf(stderr, "Could not load 'ball.png'\n");
		return 1;
	}

	return 0;
}

int dest_a()
{
	al_destroy_display(ds);
	al_destroy_timer(ti);
	al_destroy_event_queue(qe);
	al_destroy_font(fn);

	return 0;
}

int dest_b()
{
	al_destroy_bitmap(bg);
	al_destroy_bitmap(bl);

	return 0;
}

int main(void)
{
	init_a();
	init_b();

	int done = 0;
	int redraw = 1;

	int drop = 0;
	int64_t drop_st = 0; /* Drop timer start */
	int64_t drop_tt = 0; /* Drop timer temporary */
	int64_t drop_di = 0; /* Drop timer diff */
	float drop_sc = 0; /* Drop second count */
	COORD b; /* Ball coordinates */
	b.x = BLL_SX;
	b.y = BLL_SY;

	ALLEGRO_EVENT ev;
	al_start_timer(ti);

	while(1)
	{
		al_wait_for_event(qe, &ev);

		switch(ev.type)
		{
			case ALLEGRO_EVENT_TIMER:
				redraw = 1;
				break;
			case ALLEGRO_EVENT_DISPLAY_CLOSE:
				done = 1;
				break;
			case ALLEGRO_EVENT_KEY_DOWN:
				if(ev.keyboard.keycode == ALLEGRO_KEY_D)
				{
					drop_st = al_get_timer_count(ti);
					drop_tt = al_get_timer_count(ti);
					drop = 1;
				}
				if(ev.keyboard.keycode == ALLEGRO_KEY_ESCAPE)
					done = 1;
				if(ev.keyboard.keycode == ALLEGRO_KEY_S)
				{
					if(!scroll)
						scroll = 1;
					else
						scroll = 0;
				}
				break;
		}

		if(done)
			break;

		if(redraw && al_is_event_queue_empty(qe))
		{
			if(drop)
			{
				drop_tt = al_get_timer_count(ti);
				drop_di = drop_tt - drop_st;
				drop_sc = ((float)drop_di) / REFR_R;
				b = accel_calc(b.x, b.y, GRAV_C, drop_sc);
			}

			al_clear_to_color(al_map_rgb(160, 160, 160));
			al_draw_bitmap(bg, 0, 0, 0);
			al_draw_bitmap(bl, b.x, b.y, 0);
			al_draw_textf(fn, al_map_rgb(255, 255, 255), 0, 0, 0, "ball_x: %f | ball_y: %f | time: %fs | speed: %fp/s | scroll: %d", b.x, b.y, drop_sc, GRAV_C * drop_sc, scroll);
			al_flip_display();

			redraw = 0;
		}
	}

	dest_a();
	dest_b();

	return 0;
}