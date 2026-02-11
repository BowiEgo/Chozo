#include "RHI.h"

DEFINE_LOG_CATEGORY(LogRHI);

IRHI::IRHI() {}

IRHI::~IRHI() { CZ_LOG(LogRHI, Trace, "RHI destroying..."); }