require "math";
require "string";

local PowerUpActor = GetLastActorFromType("POWERUP");
local PowerUpTComp = PowerUpActor:GetComponent("TransformComponent");
local BrickActor = GetLastActorFromType("BRICK");
local ScriptComp = BrickActor:GetComponent("ScriptComponent");
local RendComp = BrickActor:GetComponent("RendererComponent");
local TransfComp = BrickActor:GetComponent("TransformComponent");
local ColliderComp = BrickActor:GetComponent("ColliderComponent");
local AudioComp = BrickActor:GetComponent("AudioSourceComponent");

ScriptComp:Setup({ spawnPowerUp = 1, powerUpThreshold = 0.3, initialHitPoints = math.random(0,3), dead = false, dieTime = 0, totalDieTime = 2, animationdead = 1, animationThreshold = 0.4, degreesPerSec = 720, initialScaleFactor = TransfComp:GetScale() })

function ScriptComp:OnEnter()
	-- Reset state of BrickActor
	self.hitPoints = self.initialHitPoints;
	ColliderComp:SetEnable(true);
	self.dead = false;
	self.dieTime = 0;
	self.animationdead = 1;
	TransfComp:SetScale(self.initialScaleFactor.x);
	TransfComp:SetRotation({x=0,y=0,z=1}, 0);
end 

function ScriptComp:OnUpdate(deltaTime)
	if(self.dead == true ) then
		-- Execute the dead animation
		if(self.animationdead < self.animationThreshold) then
			self.dieTime = self.dieTime + deltaTime;
			local scaleFactor = self.initialScaleFactor.x - (self.initialScaleFactor.x / self.totalDieTime) * self.dieTime;
			if(scaleFactor < 0) then
				scaleFactor = 0;
			end
			TransfComp:SetScale(scaleFactor);
			TransfComp:Rotate({x=0,y=0,z=1}, self.degreesPerSec * deltaTime);
			-- Reset state after totalDieTime
			if(self.dieTime > self.totalDieTime) then
				self.dead = false;
				self.dieTime = 0;
				TransfComp:Translate(10,10,0);
			end
		else
			self.dead = false;
			self.dieTime = 0;
			-- Spawn PowerUp only if it is currently disabled with a probability of powerUpThreshold
			if(self.spawnPowerUp < self.powerUpThreshold and (not PowerUpActor:IsEnable())) then
				PowerUpActor:SetEnable(true);
				PowerUpTComp:SetPosition(TransfComp:GetPosition());
			end
			-- Play explosion sound and translate out of view frustum
			AudioComp:Play(0.1, false);
			TransfComp:Translate(10,10,0);
		end
	end
end

function ScriptComp:OnCollision(collisionData)
	-- Decrease hitPoints, if dead disable collider and start the dead animation
	self.hitPoints = self.hitPoints - 1;
	if(self.hitPoints < 1) then
		ColliderComp:SetEnable(false);
		self.dead = true;
		self.animationdead = math.random();
		self.spawnPowerUp = math.random();
	end
end
