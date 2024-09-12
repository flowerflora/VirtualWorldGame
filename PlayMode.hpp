#include "Mode.hpp"

#include "Scene.hpp"

#include <glm/glm.hpp>

#include <vector>
#include <deque>

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
	} p1left, p1right, p1forward, p1back,p1fire,p2left, p2right, p2forward, p2back,p2fire;

	//local copy of the game scene (so code can change it during gameplay):
	Scene scene;

	//boats, bullets, and water:
	struct Bullet{
		Scene::Transform *star = nullptr;
		glm::vec3 v = glm::vec3(0.0f);	
		bool fired = false;
	};
	struct Boat {
		Scene::Transform *boat = nullptr;
		glm::quat base_rotation;
		uint32_t health = 10;
		std::vector<Bullet> bullets = {}; 
		Button fired,left,right,forward,backward;
		float whirlpoolTimer = 0;
	};
	Boat player1;
	Boat player2;
	
	Scene::Transform *water = nullptr;
	float waterRadius;

	float whirlpoolRadius = 10;
	float whirlpoolPull = .001;

	float wobble = 0.0f;
	std::vector<Boat*> players = {&player1,&player2};

	int winner = -1;
	
	//camera:
	Scene::Camera *camera = nullptr;

};
