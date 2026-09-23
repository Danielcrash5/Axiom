#pragma once
#include <any>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include "Scene.h"
#include <flecs.h>

namespace axiom {

    class Entity {
      public:
        Entity() = default;
        Entity(flecs::entity handle, Scene *scene)
            : m_Entity(handle), m_Scene(scene) {}

        flecs::entity_t GetID() const { return m_Entity.id(); }

        bool operator==(const Entity &other) const {
            return m_Entity == other.m_Entity && m_Scene == other.m_Scene;
        }

        std::string GetName() const {
            return m_Entity.get_mut<TagComponent>().Tag;
        }

        void SetName(const std::string &name) {
            m_Entity.get_mut<TagComponent>().Tag = name;
        }

        void Destroy() { m_Entity.destruct(); }

        // flecs::entity::emplace<T>(Args&&...) fuegt hinzu (entspricht
        // entt::registry::emplace) - Rueckgabe ist bei flecs void, daher
        // hinterher get_mut<T>() fuer die Referenz, um dieselbe Signatur wie
        // vorher (T&) zu behalten.
        template <typename T, typename... Args>
        T &AddComponent(Args &&...args) {
            m_Entity.emplace<T>(std::forward<Args>(args)...);
            return m_Entity.get_mut<T>();
        }

        // flecs::entity::set<T>(T&&) fuegt hinzu ODER ueberschreibt, falls
        // die Component schon existiert - deckt emplace_or_replace UND
        // replace gleichermassen ab (flecs unterscheidet das nicht wie entt).
        template <typename T, typename... Args>
        T &AddOrReplaceComponent(Args &&...args) {
            m_Entity.set<T>(T(std::forward<Args>(args)...));
            return m_Entity.get_mut<T>();
        }

        template <typename T, typename... Args>
        T &ReplaceComponent(Args &&...args) {
            m_Entity.set<T>(T(std::forward<Args>(args)...));
            return m_Entity.get_mut<T>();
        }

        template <typename T> void RemoveComponent() {
            m_Entity.remove<T>();
        }

        template <typename T> T &GetComponent() {
            return m_Entity.get_mut<T>();
        }

        struct ComponentInfo {
            flecs::id_t id;
            std::string name;

            std::string GetName() const { return name; }
        };

        // Laeuft ueber flecs' eingebaute Reflection (entity.each(flecs::id))
        // statt wie zuvor ueber entt::registry::storage() - flecs bringt
        // sowas nativ mit, kein Workaround noetig.
        std::vector<ComponentInfo> GetComponents() const {
            std::vector<ComponentInfo> result;
            m_Entity.each([&](flecs::id id) {
                result.push_back({id.raw_id(), id.str().c_str()});
            });
            return result;
        }

        // Echter void* statt des alten uintptr_t-Hacks (der frueher rohe
        // Adressen als Integer in std::any verpackt hat, was jeden Konsumenten
        // zwang, den Typ selbst wieder zurueckzucasten UND zu wissen, dass es
        // eigentlich ein Pointer war) - direkte, klar typisierte Verbesserung
        // im Zuge der Migration.
        std::any GetComponentById(flecs::id_t id) const {
            void *ptr = ecs_get_mut_id(m_Scene->m_World.c_ptr(), m_Entity.id(), id);
            if (!ptr) return std::any();
            return std::any(ptr);
        }

        std::any GetComponentByComponentInfo(const ComponentInfo &info) const {
            return GetComponentById(info.id);
        }

        template <typename T> bool HasComponent() const {
            return m_Entity.has<T>();
        }

        operator bool() const { return m_Entity.is_valid(); }

        operator flecs::entity() const { return m_Entity; }

      private:
        flecs::entity m_Entity{};
        Scene *m_Scene = nullptr;

        friend class Scene;
    };
} // namespace axiom
