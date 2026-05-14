#include <Runtime/App/Application.h>
#include <Runtime/App/StartupHost.h>
#include <Runtime/RHI/RHIAPI.h>

namespace CZ {

DEFINE_HANDLE_DESTROY(StartupHostObj)

bool StartupHost::IsOffscreen() const { return m_Obj->IsOffscreen(); }

Layer* StartupHost::GetStartupLayer() const { return m_Obj->GetStartupLayer(); }

void StartupHost::Draw(CommandList cmdList) { return m_Obj->Draw(cmdList); }

} // namespace CZ
