#pragma once

#include "../utilities/Math.h"
#include "LuaManager.h"

#include <LuaPlus.h>

namespace script
{
	namespace LuaUtilities
	{
		inline void ConvertVec3ToLuaObject(const Vec3& vec, LuaPlus::LuaObject& outLuaObject)
		{
			outLuaObject.AssignNewTable(script::LuaManager::GetSingleton().GetLuaState());
			outLuaObject.SetNumber("x", vec.x);
			outLuaObject.SetNumber("y", vec.y);
			outLuaObject.SetNumber("z", vec.z);
		};

		inline void ConvertVec2ToLuaObject(const Vec2& vec, LuaPlus::LuaObject& outLuaObject)
		{
			outLuaObject.AssignNewTable(script::LuaManager::GetSingleton().GetLuaState());
			outLuaObject.SetNumber("x", vec.x);
			outLuaObject.SetNumber("y", vec.y);
		};


		inline void ConvertLuaObjectToVec3(const LuaPlus::LuaObject& i_oLuaObject, Vec3& outVec3)
		{
			LuaPlus::LuaObject temp;

			temp = i_oLuaObject.Get("x");
			if (temp.IsNumber())
				outVec3.x = temp.GetFloat();
			else
				outVec3.x = 0.0f;

			temp = i_oLuaObject.Get("y");
			if (temp.IsNumber())
				outVec3.y = temp.GetFloat();
			else
				outVec3.y = 0.0f;

			temp = i_oLuaObject.Get("z");
			if (temp.IsNumber())
				outVec3.z = temp.GetFloat();
			else
				outVec3.z = 0.0f;
		};

		inline void ConvertLuaObjectToColor(const LuaPlus::LuaObject& i_oLuaObject, Color& outColor)
		{
			LuaPlus::LuaObject temp;

			temp = i_oLuaObject.Get("r");
			if (temp.IsNumber())
				outColor.r = temp.GetInteger();
			else
				outColor.r = 255;

			temp = i_oLuaObject.Get("g");
			if (temp.IsNumber())
				outColor.g = temp.GetInteger();
			else
				outColor.g = 255;

			temp = i_oLuaObject.Get("b");
			if (temp.IsNumber())
				outColor.b = temp.GetInteger();
			else
				outColor.b = 255;

			temp = i_oLuaObject.Get("a");
			if (temp.IsNumber())
				outColor.a = temp.GetInteger();
			else
				outColor.a = 255;
		};
	}
}
