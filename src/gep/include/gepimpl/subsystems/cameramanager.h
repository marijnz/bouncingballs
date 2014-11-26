#pragma once

#include "gep/interfaces/cameraManager.h"

namespace gep
{
	class ICamera;

	class CameraManager : public ICameraManager
	{
	public:
		CameraManager();
		~CameraManager();

		virtual void initialize();

		virtual void destroy();

		virtual void update(float elapsedTime);

		virtual void setActiveCamera(ICamera* camera);

		virtual ICamera* getActiveCamera();
	protected:
	private:

		ICamera* m_pActiveCam;

	};
}
