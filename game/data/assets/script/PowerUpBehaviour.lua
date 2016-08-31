require "math";
require "string";

local PowerUpActor = GetLastActorFromType("POWERUP");
local ScriptComp = PowerUpActor:GetComponent("ScriptComponent");
local TransfComp = PowerUpActor:GetComponent("TransformComponent");
local AudioComp = PowerUpActor:GetComponent("AudioSourceComponent");

ScriptComp:Setup({ yVel = 1, initialYVel = 0.5});

function ScriptComp:OnEnter()
	self.yVel = self.initialYVel;
end 

function ScriptComp:OnUpdate(deltaTime)
	TransfComp:Translate(0, -deltaTime * self.yVel,0);
	local pos = TransfComp:GetPosition();
	
	if(pos.y < -1.50) then
		PowerUpActor:SetEnable(false);
	end
end

function ScriptComp:OnCollision(collisionData)
	if(collisionData.ActorType2 == "PADDLE") then
		PowerUpActor:SetEnable(false);
		AudioComp:Play(0.1, false);
	end
end