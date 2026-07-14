#ifndef RENDER_PASS_MANAGER_HPP
#define RENDER_PASS_MANAGER_HPP

#include <unordered_map>
#include <vector>
#include <string>
#include <memory>
#include <vulkan/vulkan_raii.hpp>
#include <unordered_set>
#include <render_pass.hpp>

class RenderTarget;

// Render pass manager
class RenderPassManager
{
private:
  std::unordered_map<std::string, std::unique_ptr<RenderPass>> renderPasses;
  std::vector<RenderPass *> sortedPasses;
  bool dirty = true;

public:
  template <typename T, typename... Args>
  T *AddRenderPass(const std::string &name, Args &&...args)
  {
    static_assert(std::is_base_of<RenderPass, T>::value, "T must derive from RenderPass");

    auto it = renderPasses.find(name);
    if (it != renderPasses.end())
    {
      return dynamic_cast<T *>(it->second.get());
    }

    auto pass = std::make_unique<T>(std::forward<Args>(args)...);
    T *passPtr = pass.get();
    renderPasses[name] = std::move(pass);
    dirty = true;

    return passPtr;
  }

  RenderPass *GetRenderPass(const std::string &name)
  {
    auto it = renderPasses.find(name);
    if (it != renderPasses.end())
    {
      return it->second.get();
    }
    return nullptr;
  }

  void RemoveRenderPass(const std::string &name)
  {
    auto it = renderPasses.find(name);
    if (it != renderPasses.end())
    {
      renderPasses.erase(it);
      dirty = true;
    }
  }

  void Execute(vk::raii::CommandBuffer &commandBuffer)
  {
    if (dirty)
    {
      SortPasses();
      dirty = false;
    }

    for (auto pass : sortedPasses)
    {
      pass->Execute(commandBuffer);
    }
  }

private:
  void SortPasses()
  {
    // Topologically sort render passes based on dependencies
    sortedPasses.clear();

    // Create a copy of render passes for sorting
    std::unordered_map<std::string, RenderPass *> passMap;
    for (const auto &[name, pass] : renderPasses)
    {
      passMap[name] = pass.get();
    }

    // Perform topological sort
    std::unordered_set<std::string> visited;
    std::unordered_set<std::string> visiting;

    for (const auto &[name, pass] : passMap)
    {
      if (visited.find(name) == visited.end())
      {
        TopologicalSort(name, passMap, visited, visiting);
      }
    }
  }

  void TopologicalSort(const std::string &name,
                       const std::unordered_map<std::string, RenderPass *> &passMap,
                       std::unordered_set<std::string> &visited,
                       std::unordered_set<std::string> &visiting)
  {
    visiting.insert(name);

    auto pass = passMap.at(name);
    for (const auto &dep : pass->GetDependencies())
    {
      if (visited.find(dep) == visited.end())
      {
        if (visiting.find(dep) != visiting.end())
        {
          // Circular dependency detected
          throw std::runtime_error("Circular dependency detected in render passes");
        }
        TopologicalSort(dep, passMap, visited, visiting);
      }
    }

    visiting.erase(name);
    visited.insert(name);
    sortedPasses.push_back(pass);
  }
};

#endif // RENDER_PASS_MANAGER_HPP
