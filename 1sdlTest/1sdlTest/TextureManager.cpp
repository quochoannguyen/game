// TextureManager.cpp
#include "TextureManager.h" // Include header tương ứng
#include <iostream>         // Cho std::cerr

// Hàm LoadTexture sử dụng renderer được truyền vào
SDL_Texture* TextureManager::LoadTexture(const char* fileName, SDL_Renderer* ren) {
	if (!ren) { // Kiểm tra renderer có hợp lệ không
		std::cerr << "TextureManager Error: Renderer passed to LoadTexture is NULL!" << std::endl;
		return nullptr;
	}
	SDL_Surface* tempSurface = IMG_Load(fileName);
	if (!tempSurface) {
		std::cerr << "IMG_Load Error loading [" << fileName << "]: " << IMG_GetError() << std::endl;
		return nullptr;
	}
	SDL_Texture* tex = SDL_CreateTextureFromSurface(ren, tempSurface);
	SDL_FreeSurface(tempSurface); // Luôn giải phóng surface sau khi tạo texture
	if (!tex) {
		std::cerr << "SDL_CreateTextureFromSurface Error: " << SDL_GetError() << std::endl;
		return nullptr;
	}
	return tex;
}

// Hàm Draw sử dụng renderer được truyền vào
void TextureManager::Draw(SDL_Texture* tex, SDL_Rect src, SDL_Rect dest, SDL_Renderer* ren, SDL_RendererFlip flip) {
	if (!ren) {
		// std::cerr << "TextureManager Error: Renderer passed to Draw is NULL!" << std::endl; // Có thể bỏ log này để tránh spam
		return;
	}
	if (!tex) {
		// std::cerr << "TextureManager Error: Texture passed to Draw is NULL!" << std::endl; // Có thể bỏ log này
		return;
	}
	// Vẽ texture bằng renderer được cung cấp
	SDL_RenderCopyEx(ren, tex, &src, &dest, 0.0, NULL, flip);
}