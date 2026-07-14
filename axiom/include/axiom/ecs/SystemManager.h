#pragma once

#include "ISystem.h"

#include <algorithm>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

namespace axiom {

class Scene;

class SystemManager {
  public:
    template <typename T, typename... Args>
    std::shared_ptr<T> AddSystem(Args &&...args) {
        static_assert(std::is_base_of_v<ISystem, T>,
                      "System must derive from axiom::ISystem");
        auto system = std::make_shared<T>(std::forward<Args>(args)...);
        m_Systems.push_back(system);
        return system;
    }

    void RemoveSystem(const std::shared_ptr<ISystem> &system) {
        m_Systems.erase(std::remove(m_Systems.begin(), m_Systems.end(), system),
                        m_Systems.end());
    }

    void Clear() { m_Systems.clear(); }

    void PreUpdate(Scene &scene, double dt) {
        for (auto &system : m_Systems) {
            if (system)
                system->PreUpdate(scene, dt);
        }
    }

    void Update(Scene &scene, double dt) {
        for (auto &system : m_Systems) {
            if (system)
                system->Update(scene, dt);
        }
    }

    void FixedUpdate(Scene &scene, double dt) {
        for (auto &system : m_Systems) {
            if (system)
                system->FixedUpdate(scene, dt);
        }
    }

    void PostUpdate(Scene &scene, double dt) {
        for (auto &system : m_Systems) {
            if (system)
                system->PostUpdate(scene, dt);
        }
    }

    void Render(Scene &scene, double alpha) {
        for (auto &system : m_Systems) {
            if (system)
                system->Render(scene, alpha);
        }
    }

    void BeginRenderFrame() {
        for (auto &system : m_Systems) {
            if (system)
                system->BeginRenderFrame();
        }
    }

    void OnViewportResize(uint32_t width, uint32_t height) {
        for (auto &system : m_Systems) {
            if (system)
                system->OnViewportResize(width, height);
        }
    }

  private:
    std::vector<std::shared_ptr<ISystem>> m_Systems;
};

} // namespace axiom
