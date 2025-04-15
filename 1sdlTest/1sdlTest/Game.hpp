// Game.hpp
#ifndef GAME_HPP
#define GAME_HPP

#include <SDL.h>
#include <SDL_image.h>
#include <vector>
#include <iostream> // Để xuất lỗi

// Khai báo trước (Forward Declarations) để tránh include các header đầy đủ nếu không cần
class Manager;
class ColliderComponent;
class Entity; // Khai báo trước Entity

class SpriteComponent;
class TransformComponent;

enum class GameState {
	MainMenu,
	Playing,
	Paused,
	Exiting // Trạng thái để thoát game an toàn
};

class Game {
public:
	Game();  // Hàm khởi tạo
	~Game(); // Hàm hủy

	
	void init(const char* title, int xpos, int ypos, int width, int height, bool fullscreen);

	// Xử lý sự kiện từ người dùng (input)
	void handleEvents();

	// Cập nhật trạng thái game (di chuyển, va chạm, logic game)
	void update();

	// Vẽ các đối tượng game lên màn hình
	void render();

	// Dọn dẹp tài nguyên và tắt SDL
	void clean();

	// Kiểm tra xem vòng lặp game có nên tiếp tục không
	bool running() const { return isRunning; }

	// --- Thành viên tĩnh (Static Members) ---
	// Dùng static để dễ truy cập từ các component/system ban đầu, tương tự cách dùng biến toàn cục.
	// Lưu ý: Đối với dự án lớn hơn, nên cân nhắc các giải pháp khác (dependency injection, service locator).
	static SDL_Renderer* renderer; // Renderer chính của game
	static SDL_Event event;        // Để xử lý sự kiện SDL
	static Manager manager;        // Quản lý entities và components (ECS)
	static std::vector<ColliderComponent*> colliders; // <<< DANH SÁCH COLLIDER TẠM THỜI (Giữ nguyên từ main.cpp)
	GameState getCurrentState() const { return currentState; }

private:
	GameState currentState = GameState::MainMenu;

	// <<< THÊM CON TRỎ ĐẾN CÁC NÚT BẤM >>>
	Entity* mainMenuBackground = nullptr;
	Entity* mainMenuNewGameButton = nullptr;
	Entity* mainMenuExitButton = nullptr;
	Entity* pauseButton = nullptr; // Nút pause trong màn chơi
	Entity* pauseMenuBackground = nullptr;
	Entity* pauseContinueButton = nullptr;
	Entity* pauseNewGameButton = nullptr;
	Entity* pauseExitButton = nullptr;

	// <<< THÊM KHAI BÁO HÀM HELPER CHO MENU >>>
	void CreateMainMenu();
	void DestroyMainMenu();
	void CreatePlayingUI(); // Tạo nút Pause khi vào game
	void DestroyPlayingUI();
	void CreatePauseMenu();
	void DestroyPauseMenu();
	void HandleMouseClick(int mouseX, int mouseY);

	// <<< THÊM HÀM CHUYỂN TRẠNG THÁI >>>
	void StartNewGame();
	void PauseGame();
	void ResumeGame();
	void ExitGame();
	void ResetGame(); // Hàm để khởi tạo lại màn chơi khi chọn New Game

	bool isRunning = false;     // Cờ kiểm soát vòng lặp game
	SDL_Window* window = nullptr; // Con trỏ tới cửa sổ SDL

	// --- Biến cho Delta Time ---
	Uint32 lastFrameTime = 0; // Thời điểm của frame trước (miligiây)
	float deltaTime = 0.0f;   // Thời gian giữa các frame (giây)

	// --- Các hằng số ---
	// Di chuyển SCREEN_WIDTH/HEIGHT vào đây để đóng gói tốt hơn
	static const int SCREEN_WIDTH = 1026;
	static const int SCREEN_HEIGHT = 578;

	// --- Hàm trợ giúp riêng (Private Helper Functions - Tùy chọn) ---
	// void loadAssets();     // Ví dụ: Có thể chuyển việc tải tài nguyên vào đây
	// void createEntities(); // Ví dụ: Có thể chuyển việc tạo entity vào đây
	// void checkCollisions();// Ví dụ: Có thể chuyển logic va chạm vào đây
	// void checkScreenBounds(); // Ví dụ: Chuyển kiểm tra biên màn hình vào đây
};

#endif // GAME_HPP