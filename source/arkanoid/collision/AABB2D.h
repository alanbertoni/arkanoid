#pragma once

#include "../utilities/Math.h"

namespace collision
{
	class AABB2D
	{
	public:
		AABB2D() = default;
		AABB2D(const Vec2& vHalfExtents, const Vec2& vCenter = Vec2{});
		~AABB2D() = default;

		AABB2D(const AABB2D&) = default;
		AABB2D& operator=(const AABB2D&) = default;

		AABB2D(AABB2D&&) = default;
		AABB2D& operator=(AABB2D&&) = default;

		bool IsValid() const;

		Vec2& GetCenter();
		const Vec2& GetCenter() const;
		Vec2& GetHalfExtents();
		const Vec2& GetHalfExtents() const;
		Vec2 GetMin() const;		
		Vec2 GetMax() const;

		void SetCenter(const Vec2& vCenter);
		void SetHalfExtents(const Vec2& vHalfExtents);
		
		bool Overlaps(const AABB2D& oAABB) const;
		Vec2 IntersectionDim(const AABB2D& oAABB) const;

	private:
		Vec2 m_vCenter{ 0.0f, 0.0f };
		Vec2 m_vHalfExtents{ 0.0f, 0.0f };
	};

	inline AABB2D::AABB2D(const Vec2& vHalfExtents, const Vec2& vCenter)
		: m_vHalfExtents{ vHalfExtents }
		, m_vCenter{ vCenter }
	{
	}

	inline bool AABB2D::IsValid() const
	{
		return m_vHalfExtents.x > 0.0f && m_vHalfExtents.y > 0.0f;
	}

	inline Vec2& AABB2D::GetCenter()
	{
		return m_vCenter;
	}

	inline const Vec2& AABB2D::GetCenter() const
	{
		return m_vCenter;
	}

	inline Vec2& AABB2D::GetHalfExtents()
	{
		return m_vHalfExtents;
	}

	inline const Vec2& AABB2D::GetHalfExtents() const
	{
		return m_vHalfExtents;
	}

	inline Vec2 AABB2D::GetMin() const
	{
		return m_vCenter - m_vHalfExtents;
	}

	inline Vec2 AABB2D::GetMax() const
	{
		return m_vCenter + m_vHalfExtents;
	}

	inline void AABB2D::SetCenter(const Vec2& vCenter)
	{
		m_vCenter = vCenter;
	}

	inline void AABB2D::SetHalfExtents(const Vec2& vHalfExtents)
	{
		m_vHalfExtents = vHalfExtents;
	}

	inline bool AABB2D::Overlaps(const AABB2D& oAABB) const
	{
		Vec2 l_vD1, l_vD2;
		l_vD1 = oAABB.GetMin() - GetMax();
		l_vD2 = GetMin() - oAABB.GetMax();

		if (l_vD1.x > 0.0f || l_vD1.y > 0.0f)
			return false;

		if (l_vD2.x > 0.0f || l_vD2.y > 0.0f)
			return false;

		return true;
	}

	inline Vec2 AABB2D::IntersectionDim(const AABB2D& oAABB) const
	{
		Vec2 l_vMin = GetMin();
		Vec2 l_vOtherMin = oAABB.GetMin();
		const float l_fMinY = l_vMin.y > l_vOtherMin.y ? l_vMin.y : l_vOtherMin.y;
		const float l_fMinX = l_vMin.x > l_vOtherMin.x ? l_vMin.x : l_vOtherMin.x;

		Vec2 l_vMax = GetMax();
		Vec2 l_vOtherMax = oAABB.GetMax();
		const float l_fMaxY = l_vMax.y < l_vOtherMax.y ? l_vMax.y : l_vOtherMax.y;
		const float l_fMaxX = l_vMax.x < l_vOtherMax.x ? l_vMax.x : l_vOtherMax.x;

		return Vec2{ l_fMaxX - l_fMinX, l_fMaxY - l_fMinY };
	}
}