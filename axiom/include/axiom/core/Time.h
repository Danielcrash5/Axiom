/**
 * @file axiom/include/axiom/core/Time.h
 * @brief Declares the public interface for the time.
 */

#pragma once

namespace axiom {
    /**
     * @brief Provides the time functionality.
     * @tparam Time Template type used by this type.
     */
    class Time {
      public:
        /**
         * @brief Updates the object state.
         */
        static void update();

        /**
         * @brief Returns the delta time.
         * @return The result of the operation.
         */
        static double getDeltaTime() { return sDeltaTime; }
        /**
         * @brief Returns the time.
         * @return The result of the operation.
         */
        static double getTime() { return sTime; }

      private:
        /** @brief Stores the time. */
        static double sTime;
        /** @brief Stores the last time. */
        static double sLastTime;
        /** @brief Stores the delta time. */
        static double sDeltaTime;
    };
} // namespace axiom