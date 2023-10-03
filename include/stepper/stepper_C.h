#ifdef __cplusplus
extern "C" {
#endif

// Define a struct to act as an opaque handle to the C++ class.
typedef struct Step_wrap Step_wrap;

// Callback function type compatible with C.
typedef void (*CallbackFunction)(void* userData);

// Function to create an instance of the C++ class.
Step_wrap* Step_wrap_Create();

// Function to destroy an instance of the C++ class.
void Step_wrap_Destroy(Step_wrap* obj);

// Function to set the callback and user data.
void Step_wrap_SetCallback(Step_wrap* obj, CallbackFunction callback, void* userData);

// Function to trigger the callback.
void MyClass_TriggerCallback(MyClassWrapper* obj);

#ifdef __cplusplus
}
#endif
