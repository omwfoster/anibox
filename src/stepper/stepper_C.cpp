#include "stepper/stepper_C.h"
#include "stepper/stepper.hpp" // Include your original C++ class header.

extern "C"
{

    Stepper *Step_wrap_Create()
    {
        return new Stepper(); // Create an instance of the C++ class.
    }

    void Step_wrap_Destroy(Stepper *obj)
    {
        Stepper *instance = reinterpret_cast<Stepper *>(obj);
        delete instance; // Destroy the instance.
    }

    void Step_wrap_SetCallback(Stepper *obj, CallbackFunction callback, void *userData)
    {
        Stepper *instance = reinterpret_cast<Stepper *>(obj);
        instance->SetCallback(callback, userData);
        instance->stop();
    }

    void Step_wrap_TriggerCallback(Stepper *obj)
    {
        Stepper *instance = reinterpret_cast<Stepper *>(obj);
     //   instance->TriggerCallback();
    }
}
