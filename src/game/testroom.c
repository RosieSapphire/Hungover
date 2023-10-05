#include <GL/gl_integration.h>

#include "engine/camera.h"
#include "engine/vector.h"
#include "engine/config.h"
#include "engine/util.h"
#include "engine/player.h"

#include "game/testroom.h"

static bool is_loaded = false;

static scene_t *scene = NULL;
static texture_t test_tex;
static player_t p;

static void _testroom_load(void)
{
	if(is_loaded)
		return;

	scene = scene_load("rom:/test.scene");
	test_tex = texture_create_file("rom:/test.ci4.sprite");

	player_init(&p);

	glColor3f(1, 1, 1);
	projection_setup();

	is_loaded = true;
}

enum scene_index testroom_update(update_parms_t uparms)
{
	scene_update(scene);
	player_update(&p, scene, uparms);

	return SCENE_TESTROOM;
}

void testroom_draw(float subtick)
{
	_testroom_load();

	gl_context_begin();
	glClearColor(0.2f, 0.1f, 0.05f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	player_view_matrix_setup(&p, subtick);
	scene_draw(scene, subtick, test_tex.id);

	gl_context_end();
}
