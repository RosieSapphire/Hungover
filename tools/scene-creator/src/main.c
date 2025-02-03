#define _DEFAULT_SOURCE
#include <dirent.h>
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_IMPLEMENTATION
#define NK_GLFW_GL3_IMPLEMENTATION
#include "nuklear.h"
#include "nuklear_glfw_gl3.h"

#define NK_VERTEX_BUFFER_SIZE (512 * 1024)
#define NK_INDEX_BUFFER_SIZE (128 * 1024)
#define NK_STR_MAX 512

#include "util.h"
#include "error.h"
#include "config.h"
#include "input.h"
#include "shader.h"
#include "scene.h"

#define MOUSE_TO_OBJECT_MOVE_SCALE 0.02f

#define OBJ_POS_COMP_MIN -0x7FFF
#define OBJ_POS_COMP_MAX 0x7FFF
#define OBJ_POS_COMP_STEP 0.01f

enum {
	MODE_OBJECT_SELECT,
	MODE_OBJECT_MOVE,
};

static GLFWwindow *glfw_window = NULL;
static struct nk_context *nk_ctx = NULL;

static input_t input = { 0 };
static mat4 proj_matrix, view_matrix;

scene_t scene;
static int object_selected = -1;

static shader_t mesh_shader;
static int32_t mesh_shader_uni_is_selected, mesh_shader_uni_light_col,
	mesh_shader_uni_light_dir;
static shader_t axis_shader;

static uint8_t mode_current = MODE_OBJECT_SELECT;
static int hide_helper_geo = 0;

static float cam_dist = 6.f;
static vec3 cam_foc = { 0, 0, 0 };
static vec2 cam_ang = { -90, 0 };
static vec3 cam_forw = GLM_VEC3_ZERO_INIT;
static vec3 cam_side = GLM_VEC3_ZERO_INIT;
static vec3 cam_up = GLM_VEC3_ZERO_INIT;

static void glfw_init(void)
{
	error_check(glfwInit(), GLFW_TRUE, "Failed to init GLFW.");
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
	glfw_window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_NAME,
				       NULL, NULL);
	glfwMakeContextCurrent(glfw_window);
	error_check(glfwGetCurrentContext() == glfw_window, 1,
		    "Window is not current context.");
}

static void glfw_terminate(void)
{
	glfwDestroyWindow(glfw_window);
	glfw_window = NULL;
	glfwTerminate();
}

static void glew_init(void)
{
	glewExperimental = 0;
	error_check(glewInit(), GLEW_OK, "Failed to init GLEW.");
	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);
}

static void nuklear_init(void)
{
	struct nk_font_atlas *atlas;

	nk_ctx = nk_glfw3_init(glfw_window, NK_GLFW3_INSTALL_CALLBACKS);
	nk_glfw3_font_stash_begin(&atlas);
	nk_glfw3_font_stash_end();
}

typedef struct {
	char path_abs[NK_STR_MAX];
	char path_rel[NK_STR_MAX];
} path_t;

static int num_model_paths = 0;
static path_t *model_paths;

static void model_path_search(void)
{
	const char *path = "assets/models";
	DIR *dir = opendir(path);

	if (!dir) {
		printf("Failed to read directory '%s'\n", path);
		exit(EXIT_FAILURE);
	}

	struct dirent *ent;

	model_paths = malloc(0);

	while ((ent = readdir(dir))) {
		if (ent->d_type == DT_REG &&
		    file_extension_check(ent->d_name, ".glb")) {
			model_paths =
				realloc(model_paths, sizeof *model_paths *
							     ++num_model_paths);
			path_t *mp = model_paths + num_model_paths - 1;
			snprintf(mp->path_abs, NK_STR_MAX, "%s/%s", path,
				 ent->d_name);
			snprintf(mp->path_rel, NK_STR_MAX, "%s", ent->d_name);
		}
	}

	closedir(dir);
}

static void nuklear_main_panel_importing(const char *import_path)
{
	nk_layout_row_dynamic(nk_ctx, 30, 1);
	if (!(nk_button_label(nk_ctx, "Import Model") &&
	      is_valid_path(import_path))) {
		return;
	}

	scene_import_model_to_object(&scene, import_path, &object_selected);
}

static void nuklear_main_panel(void)
{
	if (!nk_begin(nk_ctx, "Main", nk_rect(0, 0, 230, WINDOW_HEIGHT),
		      NK_WINDOW_BORDER | NK_WINDOW_MINIMIZABLE |
			      NK_WINDOW_TITLE)) {
		nk_end(nk_ctx);
		return;
	}

	static int model_selected = 0;

	nk_layout_row_dynamic(nk_ctx, 128, 1);
	if (nk_group_begin(nk_ctx, "Models",
			   NK_WINDOW_BORDER | NK_WINDOW_TITLE)) {
		nk_layout_row_dynamic(nk_ctx, 18, 1);
		for (int i = 0; i < num_model_paths; i++) {
			int is_cur = model_selected == i;
			if (nk_selectable_label(nk_ctx, model_paths[i].path_rel,
						NK_TEXT_LEFT, &is_cur)) {
				model_selected = i;
			}
		}
		nk_group_end(nk_ctx);
	}

	nuklear_main_panel_importing(model_paths[model_selected].path_abs);

	nk_layout_row_dynamic(nk_ctx, 128, 1);
	if (nk_group_begin(nk_ctx, "Objects",
			   NK_WINDOW_BORDER | NK_WINDOW_TITLE)) {
		nk_layout_row_dynamic(nk_ctx, 18, 2);
		for (unsigned int i = 0; i < scene.num_objects; i++) {
			int is_cur = (uint32_t)object_selected == i;
			char obj_str[NK_STR_MAX];
			snprintf(obj_str, NK_STR_MAX, "Obj %d", i);
			if (nk_selectable_label(nk_ctx, obj_str, NK_TEXT_LEFT,
						&is_cur)) {
				object_selected = i;
			}
			if (nk_button_label(nk_ctx, "Delete")) {
				scene_object_remove_from_list(&scene, i);
			}
		}
		nk_group_end(nk_ctx);
	}

	/* background */

	const char *bg_color_str = "BG Color:";
	nk_layout_row_dynamic(nk_ctx, 18, 1);
	nk_text(nk_ctx, bg_color_str, strlen(bg_color_str), NK_TEXT_LEFT);
	nk_layout_row_dynamic(nk_ctx, 128, 1);
	nk_color_pick(nk_ctx, (struct nk_colorf *)scene.bg_color, NK_RGB);

	const char *light_color_str = "Light Color:";
	nk_layout_row_dynamic(nk_ctx, 18, 1);
	nk_text(nk_ctx, light_color_str, strlen(light_color_str), NK_TEXT_LEFT);
	nk_layout_row_dynamic(nk_ctx, 128, 1);
	nk_color_pick(nk_ctx, (struct nk_colorf *)scene.light_color, NK_RGB);

	/* information */
	nk_layout_row_dynamic(nk_ctx, 16, 1);

	char num_models_cached_str[NK_STR_MAX];
	char num_objects_str[NK_STR_MAX];
	char num_textures_str[NK_STR_MAX];

	snprintf(num_models_cached_str, NK_STR_MAX, "Models Cached: %d",
		 scene.num_models_cached);
	snprintf(num_objects_str, NK_STR_MAX, "Objects: %d", scene.num_objects);
	snprintf(num_textures_str, NK_STR_MAX, "Textures: %d",
		 scene.num_textures);

	nk_text(nk_ctx, num_models_cached_str, strlen(num_models_cached_str),
		NK_TEXT_LEFT);
	nk_text(nk_ctx, num_objects_str, strlen(num_objects_str), NK_TEXT_LEFT);
	nk_text(nk_ctx, num_textures_str, strlen(num_textures_str),
		NK_TEXT_LEFT);

	/* exporting */
	if (nk_button_label(nk_ctx, "Export Scene")) {
		scene_export(&scene, "out.scn");
	}

	nk_end(nk_ctx);
}

static void nuklear_object_panel(void)
{
	if (!nk_begin(nk_ctx, "Object",
		      nk_rect(WINDOW_WIDTH - 230, 0, 230, WINDOW_HEIGHT),
		      NK_WINDOW_BORDER | NK_WINDOW_MINIMIZABLE |
			      NK_WINDOW_TITLE)) {
		nk_end(nk_ctx);
		return;
	}

	if (object_selected == -1) {
		nk_end(nk_ctx);
		return;
	}

	object_t *obj_sel = scene.objects + object_selected;

	if (!obj_sel->mdl) {
		nk_end(nk_ctx);
		return;
	}

	/* basic info */

	char num_verts_str[NK_STR_MAX];
	char num_indis_str[NK_STR_MAX];
	char tex_ind_str[NK_STR_MAX];

	snprintf(num_verts_str, NK_STR_MAX, "Num Verts: %d",
		 obj_sel->mdl->meshes->num_vertices);
	snprintf(num_indis_str, NK_STR_MAX, "Num Indis: %d",
		 obj_sel->mdl->meshes->num_indices);
	snprintf(tex_ind_str, NK_STR_MAX, "Texture Index: %d",
		 obj_sel->mdl->meshes->tex_ind);

	nk_layout_row_dynamic(nk_ctx, 16, 1);
	nk_text(nk_ctx, num_verts_str, strlen(num_verts_str), NK_TEXT_LEFT);
	nk_text(nk_ctx, num_indis_str, strlen(num_indis_str), NK_TEXT_LEFT);
	nk_text(nk_ctx, tex_ind_str, strlen(tex_ind_str), NK_TEXT_LEFT);

	/* position */

	nk_layout_row_dynamic(nk_ctx, 30, 1);
	nk_text(nk_ctx, "Position:", strlen("Position"), NK_TEXT_LEFT);
	for (int i = 0; i < 3; i++) {
		const char *comp_str[3] = { "X", "Y", "Z" };
		nk_property_float(nk_ctx, comp_str[i], OBJ_POS_COMP_MIN,
				  obj_sel->position + i, OBJ_POS_COMP_MAX,
				  OBJ_POS_COMP_STEP, OBJ_POS_COMP_STEP);
	}
	if (nk_button_label(nk_ctx, "Reset")) {
		glm_vec3_zero(obj_sel->position);
	}

	/* texture */

	/* FIXME: This only references the first meshes texture! */
	const int tex_ind = obj_sel->mdl->meshes->tex_ind;

	if (tex_ind != -1) {
		texture_t *tex_cur = scene.textures + tex_ind;

		char tex_and_fmt_str[NK_STR_MAX];

		switch (tex_cur->format) {
		default:
			fprintf(stderr, "Texture %d has invalid format!\n",
				tex_ind);
			exit(EXIT_FAILURE);
			break;

		case GL_RED:
			snprintf(tex_and_fmt_str, NK_STR_MAX,
				 "Texture (%s):", "B&W");
			break;

		case GL_RGB:
			snprintf(tex_and_fmt_str, NK_STR_MAX,
				 "Texture (%s):", "RGB");
			break;

		case GL_RGBA:
			snprintf(tex_and_fmt_str, NK_STR_MAX,
				 "Texture (%s):", "RGBA");
			break;
		}

		nk_layout_row_dynamic(nk_ctx, 16, 1);
		nk_text(nk_ctx, tex_and_fmt_str, strlen(tex_and_fmt_str),
			NK_TEXT_LEFT);
		nk_layout_row_static(nk_ctx, 128, 128, 1);
		nk_image(nk_ctx, tex_cur->nkimg);
	}

	nk_end(nk_ctx);
}

static void camera_get_move_vec(vec3 move)
{
	vec3 move_up, move_side;
	glm_vec3_scale(cam_up, input.mouse_y_diff, move_up);
	glm_vec3_copy(cam_side, move_side);
	glm_vec3_scale(move_side, input.mouse_x_diff, move_side);
	glm_vec3_negate(move_side);
	glm_vec3_add(move_up, move_side, move);
	glm_vec3_scale(move, MOUSE_TO_OBJECT_MOVE_SCALE, move);
}

static void handle_object_modes(void)
{
	object_t *obj_sel;
	static vec3 obj_sel_pos_last;

	switch (mode_current) {
	case MODE_OBJECT_SELECT:
		/* switching modes */
		if (input.g_press) {
			glm_vec3_copy(scene.objects[object_selected].position,
				      obj_sel_pos_last);
			mode_current = MODE_OBJECT_MOVE;
			break;
		}

		/* mode itself */
		object_selected += input.e_press - input.q_press;
		if ((uint32_t)object_selected >= scene.num_objects) {
			object_selected = 0;
		}
		if (object_selected < 0) {
			object_selected = scene.num_objects - 1;
		}
		break;
	case MODE_OBJECT_MOVE:
		obj_sel = scene.objects + object_selected;

		/* switching modes */
		if (input.g_press || input.mb_press[GLFW_MOUSE_BUTTON_LEFT]) {
			mode_current = MODE_OBJECT_SELECT;
			break;
		}
		if (input.mb_press[GLFW_MOUSE_BUTTON_RIGHT] ||
		    input.esc_press) {
			glm_vec3_copy(obj_sel_pos_last, obj_sel->position);
			mode_current = MODE_OBJECT_SELECT;
			break;
		}

		/* mode itself */
		vec3 move;
		camera_get_move_vec(move);
		glm_vec3_negate(move);
		glm_vec3_add(obj_sel->position, move, obj_sel->position);
		break;
	}
}

#define AXIS_NUM_VERTS 6
#define AXIS_POS_OFFSET 0
#define AXIS_COL_OFFSET 3
#define AXIS_VERT_POS_OFFSET_BYTES (AXIS_POS_OFFSET * sizeof(float))
#define AXIS_VERT_COL_OFFSET_BYTES (AXIS_COL_OFFSET * sizeof(float))
#define AXIS_VERT_NUM_COMPS 7
#define AXIS_VERT_STRIDE (AXIS_VERT_NUM_COMPS * sizeof(float))

static float axis_verts[AXIS_NUM_VERTS][AXIS_VERT_NUM_COMPS] = {
	{ 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 1.f }, /* red */
	{ 1.f, 0.f, 0.f, 1.f, 0.f, 0.f, 1.f },
	{ 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 1.f }, /* green */
	{ 0.f, 1.f, 0.f, 0.f, 1.f, 0.f, 1.f },
	{ 0.f, 0.f, 0.f, 0.f, 0.f, 1.f, 1.f }, /* blue */
	{ 0.f, 0.f, 1.f, 0.f, 0.f, 1.f, 1.f }
};
static uint32_t axis_vao, axis_vbo;

static void axis_init(void)
{
	glGenVertexArrays(1, &axis_vao);
	glBindVertexArray(axis_vao);

	glGenBuffers(1, &axis_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, axis_vbo);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, AXIS_VERT_STRIDE,
			      (void *)AXIS_VERT_POS_OFFSET_BYTES);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, AXIS_VERT_STRIDE,
			      (void *)AXIS_VERT_COL_OFFSET_BYTES);
	glBufferData(GL_ARRAY_BUFFER, AXIS_NUM_VERTS * AXIS_VERT_STRIDE,
		     axis_verts, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);
}

static void axis_render(void)
{
	glDisable(GL_DEPTH_TEST);
	glBindVertexArray(axis_vao);
	glUseProgram(axis_shader.program);
	glUniformMatrix4fv(axis_shader.proj_matrix_uni, 1, GL_FALSE,
			   (float *)proj_matrix);
	glUniformMatrix4fv(axis_shader.view_matrix_uni, 1, GL_FALSE,
			   (float *)view_matrix);

	glDrawArrays(GL_LINES, 0, AXIS_NUM_VERTS);
	glBindVertexArray(0);
}

static void axis_terminate(void)
{
	glDeleteBuffers(1, &axis_vbo);
	glDeleteVertexArrays(1, &axis_vao);
}

static void camera_update(void)
{
	cam_dist -= input_scroll_y;
	if (cam_dist < 2) {
		cam_dist = 2;
	}

	if (!input.mb_down[GLFW_MOUSE_BUTTON_MIDDLE]) {
		return;
	}

	if (input.shift_down) {
		vec3 move;
		camera_get_move_vec(move);
		glm_vec3_add(cam_foc, move, cam_foc);
		return;
	}

	cam_ang[0] += input.mouse_x_diff;
	cam_ang[1] += input.mouse_y_diff;
	if (cam_ang[1] > 89.f) {
		cam_ang[1] = 89.f;
	}
	if (cam_ang[1] < -89.f) {
		cam_ang[1] = -89.f;
	}
}

static void camera_to_matrix(void)
{
	float yaw_rad = glm_rad(cam_ang[0]);
	float pitch_rad = glm_rad(cam_ang[1]);
	vec3 pos = {
		cosf(yaw_rad) * cosf(pitch_rad) * cam_dist,
		sinf(pitch_rad) * cam_dist,
		sinf(yaw_rad) * cosf(pitch_rad) * cam_dist,
	};
	glm_vec3_add(pos, cam_foc, pos);
	glm_vec3_sub(cam_foc, pos, cam_forw);
	glm_vec3_normalize(cam_forw);
	glm_vec3_cross(cam_forw, GLM_YUP, cam_side);
	glm_vec3_cross(cam_side, cam_forw, cam_up);

	glm_lookat(pos, cam_foc, GLM_YUP, view_matrix);
}

int main(void)
{
	glfw_init();
	glew_init();
	nuklear_init();
	axis_init();

	model_path_search();
	// scene = scene_import("out.scn");
	scene = scene_create_default();

	/* mesh shader */
	mesh_shader = shader_create_from_file(
		"assets/shaders/mesh-vertex.glsl",
		"assets/shaders/mesh-fragment.glsl",
		SHADER_INCLUDE_UNIFORM_TEXTURE |
			SHADER_INCLUDE_UNIFORM_PROJECTION |
			SHADER_INCLUDE_UNIFORM_VIEW |
			SHADER_INCLUDE_UNIFORM_MODEL);
	mesh_shader_uni_is_selected = shader_uniform_get_and_verify(
		mesh_shader.program, "u_is_selected");
	mesh_shader_uni_light_col = shader_uniform_get_and_verify(
		mesh_shader.program, "u_light_color");
	mesh_shader_uni_light_dir = shader_uniform_get_and_verify(
		mesh_shader.program, "u_light_dir");

	/* axis shader */
	axis_shader =
		shader_create_from_file("assets/shaders/axis-vertex.glsl",
					"assets/shaders/axis-fragment.glsl",
					SHADER_INCLUDE_UNIFORM_PROJECTION |
						SHADER_INCLUDE_UNIFORM_VIEW);

	glm_perspective(FOVY, WINDOW_ASPECT, NEAR_PLANE, FAR_PLANE,
			proj_matrix);

	while (!glfwWindowShouldClose(glfw_window)) {
		/************
		 * UPDATING *
		 ************/
		input = input_poll(glfw_window, input);
		if (scene.num_objects) {
			handle_object_modes();
			/* duplicating object */
			if (input.shift_down && input.d_press &&
			    scene.num_objects && object_selected != -1) {
				object_duplicate(
					scene.objects + scene.num_objects++,
					scene.objects + object_selected);
				object_selected = scene.num_objects - 1;
				mode_current = MODE_OBJECT_MOVE;
			}
		}
		camera_update();
		hide_helper_geo ^= input.tilde_press;

		/***********
		 * NUKLEAR *
		 ***********/
		nk_glfw3_new_frame();
		nuklear_main_panel();
		nuklear_object_panel();

		/*************
		 * RENDERING *
		 *************/
		glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
		glClearColor(scene.bg_color[0], scene.bg_color[1],
			     scene.bg_color[2], scene.bg_color[3]);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		camera_to_matrix();
		if (!hide_helper_geo) {
			axis_render();
		}

		glDisable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
		glUseProgram(mesh_shader.program);
		glUniform4fv(mesh_shader_uni_light_col, 1,
			     (const float *)&scene.light_color);
		vec3 light_dir;
		glm_vec3_negate_to(cam_forw, light_dir);
		glUniform3fv(mesh_shader_uni_light_dir, 1, light_dir);
		for (unsigned int i = 0; i < scene.num_objects; i++) {
			const int is_selected =
				(i == (uint32_t)object_selected) &&
				!hide_helper_geo;
			glUniform1i(mesh_shader_uni_is_selected, is_selected);
			object_render(scene.objects + i, &mesh_shader,
				      (const float *)proj_matrix,
				      (const float *)view_matrix, is_selected,
				      scene.textures);
		}

		nk_glfw3_render(NK_ANTI_ALIASING_ON, NK_VERTEX_BUFFER_SIZE,
				NK_INDEX_BUFFER_SIZE);

		glfwSwapBuffers(glfw_window);
	}

	num_model_paths = 0;
	free(model_paths);
	model_paths = NULL;

	glm_mat4_zero(proj_matrix);
	shader_destroy(&axis_shader);
	mesh_shader_uni_light_dir = mesh_shader_uni_light_col =
		mesh_shader_uni_is_selected = -1;
	shader_destroy(&mesh_shader);
	scene_destroy(&scene);
	axis_terminate();
	glfw_terminate();
}
