#pragma once

struct Animation {
	int index = 0;  // Dòng trong spritesheet
	int frames = 1; // S? frame trong animation
	int speed = 100;// T?c ?? (ms per frame)

	Animation() = default; // Constructor m?c ??nh
	Animation(int i, int f, int s) : index(i), frames(f), speed(s) {}
};