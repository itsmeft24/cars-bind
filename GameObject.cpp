#include <memory>

#include "GameObject.hpp"

#ifdef CARS_BIND
static int* SearchesOpen = reinterpret_cast<int*>(0x007161A8);
static int* TotalGameObjects = reinterpret_cast<int*>(0x007161AC);
#else
static std::unique_ptr<int> SearchesOpen = std::make_unique<int>();
static std::unique_ptr<int> TotalGameObjects = std::make_unique<int>();
#endif // CARS_BIND

constexpr std::uint32_t kWantsNothing = 0x0;
constexpr std::uint32_t kWantsTickWhileInactive = 0x1;
constexpr std::uint32_t kWantsTick = 0x2;
constexpr std::uint32_t kWantsRenderPre3D = 0x4;
constexpr std::uint32_t kWantsRender3D = 0x8;
constexpr std::uint32_t kWantsRenderPost3D = 0x10;
constexpr std::uint32_t kWantsPause = 0x20;
constexpr std::uint32_t kWantsLostFocus = 0x40;
constexpr std::uint32_t kWantsRestore = 0x80;
constexpr std::uint32_t kWantsEventHandler = 0x100;
constexpr std::uint32_t kWantsKeyUpHandler = 0x200;
constexpr std::uint32_t kWantsKeyDownHandler = 0x400;
constexpr std::uint32_t kWantsPacketHandler = 0x800;
constexpr std::uint32_t kWantsPrepareGeometry = 0x1000;
constexpr std::uint32_t kWantsAllEvents = 0xffffffff;

std::int32_t GameObject::Release()
{
	return std::int32_t();
}

void GameObject::Disable()
{
	flags.enabled = false;
	flags.packetEnabled = false;
	AncestorsDisabled();
}

void GameObject::DisableAll()
{
	this->flags.enabled = false;
	this->flags.packetEnabled = false;
	GameObject* child = this->firstChild;
	while (child != nullptr) {
		if (!child->flags.killMe) {
			child->DisableAll();
		}
		child = child->next;
	}
}

void GameObject::Enable()
{
	flags.enabled = true;
	flags.packetEnabled = true;
	AncestorsEnabled();
}

void GameObject::EnableAll()
{
	this->flags.enabled = true;
	this->flags.packetEnabled = true;
	GameObject* child = this->firstChild;
	while (child != nullptr) {
		if (!child->flags.killMe) {
			child->EnableAll();
		}
		child = child->next;
	}
}

void GameObject::AncestorsDisabled()
{
	GameObject* child = this->firstChild;
	while (child != nullptr) {
		if (!child->flags.killMe) {
			child->AncestorsDisabled();
		}
		child = child->next;
	}
}

void GameObject::AncestorsEnabled()
{
	GameObject* child = this->firstChild;
	while (child != nullptr) {
		if (!child->flags.killMe) {
			child->AncestorsEnabled();
		}
		child = child->next;
	}
}

void GameObject::EnableEvents(std::uint32_t param_1)
{
	descendantsWantedEventMask |= param_1;
	if (parent != nullptr) {
		parent->descendantsWantedEventMask = parent->descendantsWantedEventMask | this->descendantsWantedEventMask | this->wantedEventMask;
		UpPropogateMasks();
	}
	return;
}

void GameObject::DisableEvents(std::uint32_t mask)
{
	this->wantedEventMask &= ~mask;
}

GameObject* GameObject::Create()
{
	return this;
}

Bool GameObject::TickWhileInactive(float deltaSeconds)
{
	if (descendantsWantedEventMask & kWantsTickWhileInactive) {
		GameObject* child = firstChild;
		while (child != nullptr) {
			if (child->flags.isPaused && !child->flags.isPaused && !child->flags.killMe) {
				child->TickWhileInactive(deltaSeconds);
			}
			child = child->next;
		}
	}
	return TRUE;
}

Bool GameObject::Tick(float deltaSeconds)
{
	if ((this->descendantsWantedEventMask & kWantsTick) != 0) {
		for (GameObject* child = this->firstChild; child != nullptr; child = child->next) {
			if (child->flags.enabled && !child->flags.isPaused && !child->flags.killMe) {
				if ((child->wantedEventMask & kWantsTick) == 0) {
					if ((child->descendantsWantedEventMask & kWantsTick) != 0) {
						child->Tick(deltaSeconds);
					}
				}
				else {
					child->Tick(deltaSeconds);
				}
			}
		}
		return 1;
	}
	return 1;
}

Bool GameObject::SimulationTick(float deltaSeconds)
{
	return TRUE;
}

Bool GameObject::PrepareGeometry()
{
	if ((this->descendantsWantedEventMask & kWantsPrepareGeometry) == 0) {
		return TRUE;
	}
	for (GameObject*  child = this->firstChild; child != nullptr; child = child->next) {
		if (child->flags.enabled && !child->flags.killMe) {
			if ((child->wantedEventMask & kWantsPrepareGeometry) == 0) {
				if ((child->descendantsWantedEventMask & kWantsPrepareGeometry) != 0) {
					child->PrepareGeometry();
				}
			}
			else {
				child->PrepareGeometry();
			}
		}
	}
	return TRUE;
}

Bool GameObject::RenderPre3D()
{
	if ((this->descendantsWantedEventMask & kWantsRenderPre3D) == 0) {
		return TRUE;
	}
	for (GameObject* child = this->firstChild; child != nullptr; child = child->next) {
		if (child->flags.enabled && !child->flags.killMe) {
			if ((child->wantedEventMask & kWantsRenderPre3D) == 0) {
				if ((child->descendantsWantedEventMask & kWantsRenderPre3D) != 0) {
					child->RenderPre3D();
				}
			}
			else {
				child->RenderPre3D();
			}
		}
	}
	return TRUE;
}

Bool GameObject::Render3D(int userData)
{
	if ((this->descendantsWantedEventMask & kWantsRender3D) == 0) {
		return TRUE;
	}
	GameObject* child = this->firstChild;
	if (child == nullptr) {
		return TRUE;
	}
	do {
		if (child->flags.enabled && !child->flags.killMe) {
			if ((child->wantedEventMask & kWantsRender3D) == 0) {
				if ((child->descendantsWantedEventMask & kWantsRender3D) != 0) {
					child->Render3D(userData);
				}
			}
			else {
				child->Render3D(userData);
			}
		}
		child = child->next;
	} while (child != nullptr);
	return TRUE;
}

Bool GameObject::RenderPost3D()
{
	if ((this->descendantsWantedEventMask & kWantsRenderPost3D) == 0) {
		return TRUE;
	}
	for (GameObject* child = this->firstChild; child != nullptr; child = child->next) {
		if (child->flags.enabled && !child->flags.killMe) {
			if ((child->wantedEventMask & kWantsRenderPost3D) == 0) {
				if ((child->descendantsWantedEventMask & kWantsRenderPost3D) != 0) {
					child->RenderPost3D();
				}
			}
			else {
				child->RenderPost3D();
			}
		}
	}
	return TRUE;
}

Bool GameObject::Pause(Bool bPaused)
{
	this->flags.isPaused = bPaused;
	if ((this->descendantsWantedEventMask & kWantsPause) == 0) {
		return TRUE;
	}
	for (GameObject* child = this->firstChild; child != nullptr; child = child->next) {
		if (!child->flags.killMe && ((child->wantedEventMask & kWantsPause) != 0)) {
			child->Pause(bPaused);
		}
	}
	return TRUE;
}

Bool GameObject::LostFocus()
{
	if ((this->descendantsWantedEventMask & kWantsLostFocus) != 0) {
		for (GameObject* child = this->firstChild; child != nullptr; child = child->next) {
			if (!child->flags.killMe) {
				child->LostFocus();
			}
		}
		return TRUE;
	}
	return TRUE;
}

Bool GameObject::Restore()
{
	if ((this->descendantsWantedEventMask & kWantsRestore) != 0) {
		for (GameObject* child = this->firstChild; child != nullptr; child = child->next) {
			if (!child->flags.killMe) {
				child->Restore();
			}
		}
		return TRUE;
	}
	return TRUE;
}

Bool GameObject::EventHandler(std::int32_t eventNumber)
{
	if ((this->descendantsWantedEventMask & kWantsEventHandler) != 0) {
		for (GameObject* child = this->firstChild; child != nullptr; child = child->next) {
			if (child->flags.enabled && !child->flags.killMe && child->EventHandler(eventNumber)) {
				return TRUE;
			}
		}
	}
	return FALSE;
}

Bool GameObject::KeyUpHandler(ProcessNode* event, KeyInfo* keyInfo)
{
	if ((this->descendantsWantedEventMask & kWantsKeyUpHandler) != 0) {
		for (GameObject* child = this->firstChild; child != nullptr; child = child->next) {
			if ((child->flags.enabled && !child->flags.killMe) && child->KeyUpHandler(event, keyInfo)) {
				return TRUE;
			}
		}
	}
	return FALSE;
}

Bool GameObject::KeyDownHandler(ProcessNode* event, KeyInfo* keyInfo)
{
	if ((this->descendantsWantedEventMask & kWantsKeyDownHandler) != 0) {
		for (GameObject* child = this->firstChild; child != nullptr; child = child->next) {
			if ((child->flags.enabled && !child->flags.killMe) && child->KeyDownHandler(event, keyInfo)) {
				return TRUE;
			}
		}
	}
	return FALSE;
}

Bool GameObject::PacketHandler(std::uint32_t message, void* lpMsg, std::uint32_t idFrom, std::uint32_t idTo, std::uint32_t receiveTime)
{

	if ((this->descendantsWantedEventMask & kWantsPacketHandler) != 0) {
		for (GameObject* child = this->firstChild; child != nullptr; child = child->next) {
			if ((child->flags.packetEnabled && !child->flags.killMe) && child->PacketHandler(message, lpMsg, idFrom, idTo, receiveTime)) {
				return TRUE;
			}
		}
	}
	return FALSE;
}

void GameObject::Kill()
{
	for (GameObject* object = this->firstChild; object != nullptr; object = object->next) {
		object->Kill();
	}
	this->flags.killMe = true;
}

void GameObject::DumpHierarchy(char*, DumpType, std::int32_t)
{
	UNIMPLEMENTED(0x00583e70);
}

GameObject::GameObject(bool startEnabled)
{
	(*TotalGameObjects)++;
	this->prev = nullptr;
	this->next = nullptr;
	this->firstChild = nullptr;
	this->parent = nullptr;
	this->wantedEventMask = 0;
	this->descendantsWantedEventMask = 0;
	this->flags.enabled = startEnabled;
	this->flags.packetEnabled = startEnabled;
	this->flags.isPaused = false;
	this->flags.killMe = false;
	this->refCount = 1;
	this->typeID = 0xFF;
}

void GameObject::UpPropogateMasks()
{
	GameObject* object = this;
	while (object->parent != nullptr) {
		object->parent->descendantsWantedEventMask = object->parent->descendantsWantedEventMask | object->descendantsWantedEventMask | object->wantedEventMask;
		object = this->parent;
	}
	return;
}

GameObject::~GameObject()
{
	(*TotalGameObjects)--;
}
