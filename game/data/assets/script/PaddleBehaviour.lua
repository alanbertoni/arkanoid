require "math";
require "string";

local PaddleActor = GetActorFromName("paddle");
local ScriptComp = PaddleActor:GetComponent("ScriptComponent");
local RendComp = PaddleActor:GetComponent("RendererComponent");
local TransfComp = PaddleActor:GetComponent("TransformComponent");
local AudioComp = PaddleActor:GetComponent("AudioSourceComponent");

ScriptComp:Setup({ xVel = 1.2, initialPos = TransfComp:GetPosition() })

function ScriptComp:OnEnter()
	TransfComp:SetPosition(self.initialPos);
end 

function ScriptComp:OnUpdate(deltaTime)
	local pos = TransfComp:GetPosition();
	local moved = false;
	if(IsKey('A')) then
		pos.x = pos.x - deltaTime * self.xVel;
		moved = true;
	elseif(IsKey('D')) then
		pos.x = pos.x + deltaTime * self.xVel;
		moved = true;
	end

	if(pos.x > 0.82) then
		pos.x = 0.82;
	elseif(pos.x < -0.82) then
		pos.x = -0.82;
	end
	
	if(moved == true) then
		TransfComp:SetPosition(pos);
	end
end

function ScriptComp:OnCollision(collisionData)
	if(collisionData.ActorType2 == "BALL") then
		AudioComp:Play(0.1, false);
	end
end