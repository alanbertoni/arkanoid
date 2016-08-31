#pragma once

#include <assert.h>

/*
	http://scottbilas.com/publications/gem-singleton/
*/

namespace core
{
	template <typename T> class Singleton
	{
		static T* s_poSingleton;

	public:
		Singleton()
		{
			assert(!s_poSingleton);
			s_poSingleton = static_cast <T*> (this);
		}

		~Singleton()
		{
			assert(s_poSingleton);
			s_poSingleton = 0;
		}

		static T& GetSingleton()
		{
			assert(s_poSingleton);
			return *s_poSingleton;
		}

		static T* GetSingletonPtr()
		{
			return s_poSingleton;
		}

		static void Release()
		{
			assert(s_poSingleton);
			delete(s_poSingleton);
			s_poSingleton = NULL;
		}

	};

	template <typename T> T* Singleton <T>::s_poSingleton = 0;
}
