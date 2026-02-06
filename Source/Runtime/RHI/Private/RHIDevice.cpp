#include "RHIDevice.h"

IRHIDevice::IRHIDevice(const FRHIDeviceCreateInfo& info) : m_Data(info) {}

IRHIDevice::~IRHIDevice() = default;