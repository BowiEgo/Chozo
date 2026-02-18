#include "RHICommandBuffer.h"

IRHICommandBuffer::IRHICommandBuffer(const FRHICommandBufferCreateInfo& info) : m_Info(info) {}

IRHICommandBuffer::~IRHICommandBuffer() {}
