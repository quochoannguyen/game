// ECS/KeyboardController.h (Sửa lỗi cú pháp và logic animation)
#pragma once
#include "ECS.h"
#include "Components.h"
#include <SDL_keyboard.h>
#include <iostream>
#include "Collision.h"
class KeyboardController : public Component {
public:
	TransformComponent* transform = nullptr;
	SpriteComponent* sprite = nullptr;

	void init() override {
		if (entity->hasComponent<TransformComponent>()) {
			transform = &entity->getComponent<TransformComponent>();
		}
		else {
			std::cerr << "KC Error: Missing TransformComponent!\n";
		}
		if (entity->hasComponent<SpriteComponent>()) {
			sprite = &entity->getComponent<SpriteComponent>();
			if (sprite && !sprite->animation.empty() && sprite->animation.count("Idle")) {
				sprite->Play("Idle");
			}
		}
		else {
			std::cerr << "KC Warning: Missing SpriteComponent!\n";
		}
	}

	void update(float dt) override {
		if (!transform || !sprite || sprite->animation.empty()) return;
		const Uint8* keystate = SDL_GetKeyboardState(NULL);
		if (!keystate) return;

		bool currentGroundedState = transform->isGrounded;


		// --- Xác định vận tốc ngang ---
		float targetVelocityX = 0.0f;
		bool isTryingToMove = false;
		if (keystate[SDL_SCANCODE_A] || keystate[SDL_SCANCODE_LEFT]) {
			targetVelocityX = -transform->speed;
			isTryingToMove = true;
			if (sprite) sprite->spriteFlip = SDL_FLIP_HORIZONTAL;
		}
		else if (keystate[SDL_SCANCODE_D] || keystate[SDL_SCANCODE_RIGHT]) {
			targetVelocityX = transform->speed;
			isTryingToMove = true;
			if (sprite) sprite->spriteFlip = SDL_FLIP_NONE;
		}

		// --- Điều chỉnh trên không ---
		if (!transform->isGrounded) {
			targetVelocityX *= 0.8f; // Air control
		}

		// --- Gán vận tốc ---
		transform->velocity.x = targetVelocityX;

		const float nearZeroVelocityY = 0.1f;
		bool considerAsGrounded = currentGroundedState || (std::abs(transform->velocity.y) < nearZeroVelocityY);
	} // Kết thúc hàm update


	void TryJump() {
		if (transform && transform->isGrounded) {
			transform->velocity.y = -transform->jumpForce;
			transform->isGrounded = false;
			// Chơi animation "Jump" khi bắt đầu nhảy
			if (sprite && sprite->animation.count("Jump")) {
				sprite->Play("Jump");
			}
		}
	}
};