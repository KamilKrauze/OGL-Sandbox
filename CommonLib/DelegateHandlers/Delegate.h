#ifndef DELEGATE_H
#define DELEGATE_H

#include <functional>
#include <vector>

#include "CallbackPayload.h"

#define DEFINE_DELEGATE_TwoParam(NAME, PAYLOAD_TYPENAME, TYPE1, NAME1, TYPE2, NAME2) DEFINE_PAYLOAD_TwoParam(PAYLOAD_TYPENAME, TYPE1, NAME1, TYPE2, NAME2) \
template<typename PayloadT> \
struct __##NAME \
{ \
private: \
    std::vector<std::function<void(PayloadT&)>> callbacks; \
 \
public: \
    template<typename ObjectT, typename... Args> \
    void Bind(ObjectT* instanceRef, void (ObjectT::* func)(PayloadT&, Args...), Args... args) \
    { \
        auto& function = [=](PayloadT& payload) \
        { \
            (instanceRef->*func)(payload, args...); \
        }; \
 \
        callbacks.emplace_back(function); \
    } \
     \
    void Broadcast(PayloadT& payload) \
    { \
        for(auto func : callbacks) \
        { \
            func(payload); \
        } \
    } \
}; \
using NAME = __##NAME<PAYLOAD_TYPENAME> \

#endif //!DELEGATE_H