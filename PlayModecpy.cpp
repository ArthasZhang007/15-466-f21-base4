#include "PlayMode.hpp"

#include "LitColorTextureProgram.hpp"

#include "DrawLines.hpp"
#include "Mesh.hpp"
#include "Load.hpp"
#include "gl_errors.hpp"
#include "data_path.hpp"

#include <glm/gtc/type_ptr.hpp>

#include <random>

GLuint VAO, VBO;
static Load<void> init_buffers(LoadTagDefault, []() {
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

Load<Sound::Sample> dusty_floor_sample(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("dusty-floor.opus"));
});

PlayMode::PlayMode() : scene(*hexapod_scene)
{
	//text_generator
	text_generator.load_font(data_path("times_new_roman.ttf"));
	text_generator.reshape("Graphics is the best class", glm::vec2(1.9,1.9), glm::vec3(0.7, 0.2, 0.3 ), 0);

	//get pointers to leg for convenience:
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

	//start music loop playing:
	// (note: position will be over-ridden in update())
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

	//slowly rotates through [0,1):
	wobble += elapsed / 10.0f;
	wobble -= std::floor(wobble);

	hip->rotation = hip_base_rotation * glm::angleAxis(
											glm::radians(5.0f * std::sin(wobble * 2.0f * float(M_PI))),
											glm::vec3(0.0f, 1.0f, 0.0f));
	upper_leg->rotation = upper_leg_base_rotation * glm::angleAxis(
														glm::radians(7.0f * std::sin(wobble * 2.0f * 2.0f * float(M_PI))),
														glm::vec3(0.0f, 0.0f, 1.0f));
	lower_leg->rotation = lower_leg_base_rotation * glm::angleAxis(
														glm::radians(10.0f * std::sin(wobble * 3.0f * 2.0f * float(M_PI))),
														glm::vec3(0.0f, 0.0f, 1.0f));

	//move sound to follow leg tip position:
	leg_tip_loop->set_position(get_leg_tip_position(), 1.0f / 60.0f);

	//move camera:
	{

		//combine inputs into a move:
		constexpr float PlayerSpeed = 30.0f;
		glm::vec2 move = glm::vec2(0.0f);
		if (left.pressed && !right.pressed)
			move.x = -1.0f;
		if (!left.pressed && right.pressed)
			move.x = 1.0f;
		if (down.pressed && !up.pressed)
			move.y = -1.0f;
		if (!down.pressed && up.pressed)
			move.y = 1.0f;

		//make it so that moving diagonally doesn't go faster:
		if (move != glm::vec2(0.0f))
			move = glm::normalize(move) * PlayerSpeed * elapsed;

		glm::mat4x3 frame = camera->transform->make_local_to_parent();
		glm::vec3 right = frame[0];
		//glm::vec3 up = frame[1];
		glm::vec3 forward = -frame[2];

		camera->transform->position += move.x * right + move.y * forward;
	}

	{ //update listener to camera position:
		glm::mat4x3 frame = camera->transform->make_local_to_parent();
		glm::vec3 right = frame[0];
		glm::vec3 at = frame[3];
		Sound::listener.set_position_right(at, right, 1.0f / 60.0f);
	}

	//reset button press counters:
	left.downs = 0;
	right.downs = 0;
	up.downs = 0;
	down.downs = 0;
}

void PlayMode::draw(glm::uvec2 const &drawable_size)
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST);
	// DRAW
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClearDepth(1.0f); //1.0 is actually the default value to clear the depth buffer to, but FYI you can change it.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Draw
	// start position
	float y_offset = -100.0f;
	float cursor_x = 0;
	float cursor_y = 0;
	double line = -1;

	glUseProgram(color_texture_program->program);


	glm::mat4 proj = glm::ortho(0.0f, (float)drawable_size.x, 0.0f, (float)drawable_size.y);
	glUniformMatrix4fv(
			color_texture_program->OBJECT_TO_CLIP_mat4,
			1,
			GL_FALSE,
			glm::value_ptr(proj)
	);
	glBindVertexArray(VAO);

	for (size_t i = 0; i < text_generator.characters.size(); ++i)
	{
		textgenerator::Character c = text_generator.characters[i];
		glUniform3f(glGetUniformLocation(color_texture_program->program, "textColor"), c.red, c.green, c.blue);
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
