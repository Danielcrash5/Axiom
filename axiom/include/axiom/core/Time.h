#pragma once

class Time {
public:
    static void Update();

    static double GetDeltaTime() { return s_DeltaTime; }
    static double GetTime() { return s_Time; }

private:
    static double s_Time;
    static double s_LastTime;
    static double s_DeltaTime;
};