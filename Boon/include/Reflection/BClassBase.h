#pragma once

#define BCLASS(...)
#define BPROPERTY(...)
#define BFUNCTION(...)

#ifndef BOON_MODULE_NAME
#error BOON_MODULE_NAME must be defined for reflected targets
#endif

#define BOON_PP_CAT_IMPL(a, b) a##b
#define BOON_PP_CAT(a, b) BOON_PP_CAT_IMPL(a, b)

#define BOON_REGISTER_FN_NAME(module) BOON_PP_CAT(RegisterGeneratedClasses_, module)
#define BOON_UNREGISTER_FN_NAME(module) BOON_PP_CAT(UnregisterGeneratedClasses_, module)

#define BOON_REGISTER_FN BOON_REGISTER_FN_NAME(BOON_MODULE_NAME)
#define BOON_UNREGISTER_FN BOON_UNREGISTER_FN_NAME(BOON_MODULE_NAME)

namespace Boon
{
    class BClassRegistry;
    class NetRepRegistry;

    void BOON_REGISTER_FN(BClassRegistry&, NetRepRegistry&);
    void BOON_UNREGISTER_FN(BClassRegistry&, NetRepRegistry&);
}

#define BCLASS_BODY() \
    friend void Boon::BOON_REGISTER_FN(Boon::BClassRegistry&, Boon::NetRepRegistry&); \
    friend void Boon::BOON_UNREGISTER_FN(Boon::BClassRegistry&, Boon::NetRepRegistry&);