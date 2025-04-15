#include "Vector2D.h"

Vector2D::Vector2D() : x(0.0f), y(0.0f) {}
Vector2D::Vector2D(float x, float y) : x(x), y(y) {}

void Vector2D::Zero() {
	this->x = 0.0f;
	this->y = 0.0f;
}
// Implement các toán tử khác nếu cần