#include "PlayMode.hpp"
#include "Story.hpp"
#include "LitColorTextureProgram.hpp"

#include "DrawLines.hpp"
#include "Mesh.hpp"
#include "Load.hpp"
#include "gl_errors.hpp"
#include "data_path.hpp"

#include <glm/gtc/type_ptr.hpp>

#include <random>

GLuint VAO, VBO;
static Load< void > setup_buffers(LoadTagDefault, []() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
});
GLuint hexapod_meshes_for_lit_color_texture_program = 0;
Load<MeshBuffer> hexapod_meshes(LoadTagDefault, []() -> MeshBuffer const * {
	MeshBuffer const *ret = new MeshBuffer(data_path("hexapod.pnct"));
	hexapod_meshes_for_lit_color_texture_program = ret->make_vao_for_program(lit_color_texture_program->program);
	return ret;
});

Load<Scene> hexapod_scene(LoadTagDefault, []() -> Scene const * {
	return new Scene(data_path("hexapod.scene"), [&](Scene &scene, Scene::Transform *transform, std::string const &mesh_name) {
		Mesh const &mesh = hexapod_meshes->lookup(mesh_name);

		scene.drawables.emplace_back(transform);
		Scene::Drawable &drawable = scene.drawables.back();

		drawable.pipeline = lit_color_texture_program_pipeline;

		drawable.pipeline.vao = hexapod_meshes_for_lit_color_texture_program;
		drawable.pipeline.type = mesh.type;
		drawable.pipeline.start = mesh.start;
		drawable.pipeline.count = mesh.count;
	});
});

Load<Sound::Sample> dusty_floor_sample(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("dusty-floor.opus"));
});

PlayMode::PlayMode() : scene(*hexapod_scene)
{
	//text_generator
	//inital state
	text_generator.font_size = 50;
	text_generator.load_font(data_path("ArialCE.ttf"));
	menu_generator.font_size = 70;
	menu_generator.load_font(data_path("SuperMario256.ttf"));
	std::string teststr = "Demon - Text Based Adventure Game";
	menu_generator.println(teststr, glm::vec2(-0.6,0.5),1, glm::vec3(128,128,0));
	std::string authors = "Presented by Lingxi Zhang and Zhengjia Cao";
	menu_generator.println(authors, glm::vec2(-0.7,0),2);
	std::string instruction = "Press SPACE to continue...";
	menu_generator.println(instruction, glm::vec2(-0.7,-0),3);
	game.state_init();
	currState = &game.middle_state_1F;
	for (auto &transform : scene.transforms)
	{
		if (transform.name == "Hip.FL")
			hip = &transform;
		else if (transform.name == "UpperLeg.FL")
			upper_leg = &transform;
		else if (transform.name == "LowerLeg.FL")
			lower_leg = &transform;
	}
	if (hip == nullptr)
		throw std::runtime_error("Hip not found.");
	if (upper_leg == nullptr)
		throw std::runtime_error("Upper leg not found.");
	if (lower_leg == nullptr)
		throw std::runtime_error("Lower leg not found.");

	hip_base_rotation = hip->rotation;
	upper_leg_base_rotation = upper_leg->rotation;
	lower_leg_base_rotation = lower_leg->rotation;

	//get pointer to camera for convenience:
	if (scene.cameras.size() != 1)
		throw std::runtime_error("Expecting scene to have exactly one camera, but it has " + std::to_string(scene.cameras.size()));
	camera = &scene.cameras.front();

	leg_tip_loop = Sound::loop_3D(*dusty_floor_sample, 1.0f, get_leg_tip_position(), 10.0f);
}

PlayMode::~PlayMode()
{
}

bool PlayMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size)
{

	if (evt.type == SDL_KEYDOWN)
	{
		if (evt.key.keysym.sym == SDLK_ESCAPE)
		{
			SDL_SetRelativeMouseMode(SDL_FALSE);
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_a)
		{
			left.downs += 1;
			left.pressed = true;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_d)
		{
			right.downs += 1;
			right.pressed = true;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_w)
		{
			up.downs += 1;
			up.pressed = true;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_s)
		{
			down.downs += 1;
			down.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_SPACE) {
			space.downs += 1;
			space.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_RETURN) {
			enter.downs += 1;
			enter.pressed = true;
			return true;
		}
	}
	else if (evt.type == SDL_KEYUP)
	{
		if (evt.key.keysym.sym == SDLK_a)
		{
			left.pressed = false;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_d)
		{
			right.pressed = false;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_w)
		{
			up.pressed = false;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_s)
		{
			down.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_SPACE) {
			space.pressed = false;
			this->startup = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_RETURN) {
			enter.pressed = false;
			return true;
		}
	}
	else if (evt.type == SDL_MOUSEBUTTONDOWN)
	{
		if (SDL_GetRelativeMouseMode() == SDL_FALSE)
		{
			SDL_SetRelativeMouseMode(SDL_TRUE);
			return true;
		}
	}
	else if (evt.type == SDL_MOUSEMOTION)
	{
		if (SDL_GetRelativeMouseMode() == SDL_TRUE)
		{
			glm::vec2 motion = glm::vec2(
				evt.motion.xrel / float(window_size.y),
				-evt.motion.yrel / float(window_size.y));
			camera->transform->rotation = glm::normalize(
				camera->transform->rotation * glm::angleAxis(-motion.x * camera->fovy, glm::vec3(0.0f, 1.0f, 0.0f)) * glm::angleAxis(motion.y * camera->fovy, glm::vec3(1.0f, 0.0f, 0.0f)));
			return true;
		}
	}
	return false;
}

void PlayMode::update(float elapsed)
{
	if (down.pressed) {
		currState->pushdown();
		std::cout << currState->current_choice << "\n";
	}
	if (up.pressed) {
		currState->pushup();
		std::cout << currState->current_choice << "\n";

	}
	if (enter.pressed) {
		State nextState = (currState->choose(m));
		if (nextState.description != "id") {
			currState = &nextState;
		}
	}
	if (!startup) {
		text_generator.println(currState->description, glm::vec2(-0.9,0.8));
		size_t line_num = 4;
		size_t count = 0;
		for (auto choice : currState->choices) {
			if (count++ == currState->current_choice) {
				text_generator.println(choice.description, glm::vec2(-0.9,0.3), line_num++, glm::vec3(128,128,0));
			}
			else text_generator.println(choice.description, glm::vec2(-0.9,0.3), line_num++);
		}
	}
	//reset button press counters:
	left.downs = 0;
	right.downs = 0;
	up.downs = 0;
	down.downs = 0;
	space.downs = 0;
}

void PlayMode::draw(glm::uvec2 const &drawable_size)
{
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST);
	// DRAW
	glClearColor(0.2f, 0.2f, 0.2f, 0.6f);
	glClear(GL_COLOR_BUFFER_BIT);

	// Draw
	// start position
	float y_offset = -100.0f;
	float cursor_x = 0;
	float cursor_y = 0;
	double line = -1;
	//printf("start\n");
	if (startup) {
		for (size_t i = 0; i < menu_generator.characters.size(); ++i) {
			textgenerator::Character c = menu_generator.characters[i];
			// printf(" a : %f %f %f %f\n", c.x_offset, c.y_offset, c.x_advance, c.y_advance);
			// printf(" b : % f %f %f %f %f\n", c.start_x, c.start_y, c.red, c.green, c.blue);
			glm::mat4 to_clip = glm::mat4( 
				1 * 2.0f / float(drawable_size.x), 0.0f, 0.0f, 0.0f,
				0.0f, 1 * 2.0f / float(drawable_size.y), 0.0f, 0.0f,
				0.0f, 0.0f, 1.0f, 0.0f,
				2.0f / float(drawable_size.x), 2.0f / float(drawable_size.y), 0.0f, 1.0f);
			glUseProgram(color_texture_program->program);
			glUniform3f(glGetUniformLocation(color_texture_program->program, "textColor"), c.red, c.green, c.blue);
			glUniformMatrix4fv(color_texture_program->OBJECT_TO_CLIP_mat4, 1, GL_FALSE, glm::value_ptr(to_clip));
			glBindVertexArray(VAO);

			if (c.line != line)
			{
				cursor_x = drawable_size.x / 2.0f * c.start_x;
				cursor_y = drawable_size.y / 2.0f * c.start_y + (float)c.line * y_offset;
				line = c.line;
			}

			float xpos = cursor_x + c.x_offset + c.bearing_x;
			float ypos = cursor_y + c.y_offset - (c.height - c.bearing_y);
			int w = c.width;
			int h = c.height;

			float vertices[6][4] = {
				{xpos, ypos + h, 0.0f, 0.0f},
				{xpos, ypos, 0.0f, 1.0f},
				{xpos + w, ypos, 1.0f, 1.0f},
				{xpos, ypos + h, 0.0f, 0.0f},
				{xpos + w, ypos, 1.0f, 1.0f},
				{xpos + w, ypos + h, 1.0f, 0.0f}};
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, c.texture);

			glBindBuffer(GL_ARRAY_BUFFER, VBO);
			glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
			glBindBuffer(GL_ARRAY_BUFFER, 0);

			glDrawArrays(GL_TRIANGLES, 0, 6);

			cursor_x += c.x_advance;
			cursor_y += c.y_advance;
		}
	} else {
		for (size_t i = 0; i < text_generator.characters.size(); ++i) {
			textgenerator::Character c = text_generator.characters[i];
			// printf(" a : %f %f %f %f\n", c.x_offset, c.y_offset, c.x_advance, c.y_advance);
			// printf(" b : % f %f %f %f %f\n", c.start_x, c.start_y, c.red, c.green, c.blue);
			glm::mat4 to_clip = glm::mat4( 
				1 * 2.0f / float(drawable_size.x), 0.0f, 0.0f, 0.0f,
				0.0f, 1 * 2.0f / float(drawable_size.y), 0.0f, 0.0f,
				0.0f, 0.0f, 1.0f, 0.0f,
				2.0f / float(drawable_size.x), 2.0f / float(drawable_size.y), 0.0f, 1.0f);
			glUseProgram(color_texture_program->program);
			glUniform3f(glGetUniformLocation(color_texture_program->program, "textColor"), c.red, c.green, c.blue);
			glUniformMatrix4fv(color_texture_program->OBJECT_TO_CLIP_mat4, 1, GL_FALSE, glm::value_ptr(to_clip));
			glBindVertexArray(VAO);

			if (c.line != line)
			{
				cursor_x = drawable_size.x / 2.0f * c.start_x;
				cursor_y = drawable_size.y / 2.0f * c.start_y + (float)c.line * y_offset;
				line = c.line;
			}

			float xpos = cursor_x + c.x_offset + c.bearing_x;
			float ypos = cursor_y + c.y_offset - (c.height - c.bearing_y);
			int w = c.width;
			int h = c.height;

			float vertices[6][4] = {
				{xpos, ypos + h, 0.0f, 0.0f},
				{xpos, ypos, 0.0f, 1.0f},
				{xpos + w, ypos, 1.0f, 1.0f},
				{xpos, ypos + h, 0.0f, 0.0f},
				{xpos + w, ypos, 1.0f, 1.0f},
				{xpos + w, ypos + h, 1.0f, 0.0f}};
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, c.texture);

			glBindBuffer(GL_ARRAY_BUFFER, VBO);
			glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
			glBindBuffer(GL_ARRAY_BUFFER, 0);

			glDrawArrays(GL_TRIANGLES, 0, 6);

			cursor_x += c.x_advance;
			cursor_y += c.y_advance;
		}
	}
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glUseProgram(0);

	GL_ERRORS();

}

glm::vec3 PlayMode::get_leg_tip_position()
{
	//the vertex position here was read from the model in blender:
	return lower_leg->make_local_to_world() * glm::vec4(-1.26137f, -11.861f, 0.0f, 1.0f);
}