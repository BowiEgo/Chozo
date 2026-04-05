#include "RHIImage.h"

DEFINE_LOG_CATEGORY(LogRHIImage);

IRHIImage::IRHIImage(const WeakRef<IRHIDevice> device, const FImageSpecification& spec)
    : IRHIResource(device), m_Spec(spec) {}

IRHIImage::~IRHIImage() { CZ_LOG(LogRHIImage, Trace, "RHIImage destroying..."); }
