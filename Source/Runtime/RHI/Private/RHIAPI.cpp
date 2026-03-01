#include "RHIAPI.h"

DEFINE_LOG_CATEGORY(LogRHIAPI);

IRHIAPI* IRHIAPI::s_Instance = nullptr;

IRHIAPI::~IRHIAPI() { CZ_LOG(LogRHIAPI, Trace, "RHIAPI destroying..."); }
