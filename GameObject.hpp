#pragma once
#include "BaseObject.hpp"

struct ProcessNode;
struct KeyInfo;
enum DumpType {};

struct GameObjectFlags {
	bool enabled : 1;
	bool packetEnabled : 1;
	bool isPaused : 1;
	bool killMe : 1;
};
static_assert(sizeof(GameObjectFlags) == 1);

class GameObject : public BaseObject
{
public:
	virtual ~GameObject() override;
	virtual std::int32_t Release() override;
	virtual void Disable();
	virtual void DisableAll();
	virtual void Enable();
	virtual void EnableAll();
	virtual void AncestorsDisabled();
	virtual void AncestorsEnabled();
	virtual void EnableEvents(std::uint32_t);
	virtual void DisableEvents(std::uint32_t);
	virtual GameObject* Create();
	virtual Bool TickWhileInactive(float);
	virtual Bool Tick(float);
	virtual Bool SimulationTick(float);
	virtual Bool PrepareGeometry();
	virtual Bool RenderPre3D();
	virtual Bool Render3D(int userData);
	virtual Bool RenderPost3D();
	virtual Bool Pause(Bool);
	virtual Bool LostFocus();
	virtual Bool Restore();
	virtual Bool EventHandler(std::int32_t);
	virtual Bool KeyUpHandler(ProcessNode*, KeyInfo*);
	virtual Bool KeyDownHandler(ProcessNode*, KeyInfo*);
	virtual Bool PacketHandler(std::uint32_t, void*, std::uint32_t, std::uint32_t, std::uint32_t);
	virtual void Kill();
	virtual void DumpHierarchy(char*, DumpType, std::int32_t);

	GameObject(bool startEnabled);
	void UpPropogateMasks();
private:
	GameObject* prev;
	GameObject* next;
	GameObject* firstChild;
	GameObject* parent;
	std::uint32_t wantedEventMask;
	std::uint32_t descendantsWantedEventMask;
	std::uint8_t typeID;
	GameObjectFlags flags;
};

