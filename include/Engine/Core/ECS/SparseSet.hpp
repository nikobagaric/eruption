#pragma once

#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

namespace Engine::Core::ECS {

class ISparseSet {
public:
  virtual ~ISparseSet() = default;

  virtual bool contains(uint32_t entityIndex) const = 0;
  virtual bool remove(uint32_t entityIndex) = 0;
};

template <typename T> class SparseSet : public ISparseSet {
public:
  static constexpr uint32_t kTombstone = 0xFFFFFFFFu;

  bool contains(uint32_t entityIndex) const override {
    return entityIndex < mSparse.size() && mSparse[entityIndex] != kTombstone;
  }

  template <typename... Args> T &emplace(uint32_t entityIndex, Args &&...args) {
    if (contains(entityIndex)) {
      T &existing = mComponents[mSparse[entityIndex]];
      existing = T{std::forward<Args>(args)...};
      return existing;
    }

    if (entityIndex >= mSparse.size()) {
      mSparse.resize(entityIndex + 1, kTombstone);
    }

    mSparse[entityIndex] = static_cast<uint32_t>(mDense.size());
    mDense.push_back(entityIndex);
    return mComponents.emplace_back(std::forward<Args>(args)...);
  }

  T &get(uint32_t entityIndex) { return mComponents[mSparse[entityIndex]]; }
  const T &get(uint32_t entityIndex) const {
    return mComponents[mSparse[entityIndex]];
  }

  T *tryGet(uint32_t entityIndex) {
    return contains(entityIndex) ? &mComponents[mSparse[entityIndex]] : nullptr;
  }
  const T *tryGet(uint32_t entityIndex) const {
    return contains(entityIndex) ? &mComponents[mSparse[entityIndex]] : nullptr;
  }

  bool remove(uint32_t entityIndex) override {
    if (!contains(entityIndex)) {
      return false;
    }

    const uint32_t dense = mSparse[entityIndex];
    const uint32_t lastDense = static_cast<uint32_t>(mDense.size() - 1);

    if (dense != lastDense) {
      const uint32_t lastEntity = mDense[lastDense];
      mComponents[dense] = std::move(mComponents[lastDense]);
      mDense[dense] = lastEntity;
      mSparse[lastEntity] = dense;
    }

    mComponents.pop_back();
    mDense.pop_back();
    mSparse[entityIndex] = kTombstone;
    return true;
  }

  size_t size() const { return mDense.size(); }
  bool empty() const { return mDense.empty(); }

  const std::vector<uint32_t> &entities() const { return mDense; }
  std::vector<T> &components() { return mComponents; }
  const std::vector<T> &components() const { return mComponents; }

private:
  std::vector<uint32_t> mSparse;
  std::vector<uint32_t> mDense;
  std::vector<T> mComponents;
};

} // namespace Engine::Core::ECS
