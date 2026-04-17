#include "RHIResource.h"

#include "RHIDevice.h"

IRHIResource::IRHIResource(const WeakRef<IRHIDevice> device)
    : m_Device(device), m_ID(FUUID::Generate()) {}

IRHIResource::~IRHIResource() {}