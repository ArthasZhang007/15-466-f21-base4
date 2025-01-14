#pragma once

#include "Mode.hpp"

#include "Scene.hpp"
#include "Sound.hpp"

#include "GL.hpp"
#include <glm/glm.hpp>

#include <vector>
#include <deque>
#include "textgenerator.hpp"
// game logic in oops
#include "Story.hpp"
#include "oops.hpp"

struct PlayMode : Mode {
	PlayMode();
	virtual ~PlayMode();

	//functions called by main loop:
	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;

	//----- game state -----

	//input tracking:
	struct Button {
		uint8_t downs = 0;
		uint8_t pressed = 0;
	} left, right, down, up, space, enter;

	//local copy of the game scene (so code can change it during gameplay):
	Scene scene;

	//hexapod leg to wobble:
	Scene::Transform *hip = nullptr;
	Scene::Transform *upper_leg = nullptr;
	Scene::Transform *lower_leg = nullptr;
	glm::quat hip_base_rotation;
	glm::quat upper_leg_base_rotation;
	glm::quat lower_leg_base_rotation;
	float wobble = 0.0f;

	glm::vec3 get_leg_tip_position();

	//music coming from the tip of the leg (as a demonstration):
	std::shared_ptr< Sound::PlayingSample > leg_tip_loop;
	
	//camera:
	Scene::Camera *camera = nullptr;
	textgenerator text_generator;
	textgenerator menu_generator;
	Story game;
	State *currState;
	bool startup = true;

	bool status_init = true;
	bool status_change = true;
	int currentSelection = 0; // denote user's selection of choice
	atr_type m;
};
