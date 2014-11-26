#pragma once

#include "gep/interfaces/subsystem.h"
#include "gep/cameras.h"
namespace gep
{
	class GEP_API ICameraManager : public ISubsystem
	{
	public:
		ICameraManager() {}
		virtual ~ICameraManager() = 0 {}

		virtual void initialize() = 0;

		virtual void destroy() = 0;

		virtual void update(float elapsedTime) = 0;

		virtual void setActiveCamera(ICamera* camera) = 0;

		virtual ICamera* getActiveCamera() = 0;

	};
}
