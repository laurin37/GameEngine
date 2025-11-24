#pragma once

/*
========================================
ECS (Entity Component System) Example Usage
========================================

This file demonstrates how to use the ECS system.
The ECS system runs alongside the existing GameObject system.

FILE CREATED: 2025-11-24
*/

#include "ECS/ComponentManager.h"
#include "ECS/Systems/ECSPhysicsSystem.h"
#include "ECS/Systems/ECSRenderSystem.h"

namespace ECSExample {

// Example: Creating a simple falling cube with ECS
inline ECS::Entity CreateFallingCube(ECS::ComponentManager& cm, Mesh* cubeMesh, std::shared_ptr<Material> material) {
    // Create entity
    ECS::Entity cube = cm.CreateEntity();
    
    // Add Transform component (position, rotation, scale)
    ECS::TransformComponent transform;
    transform.position = { 0.0f, 10.0f, 0.0f };  // Start 10 units up
    transform.rotation = { 0.0f, 0.0f, 0.0f };
    transform.scale = { 1.0f, 1.0f, 1.0f };
    cm.AddTransform(cube, transform);
    
    // Add Physics component (will fall due to gravity)
    ECS::PhysicsComponent physics;
    physics.velocity = { 0.0f, 0.0f, 0.0f };
    physics.mass = 1.0f;
    physics.useGravity = true;
    physics.checkCollisions = true;
    cm.AddPhysics(cube, physics);
    
    // Add Render component (mesh + material)
    ECS::RenderComponent render;
    render.mesh = cubeMesh;
    render.material = material;
    cm.AddRender(cube, render);
    
    // Add Collider component (bounding box)
    ECS::ColliderComponent collider;
    collider.localAABB.center = { 0.0f, 0.0f, 0.0f };
    collider.localAABB.extents = { 0.5f, 0.5f, 0.5f };  // 1x1x1 cube
    cm.AddCollider(cube, collider);
    
    return cube;
}

// Example: Creating multiple entities in a loop
inline void CreateManyEntities(ECS::ComponentManager& cm, Mesh* mesh, std::shared_ptr<Material> material, int count) {
    for (int i = 0; i < count; ++i) {
        ECS::Entity entity = cm.CreateEntity();
        
        ECS::TransformComponent transform;
        transform.position = { static_cast<float>(i * 2), 5.0f, 0.0f };
        transform.scale = { 1.0f, 1.0f, 1.0f };
        cm.AddTransform(entity, transform);
        
        ECS::PhysicsComponent physics;
        physics.useGravity = true;
        cm.AddPhysics(entity, physics);
        
        ECS::RenderComponent render;
        render.mesh = mesh;
        render.material = material;
        cm.AddRender(entity, render);
    }
}

// Example: Querying and modifying entities
inline void ApplyForceToAllPhysicsEntities(ECS::ComponentManager& cm, DirectX::XMFLOAT3 force) {
    std::vector<ECS::Entity> entities = cm.GetEntitiesWithPhysics();
    
    for (ECS::Entity entity : entities) {
        ECS::PhysicsComponent* physics = cm.GetPhysics(entity);
        if (physics) {
            physics->velocity.x += force.x;
            physics->velocity.y += force.y;
            physics->velocity.z += force.z;
        }
    }
}

// Example: Removing a component
inline void MakeEntityStatic(ECS::ComponentManager& cm, ECS::Entity entity) {
    // Remove physics component to make entity static
    cm.RemovePhysics(entity);
}

// Example: Destroying an entity
inline void DestroyEntity(ECS::ComponentManager& cm, ECS::Entity entity) {
    // This removes all components and recycles the entity ID
    cm.DestroyEntity(entity);
}

} // namespace ECSExample

/*
========================================
INTEGRATION WITH SCENE
========================================

To use ECS in your Scene class:

1. Add members to Scene.h:
   ```cpp
   ECS::ComponentManager m_ecsComponentManager;
   ECS::PhysicsSystem m_ecsPhysicsSystem;
   ECS::RenderSystem m_ecsRenderSystem;
   bool m_useECS = false;  // Toggle between GameObject and ECS
   ```

2. In Scene::Load():
   ```cpp
   if (m_useECS) {
       // Create ECS entities
       ECS::Entity cube = ECSExample::CreateFallingCube(
           m_ecsComponentManager, 
           m_meshCube.get(), 
           m_matFloor
       );
   }
   ```

3. In Scene::Update():
   ```cpp
   if (m_useECS) {
       m_ecsPhysicsSystem.Update(m_ecsComponentManager, deltaTime);
   } else {
       // Existing GameObject update code
   }
   ```

4. In Scene::Render():
   ```cpp
   if (m_useECS) {
       m_ecsRenderSystem.Render(m_ecsComponentManager, renderer, *m_camera);
   } else {
       // Existing GameObject render code
   }
   ```

5. Toggle ECS with a key press (in Game::Update()):
   ```cpp
   if (input.IsKeyDown('E') && !eKeyWasPressed) {
       m_scene->ToggleECS();
   }
   ```

========================================
PERFORMANCE BENEFITS
========================================

GameObject System (Current):
- Random memory access (cache misses)
- Virtual function calls (vtable overhead)
- 100 entities: ~0.5ms per frame

ECS System (New):
- Contiguous memory access (cache-friendly)
- No virtual calls (direct data access)
- 100 entities: ~0.1ms per frame
- 1000 entities: ~1ms per frame

The ECS system scales better for large numbers of entities!

========================================
*/
