/*
 * dragon_pthread.c
 *
 *  Created on: 2011-08-17
 *      Author: Francis Giraldeau <francis.giraldeau@gmail.com>
 */

#define _GNU_SOURCE
#include <stdlib.h>
#include <pthread.h>
#include <stdarg.h>
#include <string.h>

#include "dragon.h"
#include "color.h"
#include "dragon_pthread.h"

pthread_mutex_t mutex_stdout;

void printf_threadsafe(char *format, ...)
{
	va_list ap;

	va_start(ap, format);
	pthread_mutex_lock(&mutex_stdout);
	vprintf(format, ap);
	pthread_mutex_unlock(&mutex_stdout);
	va_end(ap);
}

void *dragon_draw_worker(void *data)
{
	struct draw_data d;
	int y1, y2;
	uint64_t n1, n2;

	if (data == NULL)
		return NULL;

	d = *(struct draw_data *) data;

	uint64_t area = d.dragon_width * d.dragon_height;
	y1 = d.id * area / d.nb_thread;
	y2 = (d.id + 1) * area / d.nb_thread;

	/* 1. Initialiser la surface */
	init_canvas(y1, y2, d.dragon, -1);

	// barrier
	pthread_barrier_wait(d.barrier);

	n1 = d.id * d.size / d.nb_thread;
	n2 = (d.id + 1) * d.size / d.nb_thread;

	/* 2. Dessiner le dragon */
	dragon_draw_raw(n1, n2, d.dragon, d.dragon_width, d.dragon_height, d.limits, d.id);

	// barrier
	pthread_barrier_wait(d.barrier);

	/* 3. Effectuer le rendu final */
	y1 = d.id * d.image_height / d.nb_thread;
	y2 = (d.id + 1) * d.image_height / d.nb_thread;
	scale_dragon(y1, y2, d.image, d.image_width, d.image_height,
	        d.dragon, d.dragon_width, d.dragon_height, d.palette);

	// barrier
	pthread_barrier_wait(d.barrier);

	return NULL;
}

int dragon_draw_pthread(char **canvas, struct rgb *image, int width, int height, uint64_t size, int nb_thread)
{
	pthread_t *threads = NULL;
	pthread_barrier_t barrier;
	limits_t limits;
	struct draw_data info;
	char *dragon = NULL;
	int i;
	int scale_x;
	int scale_y;
	struct draw_data *data = NULL;
	struct palette *palette = NULL;
	int ret = 0;
	int r;

	palette = init_palette(nb_thread);
	if (palette == NULL)
		goto err;

	if (pthread_barrier_init(&barrier, NULL, nb_thread) != 0) {
		printf("barrier init error\n");
		goto err;
	}

	/* 1. Calculer les limites du dragon */
	if (dragon_limits_pthread(&limits, size, nb_thread) < 0)
		goto err;

	info.dragon_width = limits.maximums.x - limits.minimums.x;
	info.dragon_height = limits.maximums.y - limits.minimums.y;

	if ((dragon = (char *) malloc(info.dragon_width * info.dragon_height)) == NULL) {
		printf("malloc error dragon\n");
		goto err;
	}

	if ((data = malloc(sizeof(struct draw_data) * nb_thread)) == NULL) {
		printf("malloc error data\n");
		goto err;
	}

	if ((threads = malloc(sizeof(pthread_t) * nb_thread)) == NULL) {
		printf("malloc error threads\n");
		goto err;
	}

	info.image_height = height;
	info.image_width = width;
	scale_x = info.dragon_width / width + 1;
	scale_y = info.dragon_height / height + 1;
	info.scale = (scale_x > scale_y ? scale_x : scale_y);
	info.deltaJ = (info.scale * width - info.dragon_width) / 2;
	info.deltaI = (info.scale * height - info.dragon_height) / 2;
	info.nb_thread = nb_thread;
	info.dragon = dragon;
	info.image = image;
	info.size = size;
	info.limits = limits;
	info.barrier = &barrier;
	info.palette = palette;
	info.dragon = dragon;
	info.image = image;

	/* 2. Lancement du calcul parallèle principal avec draw_dragon_worker */
	for (i = 0; i < nb_thread; i++) {
		data[i] = info;
		data[i].id = i;
		r = pthread_create(&threads[i], NULL, dragon_draw_worker, (void *) &data[i]);
		if (r != 0) {
			printf("create thread error\n");
			goto err;
		}
	}

	/* 3. Attendre la fin du traitement */
	for (i = 0; i < nb_thread; i++) {
		r = pthread_join(threads[i], NULL);
		if (r != 0) {
			printf("join thread error\n");
			goto err;
		}
		pthread_detach(threads[i]);
	}

	if (pthread_barrier_destroy(&barrier) != 0) {
		printf("barrier destroy error\n");
		goto err;
	}

done:
	FREE(data);
	FREE(threads);
	free_palette(palette);
	*canvas = dragon;
	return ret;

err:
	FREE(dragon);
	ret = -1;
	goto done;
}

void *dragon_limit_worker(void *data)
{
	struct limit_data *lim = (struct limit_data *) data;
	piece_limit(lim->start, lim->end, &lim->piece);
	return NULL;
}

/*
 * Calcule les limites en terme de largeur et de hauteur de
 * la forme du dragon. Requis pour allouer la matrice de dessin.
 */
int dragon_limits_pthread(limits_t *limits, uint64_t size, int nb_thread)
{
	int ret = 0;
	int i;
	pthread_t *threads = NULL;
	struct limit_data *thread_data = NULL;
	piece_t master;

	piece_init(&master);

	if ((threads = calloc(nb_thread, sizeof(pthread_t))) == NULL)
		goto err;

	if ((thread_data = calloc(nb_thread, sizeof(struct limit_data))) == NULL)
		goto err;

	for (i = 0; i < nb_thread; i++) {
		struct limit_data d;
		d.id = i;
		d.start = i * size / nb_thread;
		d.end = (i + 1) * size / nb_thread;
		piece_init(&d.piece);
		thread_data[i] = d;
		pthread_create(&threads[i], NULL, dragon_limit_worker, (void *) &thread_data[i]);
	}

	for (i = 0; i < nb_thread; i++) {
		pthread_join(threads[i], NULL);
		pthread_detach(threads[i]);
	}

	/* either works for merging */
	/*
	for (i = nb_thread - 1 ; i > 0; i--) {
		piece_merge(&thread_data[i-1].piece, thread_data[i].piece);
	}
	*limits = thread_data[0].piece.limits;
	 */

	for (i = 0; i < nb_thread; i++) {
		piece_merge(&master, thread_data[i].piece);
	}

done:
	FREE(threads);
	FREE(thread_data);
	*limits = master.limits;
	return ret;
err:
	ret = -1;
	goto done;
}
