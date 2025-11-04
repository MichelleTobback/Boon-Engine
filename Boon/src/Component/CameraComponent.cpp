#include "Component/CameraComponent.h"

using namespace Boon;

Boon::CameraComponent::CameraComponent(bool viewportSize)
	: CameraComponent(Boon::Camera(1.f, 1.f), viewportSize){}

Boon::CameraComponent::CameraComponent(const Boon::Camera& camera, bool viewportSize)
	: Camera{ camera }, ViewportSize{viewportSize}
{
	
}

Boon::CameraComponent::~CameraComponent()
{
	
}