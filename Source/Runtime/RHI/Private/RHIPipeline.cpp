#include "RHIPipeline.h"

DEFINE_LOG_CATEGORY(LogRHIPipeline);

IRHIPipeline::IRHIPipeline(const FPipelineSpecification& spec) : m_Spec(spec) {}

IRHIPipeline::~IRHIPipeline() {
    CZ_LOG(LogRHIPipeline, Trace, "RHIPipeline destroying...");
    m_Spec.RHIShaders.clear();
}