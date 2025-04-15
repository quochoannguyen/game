#include "Game.hpp" // Chỉ cần include Game.hpp

// Khai báo trước hoặc include các file cần thiết khác nếu main cần (thường là không)

int main(int argc, char* args[]) {

	// Các hằng số cho cấu hình cửa sổ có thể đặt ở đây hoặc lấy từ Game.hpp
	const int SCREEN_WIDTH = 1026; // Có thể dùng Game::SCREEN_WIDTH nếu muốn
	const int SCREEN_HEIGHT = 578; // Có thể dùng Game::SCREEN_HEIGHT nếu muốn
	const char* WINDOW_TITLE = "Nền tảng ECS Cơ bản"; // Đặt tiêu đề cửa sổ

	Game* game = nullptr; // Sử dụng con trỏ cho đối tượng game chính (phổ biến)
	game = new Game();    // Tạo đối tượng Game trên heap

	if (game) { // Kiểm tra xem việc tạo đối tượng có thành công không
		// Khởi tạo game với các thông số cửa sổ
		game->init(WINDOW_TITLE,
				   SDL_WINDOWPOS_CENTERED, // Vị trí cửa sổ x (giữa màn hình)
				   SDL_WINDOWPOS_CENTERED, // Vị trí cửa sổ y (giữa màn hình)
				   SCREEN_WIDTH,           // Chiều rộng
				   SCREEN_HEIGHT,          // Chiều cao
				   false);                 // false = chế độ cửa sổ, true = toàn màn hình

		// --- Cài đặt giới hạn khung hình (Frame Limiting) ---
		const int FPS = 180; // Khung hình mỗi giây mong muốn
		const int FRAME_DELAY = 1000 / FPS; // Thời gian tối đa cho mỗi khung hình (ms)
		Uint32 frameStart;
		int frameTime;

		std::cout << "Bắt đầu vòng lặp Game..." << std::endl;

		// Vòng lặp game chính
		while (game->running()) {

			frameStart = SDL_GetTicks(); // Ghi lại thời điểm bắt đầu khung hình

			game->handleEvents(); // Xử lý input
			game->update();   // Cập nhật trạng thái game
			game->render();       // Vẽ game lên màn hình

			// Tính thời gian xử lý khung hình
			frameTime = SDL_GetTicks() - frameStart;

			// Nếu khung hình xử lý xong sớm hơn thời gian cho phép (FRAME_DELAY)
			if (FRAME_DELAY > frameTime) {
				// Chờ cho đủ thời gian để đạt FPS mong muốn
				SDL_Delay(FRAME_DELAY - frameTime);
			}
		} // Kết thúc vòng lặp game

		std::cout << "Kết thúc vòng lặp Game." << std::endl;

		// Dọn dẹp tài nguyên trước khi thoát
		game->clean();
		// Giải phóng bộ nhớ đã cấp phát cho đối tượng game
		delete game;
		game = nullptr; // Set con trỏ thành null sau khi xóa

	}
	else {
		std::cerr << "Lỗi: Không thể tạo đối tượng Game!" << std::endl;
		return 1; // Trả về mã lỗi
	}

	return 0; // Trả về 0 nếu mọi thứ thành công
}