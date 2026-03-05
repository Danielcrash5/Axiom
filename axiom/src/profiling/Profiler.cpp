#include "axiom/profiling/Profiler.h"
#include "axiom/core/Logger.h"

#include <chrono>
#include <thread>
#include <mutex>

namespace axiom::profiling {

    using Clock = std::chrono::steady_clock;

    static constexpr uint32_t MaxFrames = 120;

    struct ThreadState {
        std::vector<CPUEvent> Events;
        std::vector<uint32_t> EventStack;
        uint32_t Depth = 0;
    };

    static thread_local ThreadState g_ThreadState;

    struct ProfilerState {
        std::array<FrameData, MaxFrames> Frames;
        uint32_t FrameIndex = 0;

        uint64_t FrameStart = 0;

        std::mutex Mutex;
    };

    static ProfilerState g_State;

    static uint64_t Now() {
        return std::chrono::duration_cast<std::chrono::nanoseconds>(
            Clock::now().time_since_epoch()
        ).count();
    }

    double CPUEvent::DurationMs() const {
        return (End - Start) / 1000000.0;
    }

    static uint32_t GetThreadID() {
        static std::hash<std::thread::id> hasher;
        return (uint32_t)hasher(std::this_thread::get_id());
    }

    void Profiler::PrintLastFrame() {
        const FrameData& frame = GetFrame(0);

        AXIOM_INFO("Frame Time: {} ms", frame.FrameTimeMs);

        for (const auto& event : frame.CPUEvents) {
            std::string indent(event.Depth * 2, ' ');

            AXIOM_INFO("{}{} - {:.3f} ms",
                       indent,
                       event.Name,
                       event.DurationMs());
        }
    }

    void Profiler::Initialize() {
        static bool initialized = false;

        if (!initialized) {
            AXIOM_INFO("Axiom Profiler initialized");
            initialized = true;
        }
    }

    void Profiler::BeginFrame() {
        Initialize();

        auto& frame = g_State.Frames[g_State.FrameIndex];
        frame.CPUEvents.clear();

        g_State.FrameStart = Now();
    }

    void Profiler::EndFrame() {
        uint64_t frameEnd = Now();

        auto& frame = g_State.Frames[g_State.FrameIndex];

        frame.FrameTimeMs =
            (frameEnd - g_State.FrameStart) / 1000000.0;

        {
            std::lock_guard lock(g_State.Mutex);

            frame.CPUEvents.insert(
                frame.CPUEvents.end(),
                g_ThreadState.Events.begin(),
                g_ThreadState.Events.end()
            );
        }

        g_ThreadState.Events.clear();

        g_State.FrameIndex =
            (g_State.FrameIndex + 1) % MaxFrames;

        if (!g_ThreadState.EventStack.empty()) {
            AXIOM_WARN("Profiler: Unclosed profiling scopes detected!");
            g_ThreadState.EventStack.clear();
        }
    }

    void Profiler::PushEvent(std::string_view name) {
        CPUEvent event {};
        event.Name = name.data();
        event.Start = Now();
        event.ThreadID = GetThreadID();
        event.Depth = g_ThreadState.Depth++;

        g_ThreadState.Events.push_back(event);

        g_ThreadState.EventStack.push_back(
            static_cast<uint32_t>(g_ThreadState.Events.size() - 1)
        );
    }

    void Profiler::PopEvent() {
        if (g_ThreadState.EventStack.empty())
            return;

        uint64_t end = Now();

        uint32_t index = g_ThreadState.EventStack.back();
        g_ThreadState.EventStack.pop_back();

        g_ThreadState.Events[index].End = end;

        g_ThreadState.Depth--;
    }

    const FrameData& Profiler::GetFrame(uint32_t index) {
        uint32_t i =
            (g_State.FrameIndex + MaxFrames - index - 1) % MaxFrames;

        return g_State.Frames[i];
    }

    CPUScope::CPUScope(std::string_view name) {
        Profiler::PushEvent(name);
    }

    CPUScope::~CPUScope() {
        Profiler::PopEvent();
    }

}