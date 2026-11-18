/**
 * @file axiom/include/axiom/Input/CustomInputDevice.h
 * @brief Declares the public interface for the custom input device.
 */

#pragma once

#include "JoystickCodes.h"

#include <string>

namespace axiom {

    /**
     * @brief Defines the interface for the custom input device.
     * @tparam ICustomInputDevice Template type used by this type.
     */
    class ICustomInputDevice {
      public:
        /**
         * @brief Releases resources owned by ICustomInputDevice.
         */
        virtual ~ICustomInputDevice() = default;

        /**
         * @brief Updates the object state.
         */
        virtual void update() {}

        /**
         * @brief Returns the name.
         * @return The result of the operation.
         */
        virtual const std::string &getName() const = 0;
        /**
         * @brief Returns the axis count.
         * @return The result of the operation.
         */
        virtual int getAxisCount() const = 0;
        /**
         * @brief Returns the button count.
         * @return The result of the operation.
         */
        virtual int getButtonCount() const = 0;
        /**
         * @brief Returns the hat count.
         * @return The result of the operation.
         */
        virtual int getHatCount() const = 0;

        /**
         * @brief Returns the axis.
         * @param axis The axis used by this operation.
         * @return The result of the operation.
         */
        virtual float getAxis(int axis) const = 0;
        /**
         * @brief Reports whether button pressed.
         * @param button The button used by this operation.
         * @return The result of the operation.
         */
        virtual bool isButtonPressed(int button) const = 0;
        /**
         * @brief Reports whether button just pressed.
         * @param button The button used by this operation.
         * @return The result of the operation.
         */
        virtual bool isButtonJustPressed(int button) const = 0;
        /**
         * @brief Reports whether button released.
         * @param button The button used by this operation.
         * @return The result of the operation.
         */
        virtual bool isButtonReleased(int button) const = 0;
        /**
         * @brief Returns the hat.
         * @param hat The hat used by this operation.
         * @return The result of the operation.
         */
        virtual JoystickHat getHat(int hat) const = 0;
        /**
         * @brief Reports whether hat just pressed.
         * @param mask The mask used by this operation.
         * @param hat The hat used by this operation.
         * @return The result of the operation.
         */
        virtual bool isHatJustPressed(JoystickHat mask, int hat) const = 0;
        /**
         * @brief Reports whether hat released.
         * @param mask The mask used by this operation.
         * @param hat The hat used by this operation.
         * @return The result of the operation.
         */
        virtual bool isHatReleased(JoystickHat mask, int hat) const = 0;
    };

} // namespace axiom
