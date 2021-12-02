// By @ByCubed7 on Twitter

#include "Component.h"
#include "Object.h"

Component::Component(GameObject* gameObject, std::string name)
    : Object(name)
{
	this->gameObject = gameObject;

	// Send the parent a reference.
	gameObject->AddComponent(this);
}

void Component::Update(double delta, Game& game) {}
void Component::Draw(Render::Renderers& renderer) {}
