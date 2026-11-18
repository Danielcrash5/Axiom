/**
 * @file axiom/include/axiom/profiling/Profiler.h
 * @brief Declares the public interface for the profiler.
 */

#pragma once

#include <array>
#include <cstdint>
#include <source_location>
#include <string_view>
#include <vector>

namespace axiom::profiling {

    /** @brief Carries the data for the CPU event event. */
    struct CPUEvent {
        /** @brief Stores the name. */
        const char *Name;
        /** @brief Stores the start. */
        uint64_t Start;
        /** @brief Stores the end. */
        uint64_t End;
        /** @brief Stores the thread ID. */
        uint32_t ThreadID;
        /** @brief Stores the depth. */
        uint32_t Depth;

        /**
         * @brief Performs the duration ms operation.
         * @return The result of the operation.
         */
        double durationMs() const;
    };

    /** @brief Stores the data for the frame data. */
    struct FrameData {
        /** @brief Stores the CPU events. */
        std::vector<CPUEvent> CPUEvents;
        /** @brief Stores the frame time ms. */
        double FrameTimeMs = 0.0;
    };

    /**
     * @brief Collects timing and frame profiling data.
     * @tparam Profiler Template type used by this type.
     */
    class Profiler {
      public:
        /**
         * @brief Begins frame.
         */
        static void beginFrame();
        /**
         * @brief Ends frame.
         */
        static void endFrame();

        /**
         * @brief Adds event.
         * @param name The name used by this operation.
         */
        static void pushEvent(std::string_view name);
        /**
         * @brief Removes event.
         */
        static void popEvent();

        /**
         * @brief Performs the print last frame operation.
         */
        static void printLastFrame();

        /**
         * @brief Returns the frame.
         * @param index The index used by this operation.
         * @return The result of the operation.
         */
        static const FrameData &getFrame(uint32_t index);

      private:
        /**
         * @brief Initializes ialize.
         */
        static void initialize();
    };

    /**
     * @brief Provides the CPU scope functionality.
     * @tparam CPUScope Template type used by this type.
     */
    class CPUScope {
      public:
        /**
         * @brief Creates an instance of CPU scope.
         * @param name The name used by this operation.
         */
        CPUScope(std::string_view name);
        /**
         * @brief Releases resources owned by CPUScope.
         */
        ~CPUScope();
    };

} // namespace axiom::profiling

/** @brief Provides the AXIOM_PROFILE_SCOPE macro. */
#define AXIOM_PROFILE_SCOPE(name)                                              \
    axiom::profiling::CPUScope scope##__LINE__(name)

/** @brief Provides the AXIOM_PROFILE_FUNCTION macro. */
#define AXIOM_PROFILE_FUNCTION()                                               \
    axiom::profiling::CPUScope scope##__LINE__(                                \
        std::source_location::current().function_name())

/** @brief Provides the AXIOM_PROFILE_FRAME macro. */
#define AXIOM_PROFILE_FRAME() AXIOM_PROFILE_SCOPE("Frame")