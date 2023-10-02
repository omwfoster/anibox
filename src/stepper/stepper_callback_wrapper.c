#include "stepper/stepper_callback_wrapper.h"
#include "stepper/stepper.hpp" // Include your original C++ class header.

extern "C" {

MyClassWrapper* MyClass_Create() {
    return new Stepper(); // Create an instance of the C++ class.
}

void MyClass_Destroy(MyClassWrapper* obj) {
    MyClass* instance = reinterpret_cast<MyClass*>(obj);
    delete instance; // Destroy the instance.
}

void MyClass_MyCallbackFunction(MyClassWrapper* obj) {
    MyClass* instance = reinterpret_cast<MyClass*>(obj);
    instance->MyCallbackFunction(); // Call the C++ member function.
}

}