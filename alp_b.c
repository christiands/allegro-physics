/*
	Allegro Physics (alp)
	A simulation of a ball bouncing around in Allegro
*/

#define GRAV_C 9.8f /* Gravitational constant */
#define ACRS_C 1.0f /* Across movement constant */
#define REFR_R 60.0f /* Refresh rate */
#define FRIC_C 2.1f /* Friction coefficient */
#define TERM_VX 10.0f /* Terminal velocity */
#define TERM_VY 25.0f

#define WIN_H 720 /* Window height */
#define WIN_L 1280 /* Window length */

#define BDR_L 50 /* Left border offset */
#define BDR_R 50 /* Right border offset */
#define BDR_T 20 /* Top border offset */
#define BDR_B 20 /* Bottom border offset*/

#define BLL_L 128 /* Ball length */
#define BLL_H 128 /* Ball height */
#define BLL_SX 100 /* Ball starting point */
#define BLL_SY 100
#define BLL_SDH 1 /* Ball starting direction */
#define BLL_SDV 1

#include <stdio.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>

typedef struct
{
	float x;
	float y;
} COORD;

typedef struct
{
	int x; /* 1 if heading right, -1 if heading left */
	int y; /* 1 if heading down, -1 if heading top */
} DIRECTION;

typedef struct
{
	float x;
	float y;
} VELOCITY;

typedef struct
{
	COORD c;
	DIRECTION d;
	VELOCITY v;
} VECTOR;

ALLEGRO_TIMER *ti;
ALLEGRO_EVENT_QUEUE *qe;
ALLEGRO_DISPLAY *ds;
ALLEGRO_KEYBOARD_STATE kb;

ALLEGRO_BITMAP *bg;
ALLEGRO_BITMAP *bl;

ALLEGRO_FONT *fn;

int slow = 0; /* Slows the ball down on bounce, requiring reacceleration */

int64_t drop_stx = 0; /* Drop timer start */
int64_t drop_ttx = 0; /* Drop timer temporary */
int64_t drop_sty = 0; /* Drop timer start */
int64_t drop_tty = 0; /* Drop timer temporary */

VECTOR accel_calc(VECTOR curr, float accel_con, float acrss_con, float drop_tx, float drop_ty);
VECTOR coll_lgc(VECTOR curr, int bord_l, int bord_r, int bord_t, int bord_b, int wind_h, int wind_l, int ball_h, int ball_l); /* Returns a correction if true */

void res_t(char axis); /* Reset acceleration timer */
int init_a(); /* Initialize add-ons */
int init_b(); /* Initialize bitmaps */
int dest_a(); /* Destroy add-ons */
int dest_b(); /* Destroy bitmaps */

VECTOR accel_calc(VECTOR curr, float accel_con, float acrss_con, float drop_tx, float drop_ty)
{
	VECTOR t = curr;
	t.v.x = (acrss_con * t.d.x) * drop_tx; /* Velocity */
	t.v.y = (accel_con * t.d.y) * drop_ty; /* v = a * s * d */

	if((t.v.x * t.d.x) > TERM_VX)
		t.v.x = (TERM_VX * t.d.x);

	if((t.v.y * t.d.y) > TERM_VY)
		t.v.y = (TERM_VY * t.d.y);

	t.c.x = (t.c.x + t.v.x);
	t.c.y = (t.c.y + t.v.y);

	return t;
}

VECTOR coll_lgc(VECTOR curr, int bord_l, int bord_r, int bord_t, int bord_b, int wind_h, int wind_l, int ball_h, int ball_l)
{
	VECTOR t = curr;

	if(curr.c.x < bord_l) /* Left side collision */
	{
		t.c.x = bord_l;
		t.d.x = 1;
		if(slow)
			res_t('x');
	}
	else if(curr.c.x > wind_l - bord_r - ball_l) /* Right side collision */
	{
		t.c.x = wind_l - bord_r - ball_l;
		t.d.x = -1;
		if(slow)
			res_t('x');
	}

	if(curr.c.y < bord_t) /* Top collision */
	{
		t.c.y = bord_t;
		t.d.y = 1;
		if(slow)			
			res_t('y');
	}
	else if(curr.c.y > wind_h - bord_b - ball_h) /* Bottom collision */
	{	
		t.c.y = wind_h - bord_b - ball_h;
		t.d.y = -1;
		if(slow)
			res_t('y');
	}

	return t;
}

void res_t(char axis)
{
	if(axis == 'x')
	{
		drop_stx = al_get_timer_count(ti);
		drop_ttx = al_get_timer_count(ti);
	}
	else if(axis == 'y')
	{
	drop_sty = al_get_timer_count(ti);
	drop_tty = al_get_timer_count(ti);
	}
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
	int64_t drop_dix = 0; /* Drop timer diff */
	int64_t drop_diy = 0; /* Drop timer diff */
	float drop_scx = 0; /* Drop second count */
	float drop_scy = 0; /* Drop second count */

	VECTOR b; /* Ball vector */
	b.c.x = BLL_SX;
	b.c.y = BLL_SY;
	b.d.x = BLL_SDH;
	b.d.y = BLL_SDV;
	b.v.x = 0;
	b.v.y = 0;

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
					drop_stx = al_get_timer_count(ti);
					drop_ttx = al_get_timer_count(ti);
					drop_sty = al_get_timer_count(ti);
					drop_tty = al_get_timer_count(ti);

					if(!drop)
						drop = 1;
					else
						drop = 0;
				}
				if(ev.keyboard.keycode == ALLEGRO_KEY_ESCAPE)
					done = 1;
				if(ev.keyboard.keycode == ALLEGRO_KEY_S)
				{
					if(!slow)
						slow = 1;
					else
						slow = 0;
				}
				break;
		}

		if(done)
			break;

		if(redraw && al_is_event_queue_empty(qe))
		{
			if(drop)
			{
				drop_ttx = al_get_timer_count(ti);
				drop_tty = al_get_timer_count(ti);

				drop_dix = drop_ttx - drop_stx;
				drop_diy = drop_tty - drop_sty;

				drop_scx = ((float)drop_dix) / REFR_R;
				drop_scy = ((float)drop_diy) / REFR_R;

				b = coll_lgc(b, BDR_L, BDR_R, BDR_T, BDR_B, WIN_H, WIN_L, BLL_H, BLL_L);
				b = accel_calc(b, GRAV_C, ACRS_C, drop_scx, drop_scy);
			}

			al_clear_to_color(al_map_rgb(160, 160, 160));
			al_draw_bitmap(bg, 0, 0, 0);
			al_draw_bitmap(bl, b.c.x, b.c.y, 0);
			al_draw_textf(fn, al_map_rgb(255, 255, 255), 0, 0, 0, "time(x): %fs | time(y): %fs | vel(x): %fp/s | vel(y): %fp/s | drop: %d | scroll: %d", drop_scx, drop_scy, b.v.x, b.v.y, drop, slow);
			al_draw_textf(fn, al_map_rgb(255, 255, 255), 0, WIN_H - 10, 0, "b.c.x: %f | b.c.y: %f | b.d.x: %d | b.d.y: %d", b.c.x, b.c.y, b.d.x, b.d.y);
			al_flip_display();

			redraw = 0;
		}
	}

	dest_a();
	dest_b();

	return 0;
}