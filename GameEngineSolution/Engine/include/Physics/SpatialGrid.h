#pragma once

#include "../ECS/Entity.h"
#include "Collision.h" // For AABB
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <DirectXMath.h>
#include <functional>

namespace Physics {

class SpatialGrid {
public:
    SpatialGrid(float cellSize = 10.0f);

    void Insert(ECS::Entity entity, const AABB& worldAABB);
    void Remove(ECS::Entity entity);
    void Clear();

    // Returns entities in the same cells as the query AABB
    std::vector<ECS::Entity> Query(const AABB& worldAABB) const;

    // Raycast against entities in the grid
    // Returns entities that are in the cells intersected by the ray (Broadphase)
    std::vector<ECS::Entity> Raycast(
        const DirectX::XMFLOAT3& origin, 
        const DirectX::XMFLOAT3& direction, 
        float maxDistance
    ) const;

private:
    struct GridCell {
        int x, y, z;

        bool operator==(const GridCell& other) const {
            return x == other.x && y == other.y && z == other.z;
        }
    };

    struct GridCellHash {
        size_t operator()(const GridCell& cell) const {
            // Simple hash combining x, y, z
            // Using prime numbers to reduce collisions
            size_t h1 = std::hash<int>{}(cell.x);
            size_t h2 = std::hash<int>{}(cell.y);
            size_t h3 = std::hash<int>{}(cell.z);
            return h1 ^ (h2 << 1) ^ (h3 << 2); 
        }
    };

    GridCell GetCell(const DirectX::XMFLOAT3& position) const;
    std::vector<GridCell> GetCells(const AABB& aabb) const;

    float m_cellSize;
    std::unordered_map<GridCell, std::vector<ECS::Entity>, GridCellHash> m_grid;
    std::unordered_map<ECS::Entity, std::vector<GridCell>> m_entityCells; // Track which cells an entity is in for fast removal
};

} // namespace Physics
