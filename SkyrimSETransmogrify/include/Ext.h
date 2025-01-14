
#define MEMBER_FN_PREFIX(className) typedef className _MEMBER_FN_BASE_TYPE
#define DEFINE_MEMBER_FN_LONG(className, functionName, retnType, address, ...)                                         \
    typedef retnType (className::*_##functionName##_type)(__VA_ARGS__);                                                \
                                                                                                                       \
    inline _##functionName##_type* _##functionName##_GetPtr(void)                                                      \
    {                                                                                                                  \
        static const UInt32 _address = address;                                                                        \
        return (_##functionName##_type*)&_address;                                                                     \
    }

#define DEFINE_MEMBER_FN(functionName, retnType, address, ...)                                                         \
    DEFINE_MEMBER_FN_LONG(_MEMBER_FN_BASE_TYPE, functionName, retnType, address, __VA_ARGS__)

#define CALL_MEMBER_FN(obj, fn) ((*(obj)).*(*((obj)->_##fn##_GetPtr())))