#include <Runtime/App/Application.hpp>
#include <Runtime/App/StartupHost.hpp>
#include <Runtime/RHI/RHIAPI.hpp>

namespace CZ {

bool StartupHost::IsOffscreen() const { return m_Obj->IsOffscreen(); }

Layer* StartupHost::GetStartupLayer() const { return m_Obj->GetStartupLayer(); }

void StartupHost::Draw(CommandList cmdList) { return m_Obj->Draw(cmdList); }

} // namespace CZ
