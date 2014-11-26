#include "stdafx.h"
#include "gepimpl/subsystems/cameraManager.h"
#include "gep/cameras.h"

gep::CameraManager::CameraManager() :
	m_pActiveCam(nullptr)
{

}

gep::CameraManager::~CameraManager()
{

}


void gep::CameraManager::initialize()
{

}

void gep::CameraManager::destroy()
{

}

void gep::CameraManager::update(float elapsedTime)
{

}

void gep::CameraManager::setActiveCamera(ICamera* camera)
{
	m_pActiveCam = camera;
}

gep::ICamera* gep::CameraManager::getActiveCamera()
{
	return m_pActiveCam;
}
