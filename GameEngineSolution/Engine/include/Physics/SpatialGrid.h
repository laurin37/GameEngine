#pragma once

#include "ECS/Entity.h"
#include "Collision.h"
#include <unordered_map>
#include <vector>
#include <cstdint>
#include <DirectXMath.h>

// ==================================================================================
// SpatialGrid
// ----------------------------------------------------------------------------------
// Spatial hash grid for efficient broad-phase collision detection.
// Reduces collision checks from O(n²) to O(n·k) where k is average neighbors.
//
// Performance:
// - 1,000 entities: ~5,000 checks (vs 500,000 brute force)
// - 10,000 entities: ~50,000 checks (vs 50,000,000 brute force)
// ==================================================================================
class SpatialGrid {
public:
    explicit SpatialGrid(float cellSize = 10.0f)
        : m_cellSize(cellSize) {
    }
    
    // Clear all entities from the grid
    void Clear() {
        m_cells.clear();
    }
    
    // Insert an entity with its bounding box
    void Insert(ECS::Entity entity, const AABB& bounds) {
        // Calculate which cells this AABB overlaps
        DirectX::XMFLOAT3 min = {
            bounds.center.x - bounds.extents.x,
            bounds.center.y - bounds.extents.y,
            bounds.center.z - bounds.extents.z
        };
        DirectX::XMFLOAT3 max = {
            bounds.center.x + bounds.extents.x,
            bounds.center.y + bounds.extents.y,
            bounds.center.z + bounds.extents.z
        };
        
        int minCellX = static_cast<int>(std::floor(min.x / m_cellSize));
        int minCellY = static_cast<int>(std::floor(min.y / m_cellSize));
        int minCellZ = static_cast<int>(std::floor(min.z / m_cellSize));
        int maxCellX = static_cast<int>(std::floor(max.x / m_cellSize));
        int maxCellY = static_cast<int>(std::floor(max.y / m_cellSize));
        int maxCellZ = static_cast<int>(std::floor(max.z / m_cellSize));
        
        // Insert entity into all overlapping cells
        for (int x = minCellX; x <= maxCellX; ++x) {
            for (int y = minCellY; y <= maxCellY; ++y) {
                for (int z = minCellZ; z <= maxCellZ; ++z) {
                    int64_t cellHash = HashCell(x, y, z);
                    m_cells[cellHash].push_back(entity);
                }
            }
        }
    }
    
    // Query entities near a bounding box
    std::vector<ECS::Entity> Query(const AABB& bounds) const {
        // Calculate which cells to check
        DirectX::XMFLOAT3 min = {
            bounds.center.x - bounds.extents.x,
            bounds.center.y - bounds.extents.y,
            bounds.center.z - bounds.extents.z
        };
        DirectX::XMFLOAT3 max = {
            bounds.center.x + bounds.extents.x,
            bounds.center.y + bounds.extents.y,
            bounds.center.z + bounds.extents.z
        };
        
        int minCellX = static_cast<int>(std::floor(min.x / m_cellSize));
        int minCellY = static_cast<int>(std::floor(min.y / m_cellSize));
        int minCellZ = static_cast<int>(std::floor(min.z / m_cellSize));
        int maxCellX = static_cast<int>(std::floor(max.x / m_cellSize));
        int maxCellY = static_cast<int>(std::floor(max.y / m_cellSize));
        int maxCellZ = static_cast<int>(std::floor(max.z / m_cellSize));
        
        std::vector<ECS::Entity> result;
        std::unordered_map<uint32_t, bool> seen; // Deduplicate entities
        
        // Collect entities from all overlapping cells
        for (int x = minCellX; x <= maxCellX; ++x) {
            for (int y = minCellY; y <= maxCellY; ++y) {
                for (int z = minCellZ; z <= maxCellZ; ++z) {
                    int64_t cellHash = HashCell(x, y, z);
                    auto it = m_cells.find(cellHash);
                    if (it != m_cells.end()) {
                        for (ECS::Entity entity : it->second) {
                            if (seen.find(entity.id) == seen.end()) {
                                result.push_back(entity);
                                seen[entity.id] = true;
                            }
                        }
                    }
                }
            }
        }
        
        return result;
    }
    
    // Get cell size
    float GetCellSize() const { return m_cellSize; }
    
    // Set cell size (requires rebuilding grid)
    void SetCellSize(float cellSize) {
        m_cellSize = cellSize;
        Clear();
    }
    
    // Get statistics
    size_t GetCellCount() const { return m_cells.size(); }
    size_t GetTotalEntries() const {
        size_t total = 0;
        for (const auto& pair : m_cells) {
            total += pair.second.size();
        }
        return total;
    }

private:
    // Hash function for cell coordinates
    int64_t HashCell(int x, int y, int z) const {
        // Simple spatial hash (can be improved with better hash functions)
        int64_t h = x * 73856093LL;
        h ^= y * 19349663LL;
        h ^= z * 83492791LL;
        return h;
    }

    float m_cellSize;
    std::unordered_map<int64_t, std::vector<ECS::Entity>> m_cells;
};
