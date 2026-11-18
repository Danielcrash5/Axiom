/**
 * @file axiom/include/axiom/ecs/Entity.h
 * @brief Declares the public interface for the entity.
 */

#pragma once
#include <any>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include "Scene.h"
#include <flecs.h>

namespace axiom {

    /**
     * @brief Provides the entity functionality.
     * @tparam Entity Template type used by this type.
     */
    class Entity {
      public:
        /**
         * @brief Creates an instance of entity.
         */
        Entity() = default;
        /**
         * @brief Creates an instance of entity.
         * @param handle The handle used by this operation.
         * @param scene The scene used by this operation.
         */
        Entity(flecs::entity handle, Scene *scene)
            : m_entity(handle), m_scene(scene) {}

        /**
         * @brief Returns the ID.
         * @return The result of the operation.
         */
        flecs::entity_t getId() const { return m_entity.id(); }

        bool operator==(const Entity &other) const {
            return m_entity == other.m_entity && m_scene == other.m_scene;
        }

        /**
         * @brief Returns the name.
         * @return The result of the operation.
         */
        std::string getName() const {
            return m_entity.get_mut<TagComponent>().Tag;
        }

        /**
         * @brief Sets name.
         * @param name The name used by this operation.
         */
        void setName(const std::string &name) {
            m_entity.get_mut<TagComponent>().Tag = name;
        }

        /**
         * @brief Destroys  and releases its resources.
         */
        void destroy() { m_entity.destruct(); }

        // flecs::entity::emplace<T>(Args&&...) fuegt hinzu (entspricht
        // entt::registry::emplace) - Rueckgabe ist bei flecs void, daher
        // hinterher get_mut<T>() fuer die Referenz, um dieselbe Signatur wie
        // vorher (T&) zu behalten.
        /**
         * @brief Adds component.
         * @tparam T Template type used by this operation.
         * @param args The args used by this operation.
         * @return The result of the operation.
         */
        template <typename T, typename... Args>
        T &addComponent(Args &&...args) {
            m_entity.emplace<T>(std::forward<Args>(args)...);
            return m_entity.get_mut<T>();
        }

        // flecs::entity::set<T>(T&&) fuegt hinzu ODER ueberschreibt, falls
        // die Component schon existiert - deckt emplace_or_replace UND
        // replace gleichermassen ab (flecs unterscheidet das nicht wie entt).
        /**
         * @brief Adds or replace component.
         * @tparam T Template type used by this operation.
         * @param args The args used by this operation.
         * @return The result of the operation.
         */
        template <typename T, typename... Args>
        T &addOrReplaceComponent(Args &&...args) {
            m_entity.set<T>(T(std::forward<Args>(args)...));
            return m_entity.get_mut<T>();
        }

        /**
         * @brief Replaces the component attached to an entity.
         * @tparam T Template type used by this operation.
         * @param args The args used by this operation.
         * @return The result of the operation.
         */
        template <typename T, typename... Args>
        T &replaceComponent(Args &&...args) {
            m_entity.set<T>(T(std::forward<Args>(args)...));
            return m_entity.get_mut<T>();
        }

        /**
         * @brief Removes component.
         * @tparam T Template type used by this operation.
         */
        template <typename T> void removeComponent() {
            m_entity.remove<T>();
        }

        /**
         * @brief Returns the component.
         * @tparam T Template type used by this operation.
         * @return The result of the operation.
         */
        template <typename T> T &getComponent() {
            return m_entity.get_mut<T>();
        }

        /** @brief Stores the data for the component info. */
        struct ComponentInfo {
            /** @brief Stores the ID. */
            flecs::id_t id;
            /** @brief Stores the name. */
            std::string name;

            /**
             * @brief Returns the name.
             * @return The result of the operation.
             */
            std::string getName() const { return name; }
        };

        // Laeuft ueber flecs' eingebaute Reflection (entity.each(flecs::id))
        // statt wie zuvor ueber entt::registry::storage() - flecs bringt
        // sowas nativ mit, kein Workaround noetig.
        /**
         * @brief Returns the components.
         * @return The result of the operation.
         */
        std::vector<ComponentInfo> getComponents() const {
            std::vector<ComponentInfo> result;
            m_entity.each([&](flecs::id id) {
                result.push_back({id.raw_id(), id.str().c_str()});
            });
            return result;
        }

        // Echter void* statt des alten uintptr_t-Hacks (der frueher rohe
        // Adressen als Integer in std::any verpackt hat, was jeden Konsumenten
        // zwang, den Typ selbst wieder zurueckzucasten UND zu wissen, dass es
        // eigentlich ein Pointer war) - direkte, klar typisierte Verbesserung
        // im Zuge der Migration.
        /**
         * @brief Returns the component by ID.
         * @param id The ID used by this operation.
         * @return The result of the operation.
         */
        std::any getComponentById(flecs::id_t id) const {
            void *ptr = ecs_get_mut_id(m_scene->m_world.c_ptr(), m_entity.id(), id);
            if (!ptr) return std::any();
            return std::any(ptr);
        }

        /**
         * @brief Returns the component by component info.
         * @param info The info used by this operation.
         * @return The result of the operation.
         */
        std::any getComponentByComponentInfo(const ComponentInfo &info) const {
            return getComponentById(info.id);
        }

        /**
         * @brief Reports whether component.
         * @tparam T Template type used by this operation.
         * @return The result of the operation.
         */
        template <typename T> bool hasComponent() const {
            return m_entity.has<T>();
        }

        /**
         * @brief Defines the conversion, comparison, or access operation for this type.
         * @return The result of the operation.
         */
        operator bool() const { return m_entity.is_valid(); }

        /**
         * @brief Defines the conversion, comparison, or access operation for this type.
         * @return The result of the operation.
         */
        operator flecs::entity() const { return m_entity; }

      private:
        /** @brief Stores the entity. */
        flecs::entity m_entity{};
        /** @brief Stores the scene. */
        Scene *m_scene = nullptr;

        friend class Scene;
    };
} // namespace axiom
