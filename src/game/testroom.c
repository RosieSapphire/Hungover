#include <GL/gl.h>
#include <GL/gl_integration.h>

#include "engine/camera.h"
#include "engine/vector.h"
#include "engine/config.h"
#include "engine/util.h"
#include "engine/player.h"

#include "game/testroom.h"

static bool is_loaded;
static struct scene *scene;
static struct player p;

/**
 * _testroom_load - Load assets for Testroom
 *
 * Description: Loads the assets for the Testroom scene
 */
static void _testroom_load(void)
{
	if (is_loaded)
		return;

	scene = scene_load("rom:/test.scene");

	player_init(&p);

	glDisable(GL_BLEND);
	glColor3f(1, 1, 1);
	projection_setup();

	is_loaded = true;
}

/**
 * testroom_update - Update Testroom and Player
 * @uparms: Input Parameters
 *
 * Description: Updates Testroom scene and player character
 * Return: Desired next scene
 */
enum scene_index testroom_update(struct update_parms uparms)
{
	scene_update(scene);
	player_update(&p, scene, uparms);

	return (SCENE_TESTROOM);
}

/**
 * testroom_draw - Draw Testroom Scene
 * @subtick: Delta value between frames
 *
 * Description: Draws Testroom scene with player camera
 */
void testroom_draw(float subtick)
{
	_testroom_load();

	gl_context_begin();
	glClearColor(0.2f, 0.1f, 0.05f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	player_view_matrix_setup(&p, subtick);
	scene_draw(scene, subtick);
	player_item_draw(&p, subtick);

	gl_context_end();
}
