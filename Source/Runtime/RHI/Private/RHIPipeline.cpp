#include "RHIPipeline.h"

DEFINE_LOG_CATEGORY(LogRHIPipeline);

IRHIPipeline::IRHIPipeline(const FRHIPipelineCreateInfo& info) : m_Info(info) {}

IRHIPipeline::~IRHIPipeline() {
    CZ_LOG(LogRHIPipeline, Trace, "RHIPipeline destroying...");
    m_Info.RHIShaders.clear();
}