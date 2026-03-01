#include "RHIDevice.h"

DEFINE_LOG_CATEGORY(LogRHIDevice);

IRHIDevice::IRHIDevice(const FDeviceSpecification& spec) : m_Spec(spec) {}

IRHIDevice::~IRHIDevice() { CZ_LOG(LogRHIDevice, Trace, "RHIDevice destroying..."); }