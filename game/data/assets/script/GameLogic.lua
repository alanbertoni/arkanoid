require "math";
require "string";

math.randomseed( os.time() );
local totalBrickCount = 55;
local brickTypes = 5;

GameLogicObject = GetGameLogic();

function GameLogicObject:OnInit()
	-- Create all the actors of the game;
	CreateActorWithoutParent("PADDLE", "paddle");
	CreateActorWithoutParent("BALL", "ball");
	CreateActorWithoutParent("CAMERA", "mainCamera");
	CreateActorWithoutParent("POWERUP", "powerup");
	for i=1, totalBrickCount, 1
	do
		local name = string.format("brick%03d", i);
		local actor = CreateActorWithoutParent("BRICK", name);
	end
	local Background = CreateActorWithoutParent("BACKGROUND", "background");
	local BackgroundAudio = Background:GetComponent("AudioSourceComponent");
	BackgroundAudio:Play(0.05, true);
end

function GameLogicObject:OnReset()
	local PowerUpActor = GetLastActorFromType("POWERUP");
	PowerUpActor:SetEnable(false);
	
	GameLogicObject:SetupLevel();
end

function GameLogicObject:OnRunning(deltaTime)
	if(IsKeyUp('N')) then
		SendEvent("ResetGame", "");
	end
end

function GameLogicObject:SetupLevel()
	local brick = 0;
	if(math.random() < 0.5) then
		brick = GameLogicObject:HourglassLevel();
	else
		brick = GameLogicObject:SimpleLevel();
	end
	
	GameLogicObject:TranslateUnusedBricks(brick);
end

function GameLogicObject:TranslateUnusedBricks(brick)
	-- Translate the remaining bricks outside of the view frustum 
	for i=brick, totalBrickCount, 1
	do
		local name = string.format("brick%03d", i);
		local actor = GetActorFromName(name);
		local Transf = actor:GetComponent("TransformComponent");
		Transf:Translate(10,10,0);
	end
end

function GameLogicObject:SimpleLevel()
	local brick = 1;
	local startPosY = 0.7;
	local offsetY = 0.10;
	for j=0, 4, 1
	do
		local yPos = startPosY - offsetY * j;
		local startPosX = -0.75;
		local offsetX = 0.15;
		for i=0, 10, 1
		do
			local xPos = startPosX + offsetX * i;
			GameLogicObject:ResetBrickPosition(brick, xPos, yPos, j);
			brick = brick + 1;
		end
	end
	return brick;
end

function GameLogicObject:HourglassLevel()
	local brick = 1;
	local startPosY = 0.7;
	local offsetY = 0.10;
	for j=0, 4, 1
	do
		local yPos = startPosY - offsetY * j;
		local startPosX = -0.60;
		local offsetX = 0.15;
		for i=j, 8-j, 1
		do
			local xPos = startPosX + offsetX * i;
			GameLogicObject:ResetBrickPosition(brick, xPos, yPos, j);
			brick = brick + 1;
		end
	end
	
	startPosY = startPosY - offsetY * 5;
	for j=0, 4, 1
	do
		local yPos = startPosY - offsetY * j;
		local offsetX = 0.15;
		local startPosX = 0.00 - offsetX * j;
		for i=0, 2 * j, 1
		do
			local xPos = startPosX + offsetX * i;
			GameLogicObject:ResetBrickPosition(brick, xPos, yPos, 4 - j);
			brick = brick + 1;
		end
	end
	
	return brick;
end

function GameLogicObject:ResetBrickPosition(brick, xPos, yPos, color)
	local name = string.format("brick%03d", brick);
	local actor = GetActorFromName(name);
	local Transf = actor:GetComponent("TransformComponent");
	local RendComp = actor:GetComponent("RendererComponent");
	local pos = Transf:GetPosition();
	Transf:SetPosition({x=xPos, y=yPos, z=pos.z});
	
	if(color % brickTypes == 0) then
		RendComp:SetDiffuseColor({r=255,g=0,b=0});
	elseif(color % brickTypes == 1) then
		RendComp:SetDiffuseColor({r=255,g=255,b=0});
	elseif(color % brickTypes == 2) then
		RendComp:SetDiffuseColor({r=0,g=255,b=0});
	elseif(color % brickTypes == 3) then
		RendComp:SetDiffuseColor({r=0,g=255,b=255});
	elseif(color % brickTypes == 4) then
		RendComp:SetDiffuseColor({r=0,g=0,b=255});
	end
end
