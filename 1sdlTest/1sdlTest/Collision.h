#pragma once
#include <SDL_rect.h>

class ColliderComponent; // Forward declaration

class Collision {
public:
	// Kiểm tra va chạm giữa hai hình chữ nhật SDL_Rect
	static bool AABB(const SDL_Rect& rectA, const SDL_Rect& rectB);

	// Kiểm tra va chạm giữa hai ColliderComponent
	static bool AABB(const ColliderComponent& colA, const ColliderComponent& colB);

	static bool PointInRect(const SDL_Point& point, const SDL_Rect& rect);
};