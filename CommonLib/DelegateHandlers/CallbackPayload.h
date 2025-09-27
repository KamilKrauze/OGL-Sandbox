#ifndef CALLBACK_PAYLOADS_H
#define CALLBACK_PAYLOADS_H

struct CallbackPayloadBase
{
    CallbackPayloadBase() = default;
    ~CallbackPayloadBase() = default;
};

template<typename PayloadT>
struct CallbackPayload : public CallbackPayloadBase
{
    CallbackPayload() {}
    ~CallbackPayload() {}
};

#define DEFINE_PAYLOAD_TwoParam(NAME, TYPE1, NAME1, TYPE2, NAME2) struct NAME : CallbackPayload<NAME> \
{ \
    TYPE1 NAME1; \
    TYPE2 NAME2; \
    NAME(TYPE1 NAME1, TYPE2 NAME2) \
    { \
        this->NAME1 = NAME1; \
        this->NAME2 = NAME2; \
    } \
    \
    ~NAME() = default; \
    \
}; \

#endif // !CALLBACK_PAYLOADS_H