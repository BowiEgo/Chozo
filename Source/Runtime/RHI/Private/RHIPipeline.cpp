#include "RHIPipeline.h"

IRHIPipeline::IRHIPipeline(const FRHIPipelineCreateInfo& info)
    : m_Name(info.Name) {}

IRHIPipeline::~IRHIPipeline() = default;