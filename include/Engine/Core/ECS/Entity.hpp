#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>

namespace Engine::Core::ECS {

struct Entity {
  static constexpr uint32_t kInvalidIndex = 0xFFFFFFFFu;

  uint32_t index{kInvalidIndex};
  uint32_t version{0};

  bool operator==(const Entity &other) const {
    return index == other.index && version == other.version;
  }
  bool operator!=(const Entity &other) const { return !(*this == other); }
};

inline constexpr Entity kNullEntity{};

} // namespace Engine::Core::ECS

namespace std {
template <> struct hash<Engine::Core::ECS::Entity> {
  size_t operator()(const Engine::Core::ECS::Entity &entity) const noexcept {
    return (static_cast<size_t>(entity.version) << 32) ^
           static_cast<size_t>(entity.index);
  }
};
} // namespace std
