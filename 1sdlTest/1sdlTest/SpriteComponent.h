// ECS/SpriteComponent.h
#pragma once
#include "ECS.h"
#include "Animation.h"
#include <SDL.h>
#include <SDL_image.h>
#include <map>
#include <string> 
#include <iostream> 
class TransformComponent;

class SpriteComponent : public Component {
private:
	TransformComponent* transform = nullptr;
	SDL_Texture* texture = nullptr; // Vẫn giữ texture pointer nếu muốn load sau
	SDL_Rect srcRect = { 0, 0, 0, 0 };
	SDL_Rect destRect = { 0, 0, 0, 0 };

	// --- THÊM LẠI CÁC BIẾN ANIMATION ---
	bool animated = false;
	int frames = 0;
	int speed = 100; // ms per frame

public:
	int animIndex = 0; // <<< THÊM LẠI: Dòng animation hiện tại trong spritesheet
	std::map<const char*, Animation> animation; // <<< THÊM LẠI: Map lưu trữ animations
	SDL_RendererFlip spriteFlip = SDL_FLIP_NONE;

	SpriteComponent() = default;

	SpriteComponent(const char* texturePath, SDL_Renderer* rendererRef = nullptr)
	{
		// Constructor này cần được sửa lại hoặc không dùng nếu không load texture
		if (texturePath && texturePath[0] != '\0' && rendererRef) {
			if (!setTexture(texturePath, rendererRef)) {
				// Xử lý lỗi
			}
		}
	}


	SpriteComponent(const char* texturePath, bool isAnimated, SDL_Renderer* ren) // Nhận renderer để load texture
		: animated(isAnimated)
	{
		if (!setTexture(texturePath, ren)) { // Load texture trước
			// Xử lý lỗi nếu texture không load được
			std::cerr << "SpriteComponent Error: Failed to load texture in constructor." << std::endl;
			animated = false; // Tắt animation nếu không có texture
			return;
		}

		// Chỉ định nghĩa animation nếu được đánh dấu là animated
		if (animated) {
			// <<< ĐỊNH NGHĨA ANIMATIONS Ở ĐÂY >>>
			// Ví dụ: (Nhớ điều chỉnh index, frames, speed cho đúng spritesheet của bạn)
			Animation idle = Animation(0, 2, 200); // index 0, 2 frames, 200ms/frame
			Animation walk = Animation(1, 4, 150); // index 1, 4 frames, 150ms/frame
			Animation jump = Animation(2, 2, 150); // index 2, 2 frames, 150ms/frame (Tốc độ tùy chỉnh)
			Animation fall = Animation(3, 2, 150); // index 3, 2 frames, 150ms/frame (Tốc độ tùy chỉnh)

			// Thêm vào map
			animation.emplace("Idle", idle);
			animation.emplace("Walk", walk);
			animation.emplace("Jump", jump); // <<< THÊM VÀO MAP
			animation.emplace("Fall", fall); // <<< THÊM VÀO MAP
			// Bắt đầu với animation Idle
			Play("Idle");
			std::cout << "SpriteComponent: Initialized with animations. Starting with Idle." << std::endl;
		}
		else {
			std::cout << "SpriteComponent: Initialized as non-animated." << std::endl;
		}
	}

	~SpriteComponent() {
		if (texture) {
			SDL_DestroyTexture(texture);
		}
	}

	bool setTexture(const char* texturePath, SDL_Renderer* ren) {
		if (texture) SDL_DestroyTexture(texture);
		texture = nullptr; // Đặt lại texture cũ

		if (!ren) {
			std::cerr << "SpriteComponent Error: Renderer is NULL in setTexture!\n";
			return false;
		}
		if (!texturePath || texturePath[0] == '\0') {
			std::cerr << "SpriteComponent Error: Invalid texture path.\n";
			return false;
		}

		SDL_Surface* tmpSurface = IMG_Load(texturePath);
		if (!tmpSurface) {
			std::cerr << "IMG_Load Error loading [" << texturePath << "]: " << IMG_GetError() << std::endl;
			return false;
		}
		texture = SDL_CreateTextureFromSurface(ren, tmpSurface);
		SDL_FreeSurface(tmpSurface);
		if (!texture) {
			std::cerr << "SDL_CreateTextureFromSurface Error: " << SDL_GetError() << std::endl;
			return false;
		}
		std::cout << "SpriteComponent: Texture loaded [" << texturePath << "], frame size " << srcRect.w << "x" << srcRect.h << std::endl;

		// Cập nhật transform nếu nó chưa có size và component này được thêm trước transform
		if (entity && entity->hasComponent<TransformComponent>()) {
			auto* tempTransform = &entity->getComponent<TransformComponent>();
			if (tempTransform->width == 0 || tempTransform->height == 0) { // Chỉ cập nhật nếu transform chưa có size
				tempTransform->width = srcRect.w;
				tempTransform->height = srcRect.h;
				std::cout << "SpriteComponent: Updated Transform size from texture." << std::endl;
			}
		}

		return true;
	}

	void init() override {
		if (!entity->hasComponent<TransformComponent>()) {
			entity->addComponent<TransformComponent>(); // Thêm nếu chưa có
		}
		transform = &entity->getComponent<TransformComponent>();

		// <<< LẤY KÍCH THƯỚC FRAME (srcRect) TỪ TRANSFORM >>>
		if (transform) {
			srcRect.w = transform->width;
			srcRect.h = transform->height;
			// Đặt lại kích thước nếu không hợp lệ
			if (srcRect.w <= 0 || srcRect.h <= 0) {
				std::cerr << "SpriteComponent Warning: Invalid size from Transform in init(). Using 32x32.\n";
				srcRect.w = 32;
				srcRect.h = 32;
				transform->width = 32; // Đồng bộ lại transform
				transform->height = 32;
			}
		}
		else {
			// Trường hợp không có transform (không nên xảy ra)
			std::cerr << "SpriteComponent Error: Missing Transform in init()! Using 32x32.\n";
			srcRect.w = 32;
			srcRect.h = 32;
		}
		// <<< KẾT THÚC LẤY KÍCH THƯỚC >>>

		srcRect.x = 0;
		srcRect.y = 0; // Play() sẽ đặt lại nếu animated
		std::cout << "SpriteComponent Init: srcRect set to " << srcRect.w << "x" << srcRect.h << std::endl;
	}

	void UpdateAnimationState() {
		if (!transform || animation.empty()) return; // Cần transform để biết trạng thái

		bool currentGroundedState = transform->isGrounded;
		bool isMovingHorizontally = std::abs(transform->velocity.x) > 0.1f; // Ngưỡng nhỏ

		int currentAnimIndex = animIndex; // Lấy anim index hiện tại
		int idleIndex = animation.count("Idle") ? animation["Idle"].index : -1;
		int walkIndex = animation.count("Walk") ? animation["Walk"].index : -1;
		int jumpIndex = animation.count("Jump") ? animation["Jump"].index : -1;
		int fallIndex = animation.count("Fall") ? animation["Fall"].index : -1;

		if (currentGroundedState) {
			// TRÊN MẶT ĐẤT
			if (isMovingHorizontally) {
				if (walkIndex != -1 && currentAnimIndex != walkIndex) Play("Walk");
			}
			else {
				if (idleIndex != -1 && currentAnimIndex != idleIndex) Play("Idle");
			}
			// Sửa lỗi: Nếu đang chơi Jump/Fall mà đáp đất, phải chuyển ngay lập tức
			if (currentAnimIndex == jumpIndex || currentAnimIndex == fallIndex) {
				if (isMovingHorizontally && walkIndex != -1) Play("Walk");
				else if (idleIndex != -1) Play("Idle");
			}

		}
		else {
			// TRÊN KHÔNG
			if (transform->velocity.y < -0.1f) { // Bay lên
				if (jumpIndex != -1 && currentAnimIndex != jumpIndex) Play("Jump");
			}
			else if (transform->velocity.y > 0.1f) { // Rơi xuống
				if (fallIndex != -1 && currentAnimIndex != fallIndex) Play("Fall");
				// Fallback sang Jump nếu không có Fall anim
				else if (fallIndex == -1 && jumpIndex != -1 && currentAnimIndex != jumpIndex) Play("Jump");
			}
			// Giữ nguyên Jump/Fall nếu đang ở đỉnh nhảy (velocity.y gần 0)
		}
	}

	void update(float dt) override {
		if (!transform) return;

		int currentFrame = 0;
		if (animated && frames > 0 && speed > 0 && srcRect.w > 0) { // Thêm kiểm tra srcRect.w
			currentFrame = static_cast<int>((SDL_GetTicks() / speed) % frames);
			srcRect.x = srcRect.w * currentFrame;
		}
		else {
			srcRect.x = 0;
		}

		// Đảm bảo srcRect.h > 0 trước khi tính y
		if (srcRect.h > 0) {
			srcRect.y = animIndex * srcRect.h;
		}
		else {
			srcRect.y = 0; // Mặc định về hàng đầu nếu height = 0
		}


		destRect.x = static_cast<int>(transform->position.x);
		destRect.y = static_cast<int>(transform->position.y);
		destRect.w = transform->width * transform->scale;
		destRect.h = transform->height * transform->scale;
	}
	void draw() override {
		if (texture && transform && renderer && srcRect.w > 0 && srcRect.h > 0) { // Thêm kiểm tra renderer và srcRect size
			SDL_RenderCopyEx(renderer, texture, &srcRect, &destRect, 0.0, NULL, spriteFlip);
		}
	}

	void Play(const char* animName) {
		if (animation.count(animName)) {
			Animation newAnim = animation[animName];
			this->animIndex = newAnim.index;
			this->frames = newAnim.frames;
			this->speed = newAnim.speed;
			// Debug log khi Play
			std::cout << "Sprite Playing: " << animName << " (Index: " << animIndex << ", Frames: " << frames << ", Speed: " << speed << ")" << std::endl << std::flush;
		}
		else {
			// std::cerr << "Sprite Error: Animation '" << animName << "' not found!" << std::endl;
		}
	}
	// Biến tĩnh renderer (vẫn cần định nghĩa và gán giá trị trong main.cpp)
	static SDL_Renderer* renderer;
};