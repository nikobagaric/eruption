#pragma once

#include "Engine/Core/ECS/Entity.hpp"
#include "Engine/Core/ECS/SparseSet.hpp"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>
#include <utility>
#include <vector>

namespace Engine::Core::ECS {

class Registry {
public:
  Registry() = default;

  Registry(const Registry &) = delete;
  Registry &operator=(const Registry &) = delete;

  Registry(Registry &&) noexcept = default;
  Registry &operator=(Registry &&) noexcept = default;

  Entity create() {
    if (!mFreeIndices.empty()) {
      const uint32_t index = mFreeIndices.back();
      mFreeIndices.pop_back();
      mAlive[index] = true;
      return Entity{index, mVersions[index]};
    }

    const uint32_t index = static_cast<uint32_t>(mVersions.size());
    mVersions.push_back(0);
    mAlive.push_back(true);
    return Entity{index, 0};
  }

  bool valid(Entity entity) const {
    return entity.index < mVersions.size() && mAlive[entity.index] &&
           mVersions[entity.index] == entity.version;
  }

  void destroy(Entity entity) {
    if (!valid(entity)) {
      return;
    }

    for (auto &[type, pool] : mPools) {
      pool->remove(entity.index);
    }

    ++mVersions[entity.index];
    mAlive[entity.index] = false;
    mFreeIndices.push_back(entity.index);
  }

  size_t aliveCount() const { return mVersions.size() - mFreeIndices.size(); }

  template <typename T, typename... Args>
  T &emplace(Entity entity, Args &&...args) {
    return pool<T>().emplace(entity.index, std::forward<Args>(args)...);
  }

  template <typename T> void remove(Entity entity) {
    if (auto *set = findPool<T>()) {
      set->remove(entity.index);
    }
  }

  template <typename T> bool has(Entity entity) const {
    const auto *set = findPool<T>();
    return set != nullptr && set->contains(entity.index);
  }

  template <typename T> T &get(Entity entity) {
    return pool<T>().get(entity.index);
  }
  template <typename T> const T &get(Entity entity) const {
    return findPool<T>()->get(entity.index);
  }

  template <typename T> T *tryGet(Entity entity) {
    auto *set = findPool<T>();
    return set != nullptr ? set->tryGet(entity.index) : nullptr;
  }

  // Invoke fn(Entity, T&, Rest&...) for every alive entity that owns all of the
  // listed components. Iteration is driven by T's pool, walked back-to-front so
  // that fn may remove the current entity's T component in place. Do not add or
  // remove components of the iterated types for other entities during a pass.
  template <typename T, typename... Rest, typename Fn> void each(Fn &&fn) {
    auto *base = findPool<T>();
    if (base == nullptr) {
      return;
    }

    const std::vector<uint32_t> &dense = base->entities();
    for (size_t i = dense.size(); i-- > 0;) {
      const uint32_t index = dense[i];
      if (!(hasIndex<Rest>(index) && ...)) {
        continue;
      }
      fn(Entity{index, mVersions[index]}, base->get(index),
         pool<Rest>().get(index)...);
    }
  }

private:
  template <typename T> SparseSet<T> &pool() {
    const std::type_index key(typeid(T));
    auto it = mPools.find(key);
    if (it == mPools.end()) {
      it = mPools.emplace(key, std::make_unique<SparseSet<T>>()).first;
    }
    return static_cast<SparseSet<T> &>(*it->second);
  }

  template <typename T> SparseSet<T> *findPool() {
    const auto it = mPools.find(std::type_index(typeid(T)));
    return it == mPools.end() ? nullptr
                              : static_cast<SparseSet<T> *>(it->second.get());
  }
  template <typename T> const SparseSet<T> *findPool() const {
    const auto it = mPools.find(std::type_index(typeid(T)));
    return it == mPools.end()
               ? nullptr
               : static_cast<const SparseSet<T> *>(it->second.get());
  }

  template <typename T> bool hasIndex(uint32_t entityIndex) {
    const auto *set = findPool<T>();
    return set != nullptr && set->contains(entityIndex);
  }

  std::vector<uint32_t> mVersions;
  std::vector<bool> mAlive;
  std::vector<uint32_t> mFreeIndices;
  std::unordered_map<std::type_index, std::unique_ptr<ISparseSet>> mPools;
};

} // namespace Engine::Core::ECS
