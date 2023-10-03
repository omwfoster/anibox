#include "stepper/stepper_C.h"
#include "stepper/stepper.hpp" // Include your original C++ class header.

extern "C" {

Step_wrap* Step_wrap_Create() {
    return new Stepper(); // Create an instance of the C++ class.
}

void Step_wrap_Destroy(Step_wrap* obj) {
    Stepper* instance = reinterpret_cast<Stepper*>(obj);
    delete instance; // Destroy the instance.
}

void Step_wrap_SetCallback(Step_wrap* obj, CallbackFunction callback, void* userData) {
    Stepper* instance = reinterpret_cast<Stepper*>(obj);
    instance->SetCallback(callback, userData);
}

void Step_wrap_TriggerCallback(Step_wrap_* obj) {
    Stepper* instance = reinterpret_cast<Stepper*>(obj);
    instance->TriggerCallback();
}

}
