#include "Engine/Core/ECS/Registry.hpp"
#include "Engine/Core/Scene/Transform.hpp"

#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

namespace {

int gFailures = 0;

void check(bool condition, const std::string &what) {
  if (!condition) {
    std::cerr << "FAIL: " << what << std::endl;
    ++gFailures;
  }
}

using Engine::Core::ECS::Entity;
using Engine::Core::ECS::Registry;
using Engine::Core::Scene::Transform;

struct Velocity {
  float x{0.0f};
  float y{0.0f};
};

struct Tag {
  int id{0};
};

void testEntityLifecycle() {
  Registry registry;

  Entity a = registry.create();
  Entity b = registry.create();

  check(registry.valid(a), "freshly created entity a is valid");
  check(registry.valid(b), "freshly created entity b is valid");
  check(a != b, "distinct entities have distinct handles");
  check(registry.aliveCount() == 2, "aliveCount reflects two live entities");

  registry.destroy(a);
  check(!registry.valid(a), "destroyed entity a is no longer valid");
  check(registry.valid(b), "entity b survives destruction of a");
  check(registry.aliveCount() == 1, "aliveCount drops to one after destroy");

  Entity c = registry.create();
  check(registry.valid(c), "recycled entity c is valid");
  check(c.index == a.index, "recycled slot reuses the freed index");
  check(c.version != a.version, "recycled slot bumps the version");
  check(!registry.valid(a), "stale handle a stays invalid after recycle");

  registry.destroy(a); // stale handle: must be a no-op
  check(registry.valid(c), "destroying a stale handle does not touch the slot");
}

void testComponentStorage() {
  Registry registry;
  Entity e = registry.create();

  check(!registry.has<Transform>(e), "no component before emplace");
  check(registry.tryGet<Transform>(e) == nullptr, "tryGet is null before emplace");

  Transform &t = registry.emplace<Transform>(e);
  t.translation = glm::vec3(1.0f, 2.0f, 3.0f);

  check(registry.has<Transform>(e), "has component after emplace");
  check(registry.get<Transform>(e).translation.y == 2.0f, "component value round-trips");

  registry.emplace<Transform>(e).translation = glm::vec3(9.0f, 9.0f, 9.0f);
  check(registry.get<Transform>(e).translation.x == 9.0f, "re-emplace overwrites in place");

  registry.remove<Transform>(e);
  check(!registry.has<Transform>(e), "component gone after remove");
  registry.remove<Transform>(e); // second remove is a no-op

  registry.emplace<Velocity>(e, Velocity{5.0f, -5.0f});
  check(registry.get<Velocity>(e).x == 5.0f, "second component type stored independently");
}

void testDestroyStripsComponents() {
  Registry registry;
  Entity e = registry.create();
  registry.emplace<Transform>(e);
  registry.emplace<Velocity>(e);

  registry.destroy(e);
  check(!registry.valid(e), "entity invalid after destroy");

  Entity reused = registry.create();
  check(reused.index == e.index, "slot recycled");
  check(!registry.has<Transform>(reused), "recycled entity has no leftover Transform");
  check(!registry.has<Velocity>(reused), "recycled entity has no leftover Velocity");
}

void testSwapPopKeepsOthersIntact() {
  Registry registry;
  std::vector<Entity> entities;
  for (int i = 0; i < 5; ++i) {
    Entity e = registry.create();
    registry.emplace<Tag>(e, Tag{i});
    entities.push_back(e);
  }

  // Remove a middle element; swap-and-pop moves the last Tag into its slot.
  registry.remove<Tag>(entities[2]);
  check(!registry.has<Tag>(entities[2]), "removed middle element is gone");

  for (int i : {0, 1, 3, 4}) {
    check(registry.has<Tag>(entities[i]), "sibling " + std::to_string(i) + " still present");
    check(registry.get<Tag>(entities[i]).id == i,
          "sibling " + std::to_string(i) + " keeps its value after swap-pop");
  }
}

void testEachSingleAndMulti() {
  Registry registry;

  Entity onlyT = registry.create();
  registry.emplace<Transform>(onlyT);

  Entity both = registry.create();
  registry.emplace<Transform>(both);
  registry.emplace<Velocity>(both, Velocity{2.0f, 3.0f});

  Entity onlyV = registry.create();
  registry.emplace<Velocity>(onlyV, Velocity{1.0f, 1.0f});

  int transformVisits = 0;
  registry.each<Transform>([&](Entity, Transform &) { ++transformVisits; });
  check(transformVisits == 2, "each<Transform> visits every Transform holder");

  int pairVisits = 0;
  Entity seen = Engine::Core::ECS::kNullEntity;
  registry.each<Transform, Velocity>([&](Entity e, Transform &t, Velocity &v) {
    ++pairVisits;
    seen = e;
    t.translation.x += v.x;
  });
  check(pairVisits == 1, "each<Transform, Velocity> visits only the intersection");
  check(seen == both, "the intersecting entity is the expected one");
  check(registry.get<Transform>(both).translation.x == 2.0f,
        "each yields mutable component references");

  // Removing the current entity's component mid-iteration must be safe.
  int drained = 0;
  registry.each<Velocity>([&](Entity e, Velocity &) {
    ++drained;
    registry.remove<Velocity>(e);
  });
  check(drained == 2, "each visits each element exactly once while draining");
  check(registry.aliveCount() == 3, "draining components leaves entities alive");
}

void testTransformMatrix() {
  Transform identity;
  glm::mat4 m = identity.matrix();
  check(m[3][0] == 0.0f && m[3][1] == 0.0f && m[3][2] == 0.0f,
        "identity transform has zero translation column");
  check(m[0][0] == 1.0f && m[1][1] == 1.0f && m[2][2] == 1.0f,
        "identity transform has unit scale diagonal");

  Transform moved;
  moved.translation = glm::vec3(4.0f, -2.0f, 7.0f);
  glm::mat4 tm = moved.matrix();
  check(tm[3][0] == 4.0f && tm[3][1] == -2.0f && tm[3][2] == 7.0f,
        "translation lands in the matrix translation column");

  Transform scaled;
  scaled.scale = glm::vec3(2.0f, 3.0f, 4.0f);
  glm::mat4 sm = scaled.matrix();
  check(sm[0][0] == 2.0f && sm[1][1] == 3.0f && sm[2][2] == 4.0f,
        "scale lands on the matrix diagonal");
}

} // namespace

int main() {
  testEntityLifecycle();
  testComponentStorage();
  testDestroyStripsComponents();
  testSwapPopKeepsOthersIntact();
  testEachSingleAndMulti();
  testTransformMatrix();

  if (gFailures != 0) {
    std::cerr << gFailures << " check(s) failed" << std::endl;
    return EXIT_FAILURE;
  }

  std::cout << "all ECS checks passed" << std::endl;
  return EXIT_SUCCESS;
}
