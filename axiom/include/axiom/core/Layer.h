#pragma once

#include <string>

#include "axiom/events/EventBus.h"

namespace axiom {

	class Event;

	class Layer {
	public:
		explicit Layer(std::string name = "Layer")
			: m_Name(std::move(name)) {}

		virtual ~Layer() = default;

		virtual void OnAttach() {}
		virtual void OnDetach() {}

		virtual void OnPreUpdate(float dt) {}
		virtual void OnPostUpdate(float dt) {}
		virtual void OnFixedUpdate(float dt) {}
		virtual void OnUpdate(float dt) {}

		virtual void OnRender() {}


		void setEventBus(EventBus& eventBus) {
			m_EventBus = &eventBus;
		}

		const std::string& GetName() const {
			return m_Name;
		}

	protected:
		std::string m_Name;
		EventBus* m_EventBus = nullptr;
	};

}