#pragma once

#include "ECS.h"
#include "Vector2D.h"
#include "TransformComponent.h"
#include "SpriteComponent.h"
#include "KeyboardController.h"
#include "ColliderComponent.h"


#include <functional> // <<< THÊM ĐỂ DÙNG std::function
#include <string>

class TransformComponent;
class SpriteComponent;
class KeyboardController;
class ColliderComponent;

enum class ButtonState {
	Normal,
	Hover, // Nếu muốn làm hiệu ứng hover
	Pressed
};

class ButtonComponent : public Component {
public:
	TransformComponent* transform = nullptr; // Để lấy vị trí, kích thước
	ButtonState currentState = ButtonState::Normal;
	std::function<void()> onClick; // Hành động khi click

	ButtonComponent(std::function<void()> action = nullptr) : onClick(action) {}

	void init() override {
		if (!entity->hasComponent<TransformComponent>()) {
			std::cerr << "ButtonComponent Error: Missing TransformComponent!\n";
			// Nên throw hoặc xử lý lỗi nghiêm trọng hơn
			entity->addComponent<TransformComponent>();
		}
		transform = &entity->getComponent<TransformComponent>();
	}

	// update có thể dùng để xử lý hover sau này
	void update(float dt) override {
		// Logic hover có thể thêm ở đây (kiểm tra vị trí chuột)
	}

	// draw có thể thay đổi sprite dựa trên currentState sau này
	void draw() override {
		// Ví dụ: nếu có SpriteComponent, thay đổi srcRect dựa trên currentState
	}

	void TriggerClick() {
		if (onClick) {
			onClick(); // Thực thi hành động đã gán
		}
		else {
			std::cout << "Button clicked, but no action assigned." << std::endl;
		}
	}
};
