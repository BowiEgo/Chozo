#include "RHIDevice.h"

DEFINE_LOG_CATEGORY(LogRHIDevice);

IRHIDevice::IRHIDevice(const FRHIDeviceCreateInfo& info) : m_Info(info) {}

IRHIDevice::~IRHIDevice() { CZ_LOG(LogRHIDevice, Trace, "RHIDevice destroying..."); }