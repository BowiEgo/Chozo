#include "RHICommandList.h"

DEFINE_LOG_CATEGORY(LogRHICommandList);

IRHICommandList::IRHICommandList(const FRHICommandListCreateInfo& info) : m_Info(info) {}

IRHICommandList::~IRHICommandList() {
    CZ_LOG(LogRHICommandList, Trace, "RHICommandList destroying...");
}
