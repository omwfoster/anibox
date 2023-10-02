#ifdef __cplusplus
extern "C" {
#endif

// Define a struct to act as an opaque handle to the C++ class.
typedef struct MyClassWrapper MyClassWrapper;

// Function to create an instance of the class.
MyClassWrapper* MyClass_Create();

// Function to destroy an instance of the class.
void MyClass_Destroy(MyClassWrapper* obj);

// Function to call the member function.
void MyClass_MyCallbackFunction(MyClassWrapper* obj);

#ifdef __cplusplus
}
#endif