#include "RHIContext.h"

#include "RHIDevice.h"

DEFINE_LOG_CATEGORY(LogRHIContext);

IRHIContext::IRHIContext(const FContextSpec& spec) : m_Spec(spec) {}

IRHIContext::~IRHIContext() { CZ_LOG(LogRHIContext, Trace, "RHIContext destroying..."); }
