#pragma once

#include <cmath>
#include <D3D11.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>

using namespace DirectX;
using namespace DirectX::PackedVector;

typedef XMFLOAT4X4 Mat4x4;
typedef XMMATRIX Matrix;

typedef XMFLOAT4 Quat;
typedef XMFLOAT4 Vec4;
typedef XMFLOAT3 Vec3;
typedef XMFLOAT2 Vec2;

typedef XMCOLOR Color;
typedef XMFLOAT4 Color4F;

namespace
{
	inline Vec2 operator+(const Vec2& oV1, const Vec2& oV2)
	{
		return Vec2{ oV1.x + oV2.x, oV1.y + oV2.y };
	}

	inline Vec2 operator*(const Vec2& oV, float fScalar)
	{
		return Vec2{ oV.x * fScalar, oV.y * fScalar };
	}

	inline Vec2 operator-(const Vec2& oV1, const Vec2& oV2)
	{
		return Vec2{ oV1.x - oV2.x, oV1.y - oV2.y };
	}

	inline Vec3 operator+(const Vec3& oV1, const Vec3& oV2)
	{
		return Vec3{ oV1.x + oV2.x, oV1.y + oV2.y, oV1.z + oV2.z };
	}

	inline Vec3 operator*(const Vec3& oV, float fScalar)
	{
		return Vec3{ oV.x * fScalar, oV.y * fScalar, oV.z * fScalar };
	}

	inline Vec3 operator-(const Vec3& oV1, const Vec3& oV2)
	{
		return Vec3{ oV1.x - oV2.x, oV1.y - oV2.y, oV1.z - oV2.z };
	}
}

namespace utilities
{
	class MathHelper
	{
	public:
		static void LoadColor(Color4F& oDestColor, const Color& oSourceColor)
		{
			return XMStoreFloat4(&oDestColor, XMLoadColor(&oSourceColor));
		}

		static Matrix LoadMatrix(const Mat4x4& oMat)
		{
			return XMLoadFloat4x4(&oMat);
		}

		static void StoreMatrix(Mat4x4& oDestMat, const Matrix& oSourceMatrix)
		{
			return XMStoreFloat4x4(&oDestMat, oSourceMatrix);
		}

		static void SetIdentityMatrix(Mat4x4& oMatrix)
		{
			XMMATRIX l_oIdentity = XMMatrixIdentity();
			XMStoreFloat4x4(&oMatrix, l_oIdentity);
		}

		static Matrix GetIdentityMatrix()
		{
			return XMMatrixIdentity();
		}

		static void SetPerspectiveProjectionMatrix(Mat4x4& oMatrix, float fFovH, float fAspectRatio, float fNearPlane, float fFarPlane)
		{
			XMStoreFloat4x4(&oMatrix, XMMatrixPerspectiveFovLH(fFovH, fAspectRatio, fNearPlane, fFarPlane));
		}

		static void SetOrthographicProjectionMatrix(Mat4x4& oMatrix, float fViewWidth, float fViewHeight, float fNearPlane, float fFarPlane)
		{
			XMStoreFloat4x4(&oMatrix, XMMatrixOrthographicLH(fViewWidth, fViewHeight, fNearPlane, fFarPlane));
		}

		static void SetAffineTransformationMatrix(Matrix& oMatrix, const Vec3& oPos, const Quat& oQuat, const Vec3& oScale)
		{
			XMVECTOR S = XMLoadFloat3(&oScale);
			XMVECTOR P = XMLoadFloat3(&oPos);
			XMVECTOR Q = XMLoadFloat4(&oQuat);

			XMVECTOR zero = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
			oMatrix = XMMatrixAffineTransformation(S, zero, Q, P);
		}

		static void SetAffineTransformationMatrix(Mat4x4& oMatrix, const Vec3& oPos, const Quat& oQuat, const Vec3& oScale)
		{
			XMVECTOR S = XMLoadFloat3(&oScale);
			XMVECTOR P = XMLoadFloat3(&oPos);
			XMVECTOR Q = XMLoadFloat4(&oQuat);

			XMVECTOR zero = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
			XMStoreFloat4x4(&oMatrix, XMMatrixAffineTransformation(S, zero, Q, P));
		}

		static void MatrixMultiply(Mat4x4& oMat, const Mat4x4& oMat1, const Mat4x4& oMat2)
		{
			Matrix l_oM1 = LoadMatrix(oMat1);
			Matrix l_oM2 = LoadMatrix(oMat2);
			StoreMatrix(oMat, XMMatrixMultiply(l_oM1, l_oM2));
		}

		static void MatrixTranspose(Mat4x4& oDestMatrix, const Mat4x4& oSourceMatrix)
		{
			XMMATRIX l_oM = XMLoadFloat4x4(&oSourceMatrix);
			XMStoreFloat4x4(&oDestMatrix, XMMatrixTranspose(l_oM));
		}

		static Matrix MatrixTranspose(const Matrix& oMatrix)
		{
			return XMMatrixTranspose(oMatrix);
		}

		static Matrix MatrixInverse(const Matrix& oMatrix)
		{
			XMVECTOR l_oDet = XMMatrixDeterminant(oMatrix);
			return XMMatrixInverse(&l_oDet, oMatrix);
		}

		static void MatrixInverse(Mat4x4& oDestMatrix, const Mat4x4& oSourceMatrix)
		{
			XMMATRIX l_oM = XMLoadFloat4x4(&oSourceMatrix);
			XMVECTOR l_oDet = XMMatrixDeterminant(l_oM);
			XMStoreFloat4x4(&oDestMatrix, XMMatrixInverse(&l_oDet, l_oM));
		}

		static void SetQuaternionFromYawPitchRoll(Quat& oQuat, float fPitch, float fYaw, float fRoll)
		{
			XMVECTOR l_oQuat = XMQuaternionRotationRollPitchYaw(fPitch, fYaw, fRoll);
			XMStoreFloat4(&oQuat, l_oQuat);
		}

		static void SetQuaternionFromAxisAnge(Quat& oQuat, const Vec3& oAxis, float fDegrees)
		{
			XMVECTOR l_oQuat = XMQuaternionRotationAxis(XMLoadFloat3(&oAxis), ConvertFromDegreesToRadians(fDegrees));
			XMStoreFloat4(&oQuat, l_oQuat);
		}

		static void MultiplyQuaternion(Quat& oQuat, const Quat& oQ1, const Quat& oQ2)
		{
			XMVECTOR l_oQuat = XMQuaternionMultiply(XMLoadFloat4(&oQ1), XMLoadFloat4(&oQ2));
			XMStoreFloat4(&oQuat, XMQuaternionNormalize(l_oQuat));
		}

		static float ConvertFromDegreesToRadians(float fDegrees)
		{
			return XMConvertToRadians(fDegrees);
		}
	};
}
