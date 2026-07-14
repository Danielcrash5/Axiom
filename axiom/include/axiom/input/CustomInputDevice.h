#pragma once

#include "JoystickCodes.h"

#include <string>

namespace axiom {

    class ICustomInputDevice {
      public:
        virtual ~ICustomInputDevice() = default;

        virtual void Update() {}

        virtual const std::string &GetName() const = 0;
        virtual int GetAxisCount() const = 0;
        virtual int GetButtonCount() const = 0;
        virtual int GetHatCount() const = 0;

        virtual float GetAxis(int axis) const = 0;
        virtual bool IsButtonPressed(int button) const = 0;
        virtual bool IsButtonJustPressed(int button) const = 0;
        virtual bool IsButtonReleased(int button) const = 0;
        virtual JoystickHat GetHat(int hat) const = 0;
        virtual bool IsHatJustPressed(JoystickHat mask, int hat) const = 0;
        virtual bool IsHatReleased(JoystickHat mask, int hat) const = 0;
    };

} // namespace axiom
