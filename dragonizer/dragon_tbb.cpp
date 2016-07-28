/*
 * dragon_tbb.c
 *
 *  Created on: 2011-08-17
 *      Author: Francis Giraldeau <francis.giraldeau@gmail.com>
 */

#include <iostream>

extern "C" {
#include "dragon.h"
#include "color.h"
#include "utils.h"
}
#include "dragon_tbb.h"
#include "tbb/tbb.h"

using namespace std;
using namespace tbb;

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

class DragonLimits {
private:
	piece_t piece;
public:
	DragonLimits() {
		piece_init(&piece);
	}
    DragonLimits(DragonLimits &dragon, split) {
        (void) dragon;
		piece_init(&piece);
	}
	void operator()(const blocked_range<uint64_t> &r) {
		piece_limit(r.begin(), r.end(), &piece);
	}
	void join(DragonLimits &other) {
		piece_merge(&piece, other.getPiece());
	}
	const piece_t getPiece() const {
		return piece;
	}
};

typedef spin_rw_mutex Mutex;

Mutex coutMutex;

class DragonDraw {
private:
	struct draw_data *data;
public:
	DragonDraw(struct draw_data *input) : data(input) {}
	void operator() (const blocked_range<uint64_t> &r) const {
		uint64_t n1, n2;
		int i;
		struct draw_data d = *data;

		n1 = r.begin();
		n2 = r.end();

		int start_id = n1 * d.nb_thread / d.size;
		int end_id = n2 * d.nb_thread / d.size;
		if (start_id == end_id) {
			dragon_draw_raw(n1, n2, d.dragon, d.dragon_width, d.dragon_height, d.limits, start_id);
			return;
		}

		uint64_t end_1 = (start_id  + 1) * d.size / d.nb_thread;
		dragon_draw_raw(n1, end_1, d.dragon, d.dragon_width, d.dragon_height, d.limits, start_id);

		for(i = start_id + 1; i < end_id; i++) {
			uint64_t start = i * d.size / d.nb_thread;
			uint64_t end = (i + 1) * d.size / d.nb_thread;
			dragon_draw_raw(start, end, d.dragon, d.dragon_width, d.dragon_height, d.limits, i);
		}

		uint64_t start_1 = end_id * d.size / d.nb_thread;
		dragon_draw_raw(start_1, n2, d.dragon, d.dragon_width, d.dragon_height, d.limits, end_id);
	}
};

class DragonRender {
private:
	struct draw_data *data;
public:
	DragonRender(struct draw_data *input) : data(input) {}
	void operator() (const blocked_range<int> &r) const {
		// scale dragon into image
		int y1, y2;
		struct draw_data d = *data;

		y1 = r.begin();
		y2 = r.end();
		scale_dragon(y1, y2, d.image, d.image_width, d.image_height,
		        d.dragon, d.dragon_width, d.dragon_height, d.palette);
	}
};

class DragonClear {
private:
	struct draw_data *data;
public:
	DragonClear(struct draw_data *input) : data(input) {}
	void operator()(const blocked_range<int> &r) const {
		int i;
		char *dragon = data->dragon;
		for (i = r.begin(); i < r.end(); i++) {
			dragon[i] = -1;
		}
	}
};

int dragon_draw_tbb(char **canvas, struct rgb *image, int width, int height, uint64_t size, int nb_thread)
{
	struct draw_data data;
	limits_t limits;
	char *dragon = NULL;
	int dragon_width;
	int dragon_height;
	int dragon_surface;
	int scale_x;
	int scale_y;
	int scale;
	int deltaJ;
	int deltaI;

	struct palette *palette = init_palette(nb_thread);
	if (palette == NULL)
		return -1;

	/* 1. Calculer les limites du dragon */
	dragon_limits_tbb(&limits, size, nb_thread);

	task_scheduler_init init(nb_thread);

	dragon_width = limits.maximums.x - limits.minimums.x;
	dragon_height = limits.maximums.y - limits.minimums.y;
	dragon_surface = dragon_width * dragon_height;
	scale_x = dragon_width / width + 1;
	scale_y = dragon_height / height + 1;
	scale = (scale_x > scale_y ? scale_x : scale_y);
	deltaJ = (scale * width - dragon_width) / 2;
	deltaI = (scale * height - dragon_height) / 2;

	dragon = (char *) malloc(dragon_surface);
	if (dragon == NULL) {
		free_palette(palette);
		return -1;
	}

	data.nb_thread = nb_thread;
	data.dragon = dragon;
	data.image = image;
	data.size = size;
	data.image_height = height;
	data.image_width = width;
	data.dragon_width = dragon_width;
	data.dragon_height = dragon_height;
	data.limits = limits;
	data.scale = scale;
	data.deltaI = deltaI;
	data.deltaJ = deltaJ;
	data.palette = palette;
	data.tid = (int *) calloc(nb_thread, sizeof(int));

	/* 2. Initialiser la surface */
	DragonClear dragonClear(&data);
	parallel_for(blocked_range<int>(0,dragon_surface), dragonClear);

	/* 3. Dessiner le dragon */
	DragonDraw dragonDraw(&data);
	parallel_for(blocked_range<uint64_t>(0, size), dragonDraw);

	/* 4. Effectuer le rendu final */
	DragonRender dragonRender(&data);
	parallel_for(blocked_range<int>(0,height), dragonRender);

	init.terminate();

	free_palette(palette);
	FREE(data.tid);
	*canvas = dragon;
	return 0;
}

/*
 * Calcule les limites en terme de largeur et de hauteur de
 * la forme du dragon. Requis pour allouer la matrice de dessin.
 */
int dragon_limits_tbb(limits_t *limits, uint64_t size, int nb_thread)
{
	DragonLimits lim;

	task_scheduler_init init(nb_thread);
	parallel_reduce(blocked_range<uint64_t>(0,size), lim);
	init.terminate();

	piece_t piece = lim.getPiece();
	*limits = piece.limits;
	return 0;
}
