#pragma once

#include <map>
#include <string>
#include <vector>
#include <list>

typedef size_t ActorId;
typedef std::string ActorType;

const ActorId INVALID_ACTOR_ID = 0U;

typedef size_t ComponentId;
typedef size_t ComponentTypeId;

typedef size_t MeshId;
typedef size_t ShaderId;
typedef size_t InputLayoutId;
typedef size_t CBufferId;
typedef size_t TextureId;

typedef size_t AudioId;

enum ERenderPass
{
	RenderPass_Static,
	RenderPass_Actor,
	RenderPass_Background,
	RenderPass_NotRendered,
	RenderPass_Last
};

struct Dimension
{
	float m_fWidth;
	float m_fHeight;
};
