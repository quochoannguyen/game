#pragma once
#include <iostream>

class Vector2D {
public:
	float x;
	float y;

	Vector2D();
	Vector2D(float x, float y);

	// Chỉ cần các hàm cơ bản cho bước này
	void Zero();
	// Các toán tử khác có thể thêm sau nếu cần
};