#pragma once
#include <vector>
#include <memory>
#include <algorithm>
#include <bitset>
#include <array>
#include <iostream> // for cerr

class Component;
class Entity;
class Manager;

using ComponentID = std::size_t;
using Group = std::size_t; // Thêm Group nếu cần sau

inline ComponentID getComponentID() {
	static ComponentID lastID = 0;
	return lastID++;
}

template <typename T> inline ComponentID getComponentTypeID() noexcept {
	static ComponentID typeID = getComponentID();
	return typeID;
}

constexpr std::size_t maxComponents = 32;
constexpr std::size_t maxGroups = 32; // Nếu dùng group

using ComponentBitSet = std::bitset<maxComponents>;
using GroupBitset = std::bitset<maxGroups>; // Nếu dùng group
using ComponentArray = std::array<Component*, maxComponents>;

class Component {
public:
	Entity* entity = nullptr; // Con trỏ ngược lại Entity sở hữu

	virtual void init() {}
	virtual void update(float dt) {} // Sẽ cần dt sau
	virtual void draw() {}
	virtual ~Component() {}
};

class Entity {
private:
	Manager& manager; // Tham chiếu đến Manager để thêm vào group
	bool active = true;
	std::vector<std::unique_ptr<Component>> components;

	ComponentArray componentArray{};
	ComponentBitSet componentBitSet{};
	// GroupBitset groupBitset{}; // Nếu dùng group

public:
	Entity(Manager& mManager) : manager(mManager) {} // Constructor nhận Manager

	void update(float dt) { // Sẽ cần dt
		if (!active) return;
		for (auto& c : components) c->update(dt);
	}

	void draw() {
		if (!active) return;
		for (auto& c : components) c->draw();
	}

	bool isActive() const { return active; }
	void destroy() { active = false; }

	template <typename T> bool hasComponent() const {
		return componentBitSet[getComponentTypeID<T>()];
	}

	template <typename T, typename... TArgs>
	T& addComponent(TArgs&&... mArgs) {
		if (hasComponent<T>()) {
			// Nếu đã có component loại này, không nên thêm nữa
			// Hoặc có thể ghi đè/trả về component cũ
			std::cerr << "Warning: Component type already exists on entity." << std::endl;
			return getComponent<T>();
		}

		T* c = new T(std::forward<TArgs>(mArgs)...);
		c->entity = this;
		std::unique_ptr<Component> uPtr{ c };
		components.emplace_back(std::move(uPtr));

		componentArray[getComponentTypeID<T>()] = c;
		componentBitSet[getComponentTypeID<T>()] = true;

		c->init(); // Gọi init sau khi đã thêm vào
		return *c;
	}

	template<typename T> T& getComponent() const {
		auto ptr = componentArray[getComponentTypeID<T>()];
		// Nên thêm kiểm tra ptr != nullptr ở đây trước khi dereference
		if (!ptr) {
			throw std::runtime_error("Entity does not have component"); // Hoặc xử lý lỗi khác
		}
		return *static_cast<T*>(ptr);
	}

	// Các hàm Group có thể thêm sau
};

class Manager {
private:
	std::vector<std::unique_ptr<Entity>> entities;
	// std::array<std::vector<Entity*>, maxGroups> groupedEntities; // Nếu dùng group

public:
	void update(float dt) { // Sẽ cần dt
		for (auto& e : entities) e->update(dt);
	}

	void draw() {
		for (auto& e : entities) e->draw();
	}

	void refresh() {
		// Xóa các entity không active
		entities.erase(
			std::remove_if(std::begin(entities), std::end(entities),
				[](const std::unique_ptr<Entity>& mEntity) {
					return !mEntity->isActive();
				}),
			std::end(entities));

		// Cần thêm logic xóa entity khỏi group nếu dùng group
	}

	Entity& addEntity() {
		Entity* e = new Entity(*this); // Truyền tham chiếu Manager vào Entity
		std::unique_ptr<Entity> uPtr{ e };
		entities.emplace_back(std::move(uPtr));
		return *e;
	}
	Entity& getEntity(std::size_t index) {
		if (index < entities.size()) {
			return *entities[index];
		}
		throw std::out_of_range("Index out of range");
	}

	const std::vector<std::unique_ptr<Entity>>& getEntities() const {
		return entities;
	}
	
	// Các hàm Group có thể thêm sau
};