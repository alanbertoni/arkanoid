require "math";
require "string";

local BallActor = GetLastActorFromType("BALL");
local ScriptComp = BallActor:GetComponent("ScriptComponent");
local TransfComp = BallActor:GetComponent("TransformComponent");
local AudioComp = BallActor:GetComponent("AudioSourceComponent");

math.randomseed(os.time());
ScriptComp:Setup({ xVel = 1, yVel = 1, initialYVel = 1.0, initialPos = TransfComp:GetPosition() });

function ScriptComp:OnEnter()
	TransfComp:SetPosition(self.initialPos);
	self.xVel = math.random(75, 100) / 100;
	self.yVel = self.initialYVel;
	self.Ready = false;
end 

function ScriptComp:OnUpdate(deltaTime)
	if(self.Ready) then
		local pos = TransfComp:GetPosition();
		pos.x = pos.x + deltaTime * self.xVel;
		pos.y = pos.y + deltaTime * self.yVel;
		
		if(pos.y < -1.10) then
			SendEvent("ResetGame", "");
			return;
		end
		
		if(pos.x > 0.90) then
			pos.x = 0.90;
			self.xVel = self.xVel * -1;
		elseif(pos.x < -0.90) then
			pos.x = -0.90;
			self.xVel = self.xVel * -1;
		end
	
		if(pos.y > 0.90) then
			pos.y = 0.90;
			self.yVel = self.yVel * -1;
		end
		
		TransfComp:SetPosition(pos);
		return;
	end	

	if(IsKeyUp(' ')) then
		self.Ready = true;
	end
	
end

function ScriptComp:OnCollision(collisionData)
	if(collisionData.DirectionY == "BOTTOM" or collisionData.DirectionY == "TOP") then
		self.yVel = self.yVel * -1;
	end
	
	if(collisionData.ActorType2 == "BRICK") then
		AudioComp:Play(0.1, false);
		if(collisionData.DirectionX == "LEFT" or collisionData.DirectionX == "RIGHT") then
			self.xVel = self.xVel * -1;
		end
	elseif(collisionData.ActorType2 == "PADDLE") then
		local paddleToBallX = collisionData.Pos1.x - collisionData.Pos2.x;
		local paddleToBallY = collisionData.Pos1.y - collisionData.Pos2.y;
		local ballVelMag = math.sqrt(paddleToBallX * paddleToBallX + paddleToBallY * paddleToBallY);
		local ballVelY = paddleToBallY / ballVelMag;
		self.xVel = math.acos(ballVelY) * (paddleToBallX / math.abs(paddleToBallX));
	end
end