#include "Collision.h"
#include "ColliderComponent.h" // <<< Đường dẫn đúng

bool Collision::AABB(const SDL_Rect& rectA, const SDL_Rect& rectB) {
	if (rectA.x + rectA.w >= rectB.x &&    // Phải A >= Trái B
		rectB.x + rectB.w >= rectA.x &&    // Phải B >= Trái A
		rectA.y + rectA.h >= rectB.y &&    // Đáy A >= Đỉnh B
		rectB.y + rectB.h >= rectA.y) {   // Đáy B >= Đỉnh A
		return true;
	}
	return false;
}

bool Collision::AABB(const ColliderComponent& colA, const ColliderComponent& colB) {
	// Gọi lại hàm AABB với SDL_Rect bên trong component
	return AABB(colA.collider, colB.collider);
}

bool Collision::PointInRect(const SDL_Point& point, const SDL_Rect& rect) {
	return (point.x >= rect.x &&             // Điểm >= cạnh trái
			point.x < (rect.x + rect.w) &&   // Điểm < cạnh phải
			point.y >= rect.y &&             // Điểm >= cạnh trên
			point.y < (rect.y + rect.h));    // Điểm < cạnh dưới
}

