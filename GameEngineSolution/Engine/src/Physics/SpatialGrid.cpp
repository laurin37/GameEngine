#include "../../include/Physics/SpatialGrid.h"
#include <cmath>
#include <algorithm>

namespace Physics {

SpatialGrid::SpatialGrid(float cellSize) : m_cellSize(cellSize) {}

void SpatialGrid::Insert(ECS::Entity entity, const AABB& worldAABB) {
    // Remove if already exists (to handle updates)
    Remove(entity);

    std::vector<GridCell> cells = GetCells(worldAABB);
    
    for (const auto& cell : cells) {
        m_grid[cell].push_back(entity);
    }

    m_entityCells[entity] = std::move(cells);
}

void SpatialGrid::Remove(ECS::Entity entity) {
    auto it = m_entityCells.find(entity);
    if (it == m_entityCells.end()) return;

    for (const auto& cell : it->second) {
        auto& entities = m_grid[cell];
        // Remove entity from cell (swap and pop)
        auto entIt = std::find(entities.begin(), entities.end(), entity);
        if (entIt != entities.end()) {
            *entIt = entities.back();
            entities.pop_back();
        }
        
        // Clean up empty cells to save memory
        if (entities.empty()) {
            m_grid.erase(cell);
        }
    }

    m_entityCells.erase(it);
}

void SpatialGrid::Clear() {
    m_grid.clear();
    m_entityCells.clear();
}

std::vector<ECS::Entity> SpatialGrid::Query(const AABB& worldAABB) const {
    std::unordered_set<ECS::Entity> uniqueEntities;
    std::vector<GridCell> cells = GetCells(worldAABB);

    for (const auto& cell : cells) {
        auto it = m_grid.find(cell);
        if (it != m_grid.end()) {
            for (ECS::Entity entity : it->second) {
                uniqueEntities.insert(entity);
            }
        }
    }

    return std::vector<ECS::Entity>(uniqueEntities.begin(), uniqueEntities.end());
}

std::vector<ECS::Entity> SpatialGrid::Raycast(
    const DirectX::XMFLOAT3& origin, 
    const DirectX::XMFLOAT3& direction, 
    float maxDistance
) const {
    // Calculate AABB of the ray (min/max bounds) and query that.
    // It's not perfect but much faster than O(N) and easier to implement correctly first.
    
    DirectX::XMFLOAT3 endPoint = {
        origin.x + direction.x * maxDistance,
        origin.y + direction.y * maxDistance,
        origin.z + direction.z * maxDistance
    };

    DirectX::XMFLOAT3 minPos = {
        std::min(origin.x, endPoint.x),
        std::min(origin.y, endPoint.y),
        std::min(origin.z, endPoint.z)
    };

    DirectX::XMFLOAT3 maxPos = {
        std::max(origin.x, endPoint.x),
        std::max(origin.y, endPoint.y),
        std::max(origin.z, endPoint.z)
    };

    AABB rayBounds;
    rayBounds.center = {
        (minPos.x + maxPos.x) * 0.5f,
        (minPos.y + maxPos.y) * 0.5f,
        (minPos.z + maxPos.z) * 0.5f
    };
    rayBounds.extents = {
        (maxPos.x - minPos.x) * 0.5f,
        (maxPos.y - minPos.y) * 0.5f,
        (maxPos.z - minPos.z) * 0.5f
    };

    return Query(rayBounds);
}

SpatialGrid::GridCell SpatialGrid::GetCell(const DirectX::XMFLOAT3& position) const {
    return {
        static_cast<int>(std::floor(position.x / m_cellSize)),
        static_cast<int>(std::floor(position.y / m_cellSize)),
        static_cast<int>(std::floor(position.z / m_cellSize))
    };
}

std::vector<SpatialGrid::GridCell> SpatialGrid::GetCells(const AABB& aabb) const {
    std::vector<GridCell> cells;
    
    DirectX::XMFLOAT3 minPos = {
        aabb.center.x - aabb.extents.x,
        aabb.center.y - aabb.extents.y,
        aabb.center.z - aabb.extents.z
    };
    
    DirectX::XMFLOAT3 maxPos = {
        aabb.center.x + aabb.extents.x,
        aabb.center.y + aabb.extents.y,
        aabb.center.z + aabb.extents.z
    };

    GridCell minCell = GetCell(minPos);
    GridCell maxCell = GetCell(maxPos);

    for (int x = minCell.x; x <= maxCell.x; ++x) {
        for (int y = minCell.y; y <= maxCell.y; ++y) {
            for (int z = minCell.z; z <= maxCell.z; ++z) {
                cells.push_back({x, y, z});
            }
        }
    }

    return cells;
}

} // namespace Physics
