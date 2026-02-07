#include "RHIDevice.h"

IRHIDevice::IRHIDevice(const FRHIDeviceCreateInfo& info) : m_Info(info) {}

IRHIDevice::~IRHIDevice() = default;