#include "EnginePCH.h"
#include "AssetManager.h"
#include "Graphics.h"
#include "ModelLoader.h"
#include "TextureLoader.h"

AssetManager::AssetManager(Graphics* graphics)
    : m_graphics(graphics)
{
    if (!m_graphics)
    {
        throw std::runtime_error("AssetManager requires a valid Graphics pointer!");
    }
}

AssetManager::~AssetManager() = default;

std::shared_ptr<Mesh> AssetManager::LoadMesh(const std::string& filePath)
{
    // Check if mesh is already loaded
    auto it = m_meshes.find(filePath);
    if (it != m_meshes.end())
    {
        return it->second;
    }

    // Not found, load it
    auto mesh = ModelLoader::Load(m_graphics->GetDevice().Get(), filePath);
    if (mesh)
    {
        // Using make_shared to move the unique_ptr content into a shared_ptr
        m_meshes[filePath] = std::move(mesh);
        return m_meshes[filePath];
    }

    throw std::runtime_error("Failed to load mesh: " + filePath);
}

Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> AssetManager::LoadTexture(const std::wstring& filePath)
{
    // Check if texture is already loaded
    auto it = m_textures.find(filePath);
    if (it != m_textures.end())
    {
        return it->second;
    }

    // Not found, load it
    auto texture = TextureLoader::Load(m_graphics->GetDevice().Get(), m_graphics->GetContext().Get(), filePath);
    if (texture)
    {
        m_textures[filePath] = texture;
        return texture;
    }
    
    throw std::runtime_error("Failed to load texture.");
}

std::shared_ptr<Mesh> AssetManager::GetMesh(const std::string& filePath)
{
    auto it = m_meshes.find(filePath);
    if (it != m_meshes.end())
    {
        return it->second;
    }
    // In a real engine, you might want to return a default "missing" mesh
    // For now, we'll throw.
    throw std::runtime_error("Mesh not found: " + filePath);
}

Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> AssetManager::GetTexture(const std::wstring& filePath)
{
    auto it = m_textures.find(filePath);
    if (it != m_textures.end())
    {
        return it->second;
    }
    throw std::runtime_error("Texture not found.");
}
