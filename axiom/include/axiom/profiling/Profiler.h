#pragma once

#include <vector>
#include <string_view>
#include <array>
#include <cstdint>
#include <source_location>

namespace axiom::profiling {

    struct CPUEvent {
        const char* Name;
        uint64_t Start;
        uint64_t End;
        uint32_t ThreadID;
        uint32_t Depth;

        double DurationMs() const;
    };

    struct FrameData {
        std::vector<CPUEvent> CPUEvents;
        double FrameTimeMs = 0.0;
    };

    class Profiler {
    public:

        static void BeginFrame();
        static void EndFrame();

        static void PushEvent(std::string_view name);
        static void PopEvent();

        static void PrintLastFrame();

        static const FrameData& GetFrame(uint32_t index);

    private:

        static void Initialize();

    };

    class CPUScope {
    public:

        CPUScope(std::string_view name);
        ~CPUScope();

    };

} // namespace axiom::profiling


#define AXIOM_PROFILE_SCOPE(name) \
    axiom::profiling::CPUScope scope##__LINE__(name)

#define AXIOM_PROFILE_FUNCTION() \
    axiom::profiling::CPUScope scope##__LINE__(std::source_location::current().function_name())

#define AXIOM_PROFILE_FRAME() \
    AXIOM_PROFILE_SCOPE("Frame")