// Game.cpp
#include "Game.hpp"
#include "ECS.h"
#include "Components.h"
#include "TextureManager.h" // Include nếu bạn tích hợp TextureManager sau này
#include "Collision.h"
#include <functional> // <<< THÊM CHO std::bind

SDL_Renderer* Game::renderer = nullptr;
SDL_Event Game::event; // Hàm khởi tạo mặc định là đủ
Manager Game::manager;
std::vector<ColliderComponent*> Game::colliders;

SDL_Renderer* SpriteComponent::renderer = nullptr;

Game::Game() {
	// Có thể khởi tạo các giá trị mặc định ở đây nếu cần
}

Game::~Game() {
	// clean(); // Cẩn thận nếu gọi clean() hai lần
}


void Game::init(const char* title, int xpos, int ypos, int width, int height, bool fullscreen) {
	int windowFlags = SDL_WINDOW_SHOWN; // Cờ mặc định cho cửa sổ
	if (fullscreen) {
		windowFlags = SDL_WINDOW_FULLSCREEN;
	}

	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		std::cerr << "SDL không thể khởi tạo! SDL_Error: " << SDL_GetError() << std::endl;
		return; // Thoát init nếu SDL lỗi
	}
	std::cout << "SDL Subsystems Initialized!" << std::endl;

	int imgFlags = IMG_INIT_PNG | IMG_INIT_JPG;
	if (!(IMG_Init(imgFlags) & imgFlags)) {
		std::cerr << "SDL_image không thể khởi tạo! IMG_Error: " << IMG_GetError() << std::endl;
		SDL_Quit();
		return; // Thoát init
	}
	std::cout << "SDL_image Initialized!" << std::endl;

	window = SDL_CreateWindow(title, xpos, ypos, width, height, windowFlags);
	if (!window) {
		std::cerr << "Không thể tạo cửa sổ! SDL_Error: " << SDL_GetError() << std::endl;
		IMG_Quit();
		SDL_Quit();
		return; // Thoát init
	}
	std::cout << "Cửa sổ đã được tạo." << std::endl;

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (!renderer) {
		std::cerr << "Không thể tạo renderer! SDL Error: " << SDL_GetError() << std::endl;
		SDL_DestroyWindow(window);
		IMG_Quit();
		SDL_Quit();
		return; // Thoát init
	}
	SDL_SetRenderDrawColor(renderer, 100, 149, 237, 255); // Đặt màu vẽ mặc định (màu nền trời)
	std::cout << "Renderer đã được tạo." << std::endl;

	// --- Gán Renderer cho các biến tĩnh cần thiết ---
	Game::renderer = renderer; // Gán cho thành viên tĩnh của lớp Game
	SpriteComponent::renderer = renderer; // Gán cho thành viên tĩnh của SpriteComponent (như trong main.cpp)

	// --- Tạo các Entity ban đầu ---
	// Lưu ý: Xử lý lỗi tải tài nguyên được đơn giản hóa ở đây
	std::cout << "Đang tạo Entities..." << std::endl;

	// --- TẠO MAIN MENU BAN ĐẦU ---
	CreateMainMenu();
	currentState = GameState::MainMenu; // Bắt đầu ở Main Menu
	isRunning = true; // Bắt đầu vòng lặp game
	lastFrameTime = SDL_GetTicks(); // Lấy thời điểm bắt đầu
	std::cout << "Game Initialized Successfully." << std::endl;

}

void Game::handleEvents() {
	// Lấy con trỏ tới entity Player một cách an toàn hơn thay vì giả định thứ tự
	// Cách tốt nhất là dùng tag hoặc group trong Manager
	Entity* playerPtr = nullptr;
	auto& entities = manager.getEntities(); // Lấy danh sách các entity đang hoạt động
	for (auto& ent : entities) {
		// Giả sử chỉ có Player mới có KeyboardController
		if (ent->hasComponent<KeyboardController>()) {
			playerPtr = ent.get(); // Lấy con trỏ raw từ unique_ptr
			break;
		}
	}

	KeyboardController* playerKeyboardController = nullptr;
	if (playerPtr && playerPtr->isActive()) { // Kiểm tra con trỏ và trạng thái entity
		playerKeyboardController = &playerPtr->getComponent<KeyboardController>();
	}

	// Vòng lặp xử lý sự kiện SDL
	while (SDL_PollEvent(&event) != 0) {
		if (event.type == SDL_QUIT) { // Nếu nhấn nút X để đóng cửa sổ
			isRunning = false;
			ExitGame();
		}

		if (event.type == SDL_MOUSEBUTTONDOWN) {
			if (event.button.button == SDL_BUTTON_LEFT) {
				HandleMouseClick(event.button.x, event.button.y);
			}
		}

		if (currentState == GameState::Playing || currentState == GameState::Paused) {
			Entity* playerPtr = nullptr; // Tìm player nếu cần
			for (auto& ent : manager.getEntities()) {
				if (ent->hasComponent<KeyboardController>()) {
					playerPtr = ent.get(); break;
				}
			}
			KeyboardController* playerKeyboardController = nullptr;
			if (playerPtr && playerPtr->isActive()) {
				playerKeyboardController = &playerPtr->getComponent<KeyboardController>();
			}

			if (event.type == SDL_KEYDOWN) {
				switch (event.key.keysym.sym) {
				case SDLK_ESCAPE: // Nhấn ESC
					if (currentState == GameState::Playing) {
						PauseGame();
					}
					else if (currentState == GameState::Paused) {
						ResumeGame();
					}
					break;
				case SDLK_SPACE: // Nhấn phím Space (chỉ khi đang chơi)
					if (currentState == GameState::Playing && playerKeyboardController) {
						playerKeyboardController->TryJump();
					}
					break;
					// --- THÊM NÚT PAUSE BẰNG BÀN PHÍM (VÍ DỤ: P) ---
				case SDLK_p:
					if (currentState == GameState::Playing) {
						PauseGame();
					}
					else if (currentState == GameState::Paused) {
						ResumeGame();
					}
					break;
				}
			}
			// Chỉ cho KeyboardController xử lý input A, D khi đang chơi
			if (currentState == GameState::Playing && playerKeyboardController) {
				// KeyboardController::update đã tự kiểm tra trạng thái bàn phím
				// Không cần gọi gì thêm ở đây nếu logic trong KC::update đã đúng
			}
		}
	}
}


void Game::update() {
	if (!isRunning) return;

	Uint32 currentFrameTime = SDL_GetTicks();
	// Delta time tính bằng giây (float)
	deltaTime = (currentFrameTime - lastFrameTime) / 1000.0f;
	lastFrameTime = currentFrameTime;

	if (deltaTime > 0.05f) {
		deltaTime = 0.05f;
	}
	switch (currentState) {
	case GameState::MainMenu:
		// Có thể thêm logic update cho menu (vd: hiệu ứng nút)
		manager.refresh(); // Vẫn cần refresh để xóa entity nếu cần
		manager.update(deltaTime); // Update các component của nút (vd: ButtonComponent::update)
		break;

	case GameState::Playing:
		// --- Cập nhật logic game chính ---
		manager.refresh();
		manager.update(deltaTime); // Cập nhật Transform, Sprite, Keyboard input...


		// --- Xử lý Va chạm ---
	   // Tìm lại player entity (cách làm an toàn)

		{


			Entity* playerPtr = nullptr;
			for (auto& ent : manager.getEntities()) {
				if (ent->hasComponent<KeyboardController>()) { playerPtr = ent.get(); break; }
			}

			if (playerPtr && playerPtr->isActive() && playerPtr->hasComponent<TransformComponent>() && playerPtr->hasComponent<ColliderComponent>()) {

				auto& playerTransform = playerPtr->getComponent<TransformComponent>();
				auto& pCollider = playerPtr->getComponent<ColliderComponent>();

				// *** Quan trọng: Reset trạng thái isGrounded trước mỗi lần kiểm tra va chạm ***
				playerTransform.isGrounded = false;

				for (ColliderComponent* otherCollider : colliders) {
					if (otherCollider == &pCollider) continue;
					if (!otherCollider->entity || !otherCollider->entity->isActive() || !otherCollider->entity->hasComponent<TransformComponent>()) continue;

					auto& otherTransform = otherCollider->entity->getComponent<TransformComponent>();

					if (Collision::AABB(pCollider, *otherCollider)) {
						SDL_Rect playerRect = pCollider.collider;
						SDL_Rect otherRect = otherCollider->collider;

						if (otherCollider->tag == "ground" || otherCollider->tag == "wall") {

							// --- Logic xử lý va chạm đơn giản bằng cách đẩy ra theo trục có độ chồng lấn ít nhất ---
						// Đây là một cách phổ biến, có thể cần tinh chỉnh tùy theo yêu cầu game.

						// Tính toán tâm của hai hình chữ nhật
							float playerCenterX = playerTransform.position.x + pCollider.offsetX + playerRect.w / 2.0f;
							float playerCenterY = playerTransform.position.y + pCollider.offsetY + playerRect.h / 2.0f;
							float otherCenterX = otherTransform.position.x + otherCollider->offsetX + otherRect.w / 2.0f;
							float otherCenterY = otherTransform.position.y + otherCollider->offsetY + otherRect.h / 2.0f;

							float diffX = playerCenterX - otherCenterX;
							float diffY = playerCenterY - otherCenterY;

							// Tính tổng nửa chiều rộng và nửa chiều cao
							float combinedHalfWidths = playerRect.w / 2.0f + otherRect.w / 2.0f;
							float combinedHalfHeights = playerRect.h / 2.0f + otherRect.h / 2.0f;

							// Tính độ chồng lấn trên mỗi trục
							float overlapX = combinedHalfWidths - std::abs(diffX);
							float overlapY = combinedHalfHeights - std::abs(diffY);

							if (overlapX > 0 && overlapY > 0) {

								// --- Ưu tiên giải quyết theo trục có độ chồng lấn ÍT HƠN ---
								// Điều này giúp tránh bị "kẹt" vào góc khi di chuyển theo đường chéo

								if (overlapY < overlapX) {
									// --- Giải quyết va chạm dọc ---
									// Nếu player đang rơi xuống hoặc đứng yên VÀ tâm player ở trên tâm vật cản
									if (diffY < 0 && playerTransform.velocity.y >= 0) {
										// Player đáp xuống vật cản (ground hoặc wall)
										// std::cout << "Landed on " << otherCollider->tag << std::endl; // DEBUG
										playerTransform.isGrounded = true;
										playerTransform.velocity.y = 0;
										// Đặt lại vị trí Y ngay trên đỉnh của vật cản (trừ đi chiều cao collider)
										playerTransform.setY(otherTransform.position.y + otherCollider->offsetY - playerRect.h);
									}
									// Nếu player đang bay lên VÀ tâm player ở dưới tâm vật cản
									else if (diffY > 0 && playerTransform.velocity.y < 0) {
										// Player đập đầu vào đáy vật cản
										// std::cout << "Hit ceiling on " << otherCollider->tag << std::endl; // DEBUG
										playerTransform.velocity.y = 0; // Dừng vận tốc lên
										// Đặt lại vị trí Y ngay dưới đáy vật cản
										playerTransform.setY(otherTransform.position.y + otherCollider->offsetY + otherRect.h);
									}
									// Cập nhật lại vị trí collider sau khi điều chỉnh transform
									pCollider.collider.y = static_cast<int>(playerTransform.position.y + pCollider.offsetY);
								}
								else {
									// --- Giải quyết va chạm ngang ---
									// Nếu tâm player ở bên trái tâm vật cản
									if (diffX < 0) {
										// Player va vào cạnh PHẢI của vật cản -> Đẩy Player sang TRÁI
										// std::cout << "Hit right side of " << otherCollider->tag << std::endl; // DEBUG
										playerTransform.velocity.x = 0; // Dừng vận tốc ngang
										// Đặt lại vị trí X ngay bên trái vật cản
										playerTransform.setX(otherTransform.position.x + otherCollider->offsetX - playerRect.w);
									}
									// Nếu tâm player ở bên phải tâm vật cản
									else {
										// Player va vào cạnh TRÁI của vật cản -> Đẩy Player sang PHẢI
										// std::cout << "Hit left side of " << otherCollider->tag << std::endl; // DEBUG
										playerTransform.velocity.x = 0;
										// Đặt lại vị trí X ngay bên phải vật cản
										playerTransform.setX(otherTransform.position.x + otherCollider->offsetX + otherRect.w);
									}
									// Cập nhật lại vị trí collider sau khi điều chỉnh transform
									pCollider.collider.x = static_cast<int>(playerTransform.position.x + pCollider.offsetX);
								}


							}

						}


					}
					float playerWidth = pCollider.collider.w;
					float playerLeftEdge = playerTransform.position.x + pCollider.offsetX; // Cạnh trái thực tế của collider
					float playerRightEdge = playerLeftEdge + playerWidth; // Cạnh phải thực tế

					if (playerLeftEdge < 0) { // Nếu vượt qua cạnh trái màn hình
						playerTransform.setX(0 - pCollider.offsetX); // Đặt lại vị trí X sao cho cạnh trái collider = 0
						if (playerTransform.velocity.x < 0) playerTransform.velocity.x = 0; // Dừng di chuyển nếu đang đi sang trái
						pCollider.collider.x = 0; // Cập nhật collider x
					}
					else if (playerRightEdge > SCREEN_WIDTH) { // Nếu vượt qua cạnh phải màn hình
						playerTransform.setX(SCREEN_WIDTH - playerWidth - pCollider.offsetX); // Đặt lại vị trí X sao cho cạnh phải collider = SCREEN_WIDTH
						if (playerTransform.velocity.x > 0) playerTransform.velocity.x = 0; // Dừng di chuyển nếu đang đi sang phải
						pCollider.collider.x = static_cast<int>(playerTransform.position.x + pCollider.offsetX); // Cập nhật collider x
					}
				}// Kết thúc khối if kiểm tra player hợp lệ

				if (playerPtr && playerPtr->isActive() && playerPtr->hasComponent<SpriteComponent>()) {
					playerPtr->getComponent<SpriteComponent>().UpdateAnimationState(); // Gọi hàm mới
				}

				for (auto& entity : manager.getEntities()) {
					if (entity->isActive() && entity->hasComponent<ColliderComponent>()) {
						entity->getComponent<ColliderComponent>().update(deltaTime); // Đảm bảo hàm này cập nhật collider.{x,y} dựa trên transform
					}
				}

				if (playerPtr->hasComponent<SpriteComponent>()) {
					playerPtr->getComponent<SpriteComponent>().UpdateAnimationState();
				}

			}



		}
		// --- Cập nhật vị trí các collider khác (nếu cần) ---
		for (auto& entity : manager.getEntities()) {
			if (entity->isActive() && entity->hasComponent<ColliderComponent>()) {
				entity->getComponent<ColliderComponent>().update(deltaTime);
			}
		}
		break; // Kết thúc case Playing

	case GameState::Paused:
		// Không cập nhật logic game, chỉ có thể cập nhật UI menu pause nếu cần
		manager.refresh();
		manager.update(deltaTime); // Update các component của nút pause
		break;

	case GameState::Exiting:
		isRunning = false; // Đặt cờ để thoát vòng lặp chính
		break;
	}
}



void Game::render() {
	if (!renderer) return; // Kiểm tra an toàn

	// Xóa màn hình với màu nền đã đặt (trong init hoặc set lại ở đây)
	SDL_SetRenderDrawColor(renderer, 100, 149, 237, 255); // Màu trời xanh
	SDL_RenderClear(renderer);

	switch (currentState) {
	case GameState::MainMenu:
		// Vẽ các entity của Main Menu (Background, Buttons)
		manager.draw(); // Giả sử các entity menu có SpriteComponent
		break;

	case GameState::Playing:
		// Vẽ màn chơi
		manager.draw(); // Vẽ player, ground, wall, tree, v.v.
		// Vẽ UI trong game (nút Pause)
		if (pauseButton && pauseButton->isActive()) {
			pauseButton->draw(); // Gọi hàm draw của nút (nếu có SpriteComponent)
		}
		break;

	case GameState::Paused:
		for (auto& entity : manager.getEntities()) {
			bool isPauseUI = (entity.get() == pauseMenuBackground ||
									 entity.get() == pauseContinueButton ||
									 entity.get() == pauseNewGameButton ||
									 entity.get() == pauseExitButton);

			if (!isPauseUI && entity->isActive()) { // Chỉ vẽ entity không phải UI Pause
				entity->draw();
			}
		}

		if (pauseMenuBackground && pauseMenuBackground->isActive()) pauseMenuBackground->draw();
		if (pauseContinueButton && pauseContinueButton->isActive()) pauseContinueButton->draw();
		if (pauseNewGameButton && pauseNewGameButton->isActive()) pauseNewGameButton->draw();
		if (pauseExitButton && pauseExitButton->isActive()) pauseExitButton->draw();
		break;
	}

	// --- Vẽ Debug Colliders (Tùy chọn) ---
	// Có thể thêm một biến bool để bật/tắt chế độ debug này
	bool drawDebugColliders = true; // Đặt thành false để tắt
	if (drawDebugColliders) {
		SDL_SetRenderDrawColor(renderer, 255, 0, 0, 150); // Màu đỏ, hơi trong suốt cho debug
		for (const auto& colliderComp : colliders) {
			// Chỉ vẽ nếu component và entity của nó hợp lệ và đang hoạt động
			if (colliderComp && colliderComp->entity && colliderComp->entity->isActive()) {
				// Lấy hình chữ nhật collider đã được cập nhật trong hàm update()
				SDL_Rect debugRect = colliderComp->collider;
				SDL_RenderDrawRect(renderer, &debugRect); // Vẽ viền hình chữ nhật
			}
		}
		// Lưu ý: Không cần vẽ ground/wall riêng biệt ở đây nữa nếu chúng có SpriteComponent
		// và được vẽ bởi manager.draw(). Nếu chúng không có Sprite và bạn muốn vẽ màu nền,
		// bạn cần lấy collider của chúng và vẽ như trên hoặc bằng SDL_RenderFillRect.
	}

	// --- Hiển thị những gì đã vẽ lên màn hình ---
	SDL_RenderPresent(renderer);
}


void Game::HandleMouseClick(int mouseX, int mouseY) {
	SDL_Point mousePoint = { mouseX, mouseY };

	// Xác định nút nào cần kiểm tra dựa trên trạng thái hiện tại
	std::vector<Entity*> currentButtons;
	if (currentState == GameState::MainMenu) {
		if (mainMenuNewGameButton) currentButtons.push_back(mainMenuNewGameButton);
		if (mainMenuExitButton) currentButtons.push_back(mainMenuExitButton);
	}
	else if (currentState == GameState::Paused) {
		if (pauseContinueButton) currentButtons.push_back(pauseContinueButton);
		if (pauseNewGameButton) currentButtons.push_back(pauseNewGameButton);
		if (pauseExitButton) currentButtons.push_back(pauseExitButton);
	}
	else if (currentState == GameState::Playing) {
		if (pauseButton) currentButtons.push_back(pauseButton); // Kiểm tra nút pause trong game
	}

	// Kiểm tra click trên các nút hợp lệ
	for (Entity* buttonEntity : currentButtons) {
		if (buttonEntity && buttonEntity->isActive() && buttonEntity->hasComponent<ButtonComponent>() && buttonEntity->hasComponent<TransformComponent>()) {
			auto& button = buttonEntity->getComponent<ButtonComponent>();
			auto& transform = buttonEntity->getComponent<TransformComponent>();
			SDL_Rect buttonRect = {
				static_cast<int>(transform.position.x),
				static_cast<int>(transform.position.y),
				transform.width * transform.scale,
				transform.height * transform.scale
			};

			if (Collision::PointInRect(mousePoint, buttonRect)) {
				button.TriggerClick(); // Gọi hành động của nút
				break; // Chỉ xử lý click trên một nút mỗi lần
			}
		}
	}
	// Nếu click vào nút Pause trong màn chơi
	if (currentState == GameState::Playing && pauseButton && pauseButton->isActive() && pauseButton->hasComponent<TransformComponent>()) {
		auto& transform = pauseButton->getComponent<TransformComponent>();
		SDL_Rect buttonRect = { /*...*/ };
		if (Collision::PointInRect(mousePoint, buttonRect)) {
			PauseGame(); // Gọi hàm PauseGame
			return; // Không cần kiểm tra nút khác nữa
		}
	}

}

void Game::CreateMainMenu() {
	std::cout << "Creating Main Menu..." << std::endl;
	// 1. Tạo Background (nếu có)
	auto& bg = manager.addEntity();
	// Giả sử background chiếm toàn màn hình
	bg.addComponent<TransformComponent>(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 1);
	// Thêm SpriteComponent cho background
	bg.addComponent<SpriteComponent>("asset/Menu_background.jpg", false, Game::renderer);
	mainMenuBackground = &bg; // Lưu tham chiếu (hoặc dùng tag)
	bg.getComponent<TransformComponent>().isGrounded = true;

	// 2. Tạo nút New Game
	auto& ngButton = manager.addEntity();
	// Cần xác định kích thước thực tế của ảnh nút
	int ngButtonW = 142; // Ví dụ
	int ngButtonH = 59; // Ví dụ
	// Đặt vị trí nút (ví dụ: căn giữa theo chiều ngang, đặt ở vị trí Y cụ thể)
	float ngButtonX = (SCREEN_WIDTH / 2.0f) - (ngButtonW / 2.0f);
	float ngButtonY = 250; // Ví dụ
	ngButton.addComponent<TransformComponent>(ngButtonX, ngButtonY, ngButtonW, ngButtonH, 1);
	ngButton.addComponent<SpriteComponent>("asset/NewGame_button.png", false, Game::renderer);
	// Gán hành động cho nút bằng lambda function hoặc std::bind
	ngButton.addComponent<ButtonComponent>([this]() { this->StartNewGame(); });
	// Hoặc: ngButton.addComponent<ButtonComponent>(std::bind(&Game::StartNewGame, this));
	mainMenuNewGameButton = &ngButton;
	ngButton.getComponent<TransformComponent>().isGrounded = true;

	// 3. Tạo nút Exit
	auto& exButton = manager.addEntity();
	int exButtonW = 142; // Ví dụ
	int exButtonH = 59; // Ví dụ
	float exButtonX = (SCREEN_WIDTH / 2.0f) - (exButtonW / 2.0f);
	float exButtonY = 320; // Ví dụ (đặt dưới nút New Game)
	exButton.addComponent<TransformComponent>(exButtonX, exButtonY, exButtonW, exButtonH, 1);
	exButton.addComponent<SpriteComponent>("asset/Exit_button.png", false, Game::renderer);
	exButton.addComponent<ButtonComponent>([this]() { this->ExitGame(); });
	exButton.getComponent<TransformComponent>().isGrounded = true;
	mainMenuExitButton = &exButton;
}

void Game::DestroyMainMenu() {
	std::cout << "Destroying Main Menu..." << std::endl;
	if (mainMenuBackground) mainMenuBackground->destroy();
	if (mainMenuNewGameButton) mainMenuNewGameButton->destroy();
	if (mainMenuExitButton) mainMenuExitButton->destroy();
	// Đặt lại con trỏ
	mainMenuBackground = nullptr;
	mainMenuNewGameButton = nullptr;
	mainMenuExitButton = nullptr;
	manager.refresh(); // Xóa các entity đã đánh dấu destroy
}

void Game::CreatePlayingUI() {
	std::cout << "Creating Playing UI (Pause Button)..." << std::endl;
	auto& pButton = manager.addEntity();
	int pButtonW = 142; // Ví dụ size nút Pause
	int pButtonH = 59;
	float pButtonX = SCREEN_WIDTH - pButtonW - 15; // Góc trên phải
	float pButtonY = 20;
	pButton.addComponent<TransformComponent>(pButtonX, pButtonY, pButtonW, pButtonH, 1);
	pButton.addComponent<SpriteComponent>("asset/pause_button.png", false, Game::renderer);
	pButton.getComponent<TransformComponent>().isGrounded = true;
	pButton.addComponent<ButtonComponent>([this]() { this->PauseGame(); });
	pauseButton = &pButton;
}

void Game::DestroyPlayingUI() {
	std::cout << "Destroying Playing UI..." << std::endl;
	if (pauseButton) pauseButton->destroy();
	pauseButton = nullptr;
	manager.refresh();
}

void Game::CreatePauseMenu() {
	std::cout << "Creating Pause Menu..." << std::endl;
	// Tương tự CreateMainMenu, tạo background và các nút Continue, New Game, Exit
	// Đặt vị trí các nút phù hợp cho menu pause

	// Ví dụ Nút Continue
	auto& contButton = manager.addEntity();
	int contButtonW = 142, contButtonH = 59;
	float contButtonX = (SCREEN_WIDTH / 2.0f) - (contButtonW / 2.0f);
	float contButtonY = 200; // Ví dụ vị trí
	contButton.addComponent<TransformComponent>(contButtonX, contButtonY, contButtonW, contButtonH, 1);
	contButton.addComponent<SpriteComponent>("asset/Resume_button.png", false, Game::renderer); // Dùng ảnh Resume
	contButton.addComponent<ButtonComponent>([this]() { this->ResumeGame(); });
	contButton.getComponent<TransformComponent>().isGrounded = true;
	pauseContinueButton = &contButton;

	// Ví dụ Nút New Game (trong Pause Menu)
	auto& pngButton = manager.addEntity();
	int pngButtonW = 142, pngButtonH = 59;
	float pngButtonX = (SCREEN_WIDTH / 2.0f) - (pngButtonW / 2.0f);
	float pngButtonY = 270;
	pngButton.addComponent<TransformComponent>(pngButtonX, pngButtonY, pngButtonW, pngButtonH, 1);
	pngButton.addComponent<SpriteComponent>("asset/NewGame_button.png", false, Game::renderer);
	pngButton.addComponent<ButtonComponent>([this]() { this->StartNewGame(); }); // Bắt đầu game mới từ Pause
	pngButton.getComponent<TransformComponent>().isGrounded = true;
	pauseNewGameButton = &pngButton;


	// Ví dụ Nút Exit (trong Pause Menu)
	auto& pexButton = manager.addEntity();
	int pexButtonW = 142, pexButtonH = 59;
	float pexButtonX = (SCREEN_WIDTH / 2.0f) - (pexButtonW / 2.0f);
	float pexButtonY = 340;
	pexButton.addComponent<TransformComponent>(pexButtonX, pexButtonY, pexButtonW, pexButtonH, 1);
	pexButton.addComponent<SpriteComponent>("asset/Exit_button.png", false, Game::renderer);
	pexButton.addComponent<ButtonComponent>([this]() { this->ExitGame(); });
	pexButton.getComponent<TransformComponent>().isGrounded = true;
	pauseExitButton = &pexButton;

	auto& pbg = manager.addEntity();
	pbg.addComponent<TransformComponent>(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 1);
	pbg.addComponent<SpriteComponent>("asset/Menu_background.jpg", false, Game::renderer);
	pauseMenuBackground = &pbg;
	pbg.getComponent<TransformComponent>().isGrounded = true;

}

void Game::DestroyPauseMenu() {
	std::cout << "Destroying Pause Menu..." << std::endl;
	if (pauseMenuBackground) pauseMenuBackground->destroy();
	if (pauseContinueButton) pauseContinueButton->destroy();
	if (pauseNewGameButton) pauseNewGameButton->destroy();
	if (pauseExitButton) pauseExitButton->destroy();
	pauseMenuBackground = nullptr;
	pauseContinueButton = nullptr;
	pauseNewGameButton = nullptr;
	pauseExitButton = nullptr;
	manager.refresh();
}

void Game::ResetGame() {
	std::cout << "Resetting Game..." << std::endl;
	// 1. Xóa tất cả entity hiện tại (trừ các entity hệ thống nếu có)
	auto& entities = manager.getEntities();
	for (int i = entities.size() - 1; i >= 0; --i) { // Duyệt ngược để xóa an toàn
		entities[i]->destroy();
	}
	manager.refresh(); // Thực hiện xóa
	colliders.clear(); // Xóa danh sách collider cũ

	// 2. Tạo lại các entity game ban đầu
	auto& player = manager.addEntity();
	player.addComponent<TransformComponent>(100.0f, 100.0f, 100, 113, 1);
	player.addComponent<SpriteComponent>("asset/player_idle1.png", true, renderer);
	player.addComponent<KeyboardController>();
	auto& playerCollider = player.addComponent<ColliderComponent>("player");
	colliders.push_back(&playerCollider);

	auto& ground = manager.addEntity();
	int groundY = 500, groundHeight = 50;
	ground.addComponent<TransformComponent>(0.0f, static_cast<float>(groundY), SCREEN_WIDTH, groundHeight, 1);
	auto& groundCollider = ground.addComponent<ColliderComponent>("ground");
	groundCollider.transform->isGrounded = true;
	colliders.push_back(&groundCollider);

	auto& wall = manager.addEntity();
	float wallX = 700.0f, wallY = 400.0f; int wallWidth = 30, wallHeight = 100;
	wall.addComponent<TransformComponent>(wallX, wallY, wallWidth, wallHeight, 1); // <<< Sửa lại thứ tự W/H
	wall.addComponent<ColliderComponent>("wall");
	wall.getComponent<TransformComponent>().isGrounded = true;
	colliders.push_back(&wall.getComponent<ColliderComponent>());

	auto& tree = manager.addEntity();
	float treeX = 400.0f, treeY = 400.0f; int treeWidth = 30, treeHeight = 100; // <<< Sửa lại kích thước tree
	tree.addComponent<TransformComponent>(treeX, treeY, treeWidth, treeHeight, 1); // <<< Sửa lại thứ tự W/H
	tree.addComponent<SpriteComponent>("asset/grass.png", false, Game::renderer);
	tree.addComponent<ColliderComponent>("wall"); // <<< Sửa lại Tag cho đúng
	tree.getComponent<TransformComponent>().isGrounded = true;
	colliders.push_back(&tree.getComponent<ColliderComponent>());

	// 3. Tạo lại UI trong game (nút Pause)
	CreatePlayingUI();
}


void Game::StartNewGame() {
	std::cout << "Starting New Game..." << std::endl;
	// Hủy menu hiện tại (MainMenu hoặc PauseMenu)
	if (currentState == GameState::MainMenu) {
		DestroyMainMenu();
	}
	else if (currentState == GameState::Paused) {
		DestroyPauseMenu();
	}
	// Reset lại trạng thái game
	ResetGame();
	// Chuyển sang trạng thái Playing
	currentState = GameState::Playing;
}

void Game::PauseGame() {
	if (currentState == GameState::Playing) {
		std::cout << "Game Paused." << std::endl;
		currentState = GameState::Paused;
		// Không hủy nút Pause trong game, chỉ tạo menu Pause
		CreatePauseMenu();
	}
}

void Game::ResumeGame() {
	if (currentState == GameState::Paused) {
		std::cout << "Resuming Game..." << std::endl;
		DestroyPauseMenu(); // Hủy menu Pause
		currentState = GameState::Playing; // Quay lại chơi
	}
}

void Game::ExitGame() {
	std::cout << "Exiting Game..." << std::endl;
	currentState = GameState::Exiting; // Đặt trạng thái thoát
}


void Game::clean() {
	std::cout << "Đang dọn dẹp trò chơi..." << std::endl;

	// Dọn dẹp entities và components (nếu Manager có hàm clear)
	// manager.clear(); // Giả sử Manager có hàm này

	// Hủy các đối tượng SDL
	if (renderer) {
		SDL_DestroyRenderer(renderer);
		renderer = nullptr; // Đặt thành null sau khi hủy để tránh sử dụng lại
		std::cout << "Renderer đã được hủy." << std::endl;
	}
	if (window) {
		SDL_DestroyWindow(window);
		window = nullptr; // Đặt thành null
		std::cout << "Cửa sổ đã được hủy." << std::endl;
	}

	// Thoát các hệ thống con của SDL
	IMG_Quit(); // Dọn dẹp SDL_image
	SDL_Quit(); // Dọn dẹp SDL
	std::cout << "Dọn dẹp trò chơi hoàn tất." << std::endl;
}