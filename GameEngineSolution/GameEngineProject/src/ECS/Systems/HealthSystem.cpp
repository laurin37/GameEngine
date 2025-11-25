#include "../../../include/ECS/Systems/HealthSystem.h"
#include <iostream>
#include <format>

void HealthSystem::Update(float deltaTime) {
    auto healthArray = m_componentManager.GetComponentArray<ECS::HealthComponent>();
    
    for (size_t i = 0; i < healthArray->GetSize(); ++i) {
        ECS::Entity entity = healthArray->GetEntityAtIndex(i);
        ECS::HealthComponent& health = healthArray->GetData(entity);

        if (health.isDead) continue;

        // Regeneration
        if (health.regenerationRate > 0.0f && health.currentHealth < health.maxHealth) {
            health.currentHealth += health.regenerationRate * deltaTime;
            if (health.currentHealth > health.maxHealth) {
                health.currentHealth = health.maxHealth;
            }
        }

        // Death check
        if (health.currentHealth <= 0.0f) {
            health.currentHealth = 0.0f;
            health.isDead = true;
            
            // Log death (placeholder for event system)
            std::cout << std::format("Entity {} died!", entity) << std::endl;
            
            // Optional: Disable other components on death
            if (m_componentManager.HasComponent<ECS::ColliderComponent>(entity)) {
                m_componentManager.GetComponent<ECS::ColliderComponent>(entity).enabled = false;
            }
            if (m_componentManager.HasComponent<ECS::PhysicsComponent>(entity)) {
                m_componentManager.GetComponent<ECS::PhysicsComponent>(entity).checkCollisions = false;
                // Maybe stop movement?
                m_componentManager.GetComponent<ECS::PhysicsComponent>(entity).velocity = { 0.0f, 0.0f, 0.0f };
            }
            if (m_componentManager.HasComponent<ECS::RenderComponent>(entity)) {
                // Hide the entity by removing the mesh reference
                m_componentManager.GetComponent<ECS::RenderComponent>(entity).mesh = nullptr;
            }
        }
    }
}
