#ifndef C_WRAPPER_H_
#define C_WRAPPER_H_


#ifdef __cplusplus
    #include "stepper.hpp"
extern "C" {
#else
    typedef struct Step_wrap Step_wrap;
#endif



// Callback function type compatible with C.
typedef void (*CallbackFunction)(void* userData);

// Function to create an instance of the C++ class.
Stepper* Step_wrap_Create();

// Function to destroy an instance of the C++ class.
void Step_wrap_Destroy(Stepper* obj);

// Function to set the callback and user data.
void Step_wrap_SetCallback(Stepper* obj, CallbackFunction callback, void* userData);

// Function to trigger the callback.
//void Step_wrap_TriggerCallback(Step_wrap* obj);

#ifdef __cplusplus
}
#endif

#endif /* C_WRAPPER_H_ */
