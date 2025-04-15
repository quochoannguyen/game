#pragma once
#include "ECS.h"
#include "Components.h" // Cần TransformComponent
#include <SDL_rect.h>
#include <string>
#include <vector> // Cho Game::colliders sau này
#include <iostream>

// Forward declaration của Game nếu cần truy cập biến tĩnh (chưa cần ở bước này)
// class Game;

class ColliderComponent : public Component {
public:
	SDL_Rect collider; // Hình chữ nhật va chạm
	std::string tag;   // Nhãn để phân loại (ví dụ: "player", "ground", "wall")
	TransformComponent* transform = nullptr; // Con trỏ tới TransformComponent của entity

	int offsetX = 0; // Độ lệch của collider theo trục X so với transform->position.x
	int offsetY = 0; // Độ lệch của collider theo trục Y so với transform->position.y

	// Static list để lưu tất cả collider (sẽ cần class Game quản lý sau)
	// static std::vector<ColliderComponent*> colliders; // Tạm thời chưa dùng
	ColliderComponent(std::string t = "untagged", int offX = 0, int offY = 0)
		: tag(t), offsetX(offX), offsetY(offY)
	{
		collider = { 0, 0, 0, 0 }; // Khởi tạo rect cơ bản
	}

	void init() override {
		if (!entity->hasComponent<TransformComponent>()) {
			std::cerr << "ColliderComponent Warning: Entity missing TransformComponent. Adding default.\n";
			entity->addComponent<TransformComponent>();
		}
		transform = &entity->getComponent<TransformComponent>();

		// Khởi tạo kích thước collider ban đầu từ transform
		if (transform) {
			collider.w = transform->width * transform->scale;
			collider.h = transform->height * transform->scale;
			collider.x = static_cast<int>(transform->position.x) + offsetX;
			collider.y = static_cast<int>(transform->position.y) + offsetY;
		}

		// Thêm vào danh sách static (sẽ làm khi có class Game)
		// Game::colliders.push_back(this);
	}

	void update(float dt) override {
		// Cập nhật vị trí và kích thước collider theo transform
		if (transform) {
			collider.x = static_cast<int>(transform->position.x);
			collider.y = static_cast<int>(transform->position.y);
			collider.w = transform->width * transform->scale;
			collider.h = transform->height * transform->scale;
		}
	}
};