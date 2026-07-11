#pragma once
#include <memory>
#include "IRHIBackend.h"

namespace axiom::renderer::rhi {

	// Einziger öffentlicher Einstiegspunkt für WebGPU – der Rückgabetyp
	// ist reines IRHIBackend, keine Dawn-Typen tauchen in der Public-API auf.
	[[nodiscard]] RHIResult<std::unique_ptr<IRHIBackend>> createVulkanBackend();

} // namespace axiom::renderer::rhi