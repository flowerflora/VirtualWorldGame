#include "PlayMode.hpp"

#include "LitColorTextureProgram.hpp"

#include "DrawLines.hpp"
#include "Mesh.hpp"
#include "Load.hpp"
#include "gl_errors.hpp"
#include "data_path.hpp"

#include <glm/gtc/type_ptr.hpp>

#include <random>

GLuint boat_meshes_for_lit_color_texture_program = 0;
Load< MeshBuffer > boat_meshes(LoadTagDefault, []() -> MeshBuffer const * {
	MeshBuffer const *ret = new MeshBuffer(data_path("boat.pnct"));
	boat_meshes_for_lit_color_texture_program = ret->make_vao_for_program(lit_color_texture_program->program);
	return ret;
});

Load< Scene > boat_scene(LoadTagDefault, []() -> Scene const * {
	return new Scene(data_path("boat.scene"), [&](Scene &scene, Scene::Transform *transform, std::string const &mesh_name){
		Mesh const &mesh = boat_meshes->lookup(mesh_name);

		scene.drawables.emplace_back(transform);
		Scene::Drawable &drawable = scene.drawables.back();

		drawable.pipeline = lit_color_texture_program_pipeline;

		drawable.pipeline.vao = boat_meshes_for_lit_color_texture_program;
		drawable.pipeline.type = mesh.type;
		drawable.pipeline.start = mesh.start;
		drawable.pipeline.count = mesh.count;

	});
});

PlayMode::PlayMode() : scene(*boat_scene) {
	//get pointers to leg for convenience:
	for (auto &transform : scene.transforms) {
		if (transform.name == "boat1") player1.boat = &transform;
		else if (transform.name == "boat2") player2.boat = &transform;
		else if (transform.name == "water") water = &transform;
		else if (transform.name.find("star") != std::string::npos){
			Bullet star;
			star.star = &transform;
			if(transform.name.find("p1")!= std::string::npos) {
				player1.bullets.emplace_back(star);
			}
			else player2.bullets.emplace_back(star);
		}
	}
	if (player1.boat == nullptr) throw std::runtime_error("player1 boat not found.");
	if (player2.boat == nullptr) throw std::runtime_error("player2 boat not found.");
	if (water == nullptr) throw std::runtime_error("water not found.");

	player1.base_rotation = player1.boat->rotation;
	player2.base_rotation = player2.boat->rotation;
	water->position = glm::vec3(0.0f);
	
	// we'll base it off the scene, where I had the boats placed at the boundaries
	waterRadius = glm::length(player1.boat->position)+1.0f;
	whirlpoolRadius = glm::length(player2.boat->position)-1.0f;

	// now set boat positions to opposite ends
	player1.boat->position = glm::vec3(waterRadius/2,waterRadius/2,5);
	player2.boat->position = glm::vec3(-waterRadius/2,-waterRadius/2,5);

	//get pointer to camera for convenience:
	if (scene.cameras.size() != 1) throw std::runtime_error("Expecting scene to have exactly one camera, but it has " + std::to_string(scene.cameras.size()));
	camera = &scene.cameras.front();
	camera->transform->position = glm::vec3(0.0f,0.0f,150.0f);

	// setting the buttons
	player1.fired = p1fire;
	player1.left = p1left;
	player1.right = p1right;
	player1.forward = p1forward;
	player1.backward = p1back;
	player2.fired = p2fire;
	player2.left = p2left;
	player2.right = p2right;
	player2.forward = p2forward;
	player2.backward = p2back;
}

PlayMode::~PlayMode() {
}

bool PlayMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {

	if (evt.type == SDL_KEYDOWN) { 
		if (evt.key.keysym.sym == SDLK_LEFT) {
			player2.left.downs += 1;
			player2.left.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_RIGHT) {
			player2.right.downs += 1;
			player2.right.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_UP) {
			player2.forward.downs += 1;
			player2.forward.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_DOWN) {
			player2.backward.downs += 1;
			player2.backward.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_SLASH) { 
			player2.fired.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_a) {
			player1.left.downs += 1;
			player1.left.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_d) {
			player1.right.downs += 1;
			player1.right.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_w) {
			player1.forward.downs += 1;
			player1.forward.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_s) {
			player1.backward.downs += 1;
			player1.backward.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_f) { 
			player1.fired.pressed = true;
			return true;
		}
	} else if (evt.type == SDL_KEYUP) {
		if (evt.key.keysym.sym == SDLK_LEFT) {
			player2.left.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_RIGHT) { 
			player2.right.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_UP) { 
			player2.forward.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_DOWN) { 
			player2.backward.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_SLASH) { 
			player2.fired.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_a) {
			player1.left.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_d) {
			player1.right.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_w) {
			player1.forward.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_s) {
			player1.backward.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_f) {
			player1.fired.pressed = false;
			return true;
		} 
	}

	return false;
}

void PlayMode::update(float elapsed) {
	if (winner != -1){
		return; // game over
	}

	//slowly rotates through [0,1):
	// edit into pull towards the center
	wobble += elapsed / 8.0f;
	wobble -= std::floor(wobble);

	for (size_t i = 0; i< players.size();i++){
		Boat* b = players[i];
		Boat* op = players[(i+1)%2];

		// bobbing effect
		b->boat->rotation.x = (b->base_rotation * glm::angleAxis(
		glm::radians(7.0f * std::sin(wobble * 2.0f * 2.0f * float(M_PI))),
		glm::vec3(1.0f, 0.0f, 0.0f))).x;

		// checks if boats in whirl pool -- apply damage
		b->whirlpoolTimer +=elapsed; //want to apply once a second for now
		if (glm::length(b->boat->position)<=whirlpoolRadius && b->whirlpoolTimer>=1.5){
			b->health--;
			b->whirlpoolTimer = b->whirlpoolTimer - std::floor(b->whirlpoolTimer);
		}

		float gravity = 10.0f;
		// check collisions
		for(size_t j = 0; j<b->bullets.size();j++){
			if (b->bullets[j].fired){
			// see if hits other boat
			if (glm::length(op->boat->position - b->bullets[j].star->position)<3.0f){
				// update their health
				op->health--;
				// reset bullet
				b->bullets[j].fired = false;
				b->bullets[j].star->position= glm::vec3( -1.0f, -1,-10); //move out of sight + range
			}
			// spin cus why not
			b->bullets[j].star->rotation *= glm::angleAxis( glm::radians(elapsed*200), glm::vec3(0.0f, 0.0f, 1.0f) );
	
			// update trajectory
			b->bullets[j].v.z -= gravity * elapsed;
			b->bullets[j].star->position += b->bullets[j].v * elapsed;
			// reset bullet if out of field
			if (b->bullets[j].star->position.z<-5.0f){
				b->bullets[j].star->position = glm::vec3( -1.0f, -1,-10);
				b->bullets[j].fired = false;
			}
			}
			else if (b->fired.pressed){
				b->fired.pressed = false;
				b->bullets[j].fired = true; 
				b->bullets[j].star->position = b->boat->position; 
				float bulletangle = glm::eulerAngles(b->boat->rotation).z;
				b->bullets[j].v = glm::vec3(std::cos(bulletangle),std::sin(bulletangle),1)*10.0f;
			}
		}
		// check if health is below 0
		if (b->health <=0 ){ 
			//end the game, announce winner: op wins
			printf("Player %zu wins!\n",(i+1)%2+1);
			winner = (i+1)%2+1;
		}
		
		// moving the boat
		//combine inputs into a move:
		constexpr float PlayerSpeed = 5.0f;
		glm::vec2 move = glm::vec2(0.0f);
		if (b->left.pressed && !b->right.pressed) {
			b->boat->rotation *= glm::angleAxis( glm::radians(50.0f* elapsed), glm::vec3(0.0f, 0.0f, 1.0f));
		}
		if (!b->left.pressed && b->right.pressed) {
			b->boat->rotation *= glm::angleAxis( glm::radians(-50.0f*elapsed), glm::vec3(0.0f, 0.0f, 1.0f));
		}
		if (b->backward.pressed && !b->forward.pressed) {
			float angle = (glm::eulerAngles(b->boat->rotation)[2]);
			move -= glm::vec2(std::cos(angle),std::sin(angle));
		}
		if (!b->backward.pressed && b->forward.pressed) {
			float angle = (glm::eulerAngles(b->boat->rotation)[2]);
			move += glm::vec2(std::cos(angle),std::sin(angle));
		}
		//make it so that moving diagonally doesn't go faster:
		if (move != glm::vec2(0.0f)) move = glm::normalize(move) * PlayerSpeed * elapsed;
		
		glm::vec3 originalpos = b->boat->position ;
		b->boat->position += glm::vec3(move.x, move.y,0) ;
		
		// also need to bound them to the water edges
		if (glm::length(b->boat->position) > waterRadius){
			b->boat->position =  glm::normalize(b->boat->position) * waterRadius;
			b->boat->position.z = 5.0f;
		}

		// drag to center of whirlpool
		glm::vec3 pull = glm::normalize(b->boat->position)*( glm::length(b->boat->position) - whirlpoolPull * elapsed);
		b->boat->position = glm::vec3(pull.x,pull.y,5);
		whirlpoolPull +=elapsed/50;

		// avoid boat collisions, kinda janky but it kinda works
		if(glm::length(b->boat->position - op->boat->position)<6.0f){
			b->boat->position = originalpos;
		}

		//reset button press counters:
		b->left.downs = 0;
		b->right.downs = 0;
		b->forward.downs = 0;
		b->backward.downs = 0;
	}
	
	water->rotation *= glm::angleAxis( glm::radians(std::max(elapsed*whirlpoolPull*10,elapsed)), glm::vec3(0.0f, 0.0f, 1.0f) );
	
}

void PlayMode::draw(glm::uvec2 const &drawable_size) {
	//update camera aspect ratio for drawable:
	camera->aspect = float(drawable_size.x) / float(drawable_size.y);

	//set up light type and position for lit_color_texture_program:
	// TODO: consider using the Light(s) in the scene to do this
	glUseProgram(lit_color_texture_program->program);
	glUniform1i(lit_color_texture_program->LIGHT_TYPE_int, 1);
	glUniform3fv(lit_color_texture_program->LIGHT_DIRECTION_vec3, 1, glm::value_ptr(glm::vec3(0.0f, 0.0f,-1.0f)));
	glUniform3fv(lit_color_texture_program->LIGHT_ENERGY_vec3, 1, glm::value_ptr(glm::vec3(1.0f, 1.0f, 1.0f)));
	glUseProgram(0);

	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClearDepth(1.0f); //1.0 is actually the default value to clear the depth buffer to, but FYI you can change it.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS); //this is the default depth comparison function, but FYI you can change it.

	GL_ERRORS(); //print any errors produced by this setup code

	scene.draw(*camera);

	{ //use DrawLines to overlay some text:
		glDisable(GL_DEPTH_TEST);
		float aspect = float(drawable_size.x) / float(drawable_size.y);
		DrawLines lines(glm::mat4(
			1.0f / aspect, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		));

		constexpr float H = 0.09f;
		// lines.draw_text("Player 1 health: " + std::to_string(player1.health) +
		//  "					Player 2 health:"+ std::to_string(player2.health),
		// 	glm::vec3(-aspect + 0.1f * H, -1.0 + 0.1f * H, 0.0),
		// 	glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
		// 	glm::u8vec4(0x00, 0x00, 0x00, 0x00));
		// float ofs = 2.0f / drawable_size.y;
		lines.draw_text("Player 1 health: " + std::to_string(player1.health) + 
		"                                                                        "+
		"Player 2 health: "+ std::to_string(player2.health),
			glm::vec3(-aspect + 0.1f * H , -1.0 + 0.1f * H , 0.0),
			glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
			glm::u8vec4(0xff, 0xff, 0xff, 0x00));

		if(winner !=-1){
			lines.draw_text("Game over! Player " + std::to_string(winner) + " Wins",
				glm::vec3(-.6,-.3, 0.0),
				glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
				glm::u8vec4(0xff, 0xff, 0xff, 0x00));
		}
	}
}
