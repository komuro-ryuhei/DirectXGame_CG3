#pragma once
#include "Matrix.h"
#define _USE_MATH_DEFINES
#include <cmath>

/// <summary>
/// 2次元ベクトル
/// </summary>
struct Vector2 final {
	float x;
	float y;
};

/// <summary>
/// 3次元ベクトル
/// </summary>
struct Vector3 final {
	float x;
	float y;
	float z;
};

/// <summary>
/// 4次元ベクトル
/// </summary>
struct Vector4 final {
	float x;
	float y;
	float z;
	float w;
};

// 加算(Matrix4x4)
Matrix4x4 Add(const Matrix4x4& m1, const Matrix4x4& m2) {
	Matrix4x4 result;

	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			result.m[i][j] = m1.m[i][j] + m2.m[i][j];
		}
	}
	return result;
}

// 減算(Matrix4x4)
Matrix4x4 Subtract(const Matrix4x4& m1, const Matrix4x4& m2) {
	Matrix4x4 result;

	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			result.m[i][j] = m1.m[i][j] - m2.m[i][j];
		}
	}
	return result;
}

// 積(Matrix4x4)
Matrix4x4 Multiply(const Matrix4x4& m1, const Matrix4x4& m2) {
	Matrix4x4 result = {};

	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			for (int k = 0; k < 4; k++) {
				result.m[i][j] += m1.m[i][k] * m2.m[k][j];
			}
		}
	}
	return result;
}

// 単位行列作成
Matrix4x4 MakeIdentity4x4() {
	Matrix4x4 identity;

	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			identity.m[i][j] = (i == j) ? 1.0f : 0.0f;
		}
	}
	return identity;
}

// 逆行列
Matrix4x4 Inverse4x4(Matrix4x4& matrix) {
	Matrix4x4 result = {};

	float a = matrix.m[0][0];
	float b = matrix.m[0][1];
	float c = matrix.m[0][2];
	float d = matrix.m[0][3];
	float e = matrix.m[1][0];
	float f = matrix.m[1][1];
	float g = matrix.m[1][2];
	float h = matrix.m[1][3];
	float i = matrix.m[2][0];
	float j = matrix.m[2][1];
	float k = matrix.m[2][2];
	float l = matrix.m[2][3];
	float m = matrix.m[3][0];
	float n = matrix.m[3][1];
	float o = matrix.m[3][2];
	float p = matrix.m[3][3];

	float det = a * (f * (k * p - l * o) - g * (j * p - l * n) + h * (j * o - k * n)) -
		b * (e * (k * p - l * o) - g * (i * p - l * m) + h * (i * o - k * m)) +
		c * (e * (j * p - l * n) - f * (i * p - l * m) + h * (i * n - j * m)) -
		d * (e * (j * o - k * n) - f * (i * o - k * m) + g * (i * n - j * m));

	if (det == 0.0f) {
		return result;
	}

	float invDet = 1.0f / det;

	result.m[0][0] = invDet * (matrix.m[1][1] * (matrix.m[2][2] * matrix.m[3][3] - matrix.m[2][3] * matrix.m[3][2]) -
		matrix.m[1][2] * (matrix.m[2][1] * matrix.m[3][3] - matrix.m[2][3] * matrix.m[3][1]) +
		matrix.m[1][3] * (matrix.m[2][1] * matrix.m[3][2] - matrix.m[2][2] * matrix.m[3][1]));

	result.m[0][1] = invDet * (-(matrix.m[0][1] * (matrix.m[2][2] * matrix.m[3][3] - matrix.m[2][3] * matrix.m[3][2]) -
		matrix.m[0][2] * (matrix.m[2][1] * matrix.m[3][3] - matrix.m[2][3] * matrix.m[3][1]) +
		matrix.m[0][3] * (matrix.m[2][1] * matrix.m[3][2] - matrix.m[2][2] * matrix.m[3][1])));

	result.m[0][2] = invDet * (matrix.m[0][1] * (matrix.m[1][2] * matrix.m[3][3] - matrix.m[1][3] * matrix.m[3][2]) -
		matrix.m[0][2] * (matrix.m[1][1] * matrix.m[3][3] - matrix.m[1][3] * matrix.m[3][1]) +
		matrix.m[0][3] * (matrix.m[1][1] * matrix.m[3][2] - matrix.m[1][2] * matrix.m[3][1]));

	result.m[0][3] = invDet * (-(matrix.m[0][1] * (matrix.m[1][2] * matrix.m[2][3] - matrix.m[1][3] * matrix.m[2][2]) -
		matrix.m[0][2] * (matrix.m[1][1] * matrix.m[2][3] - matrix.m[1][3] * matrix.m[2][1]) +
		matrix.m[0][3] * (matrix.m[1][1] * matrix.m[2][2] - matrix.m[1][2] * matrix.m[2][1])));

	result.m[1][0] = invDet * (-(matrix.m[1][0] * (matrix.m[2][2] * matrix.m[3][3] - matrix.m[2][3] * matrix.m[3][2]) -
		matrix.m[1][2] * (matrix.m[2][0] * matrix.m[3][3] - matrix.m[2][3] * matrix.m[3][0]) +
		matrix.m[1][3] * (matrix.m[2][0] * matrix.m[3][2] - matrix.m[2][2] * matrix.m[3][0])));

	result.m[1][1] = invDet * (matrix.m[0][0] * (matrix.m[2][2] * matrix.m[3][3] - matrix.m[2][3] * matrix.m[3][2]) -
		matrix.m[0][2] * (matrix.m[2][0] * matrix.m[3][3] - matrix.m[2][3] * matrix.m[3][0]) +
		matrix.m[0][3] * (matrix.m[2][0] * matrix.m[3][2] - matrix.m[2][2] * matrix.m[3][0]));

	result.m[1][2] = invDet * (-(matrix.m[0][0] * (matrix.m[1][2] * matrix.m[3][3] - matrix.m[1][3] * matrix.m[3][2]) -
		matrix.m[0][2] * (matrix.m[1][0] * matrix.m[3][3] - matrix.m[1][3] * matrix.m[3][0]) +
		matrix.m[0][3] * (matrix.m[1][0] * matrix.m[3][2] - matrix.m[1][2] * matrix.m[3][0])));

	result.m[1][3] = invDet * (matrix.m[0][0] * (matrix.m[1][2] * matrix.m[2][3] - matrix.m[1][3] * matrix.m[2][2]) -
		matrix.m[0][2] * (matrix.m[1][0] * matrix.m[2][3] - matrix.m[1][3] * matrix.m[2][0]) +
		matrix.m[0][3] * (matrix.m[1][0] * matrix.m[2][2] - matrix.m[1][2] * matrix.m[2][0]));

	result.m[2][0] = invDet * (matrix.m[1][0] * (matrix.m[2][1] * matrix.m[3][3] - matrix.m[2][3] * matrix.m[3][1]) -
		matrix.m[1][1] * (matrix.m[2][0] * matrix.m[3][3] - matrix.m[2][3] * matrix.m[3][0]) +
		matrix.m[1][3] * (matrix.m[2][0] * matrix.m[3][1] - matrix.m[2][1] * matrix.m[3][0]));

	result.m[2][1] = invDet * (-(matrix.m[0][0] * (matrix.m[2][1] * matrix.m[3][3] - matrix.m[2][3] * matrix.m[3][1]) -
		matrix.m[0][1] * (matrix.m[2][0] * matrix.m[3][3] - matrix.m[2][3] * matrix.m[3][0]) +
		matrix.m[0][3] * (matrix.m[2][0] * matrix.m[3][1] - matrix.m[2][1] * matrix.m[3][0])));

	result.m[2][2] = invDet * (matrix.m[0][0] * (matrix.m[1][1] * matrix.m[3][3] - matrix.m[1][3] * matrix.m[3][1]) -
		matrix.m[0][1] * (matrix.m[1][0] * matrix.m[3][3] - matrix.m[1][3] * matrix.m[3][0]) +
		matrix.m[0][3] * (matrix.m[1][0] * matrix.m[3][1] - matrix.m[1][1] * matrix.m[3][0]));

	result.m[2][3] = invDet * (-(matrix.m[0][0] * (matrix.m[1][1] * matrix.m[2][3] - matrix.m[1][3] * matrix.m[2][1]) -
		matrix.m[0][1] * (matrix.m[1][0] * matrix.m[2][3] - matrix.m[1][3] * matrix.m[2][0]) +
		matrix.m[0][3] * (matrix.m[1][0] * matrix.m[2][1] - matrix.m[1][1] * matrix.m[2][0])));

	result.m[3][0] = invDet * (-(matrix.m[1][0] * (matrix.m[2][1] * matrix.m[3][2] - matrix.m[2][2] * matrix.m[3][1]) -
		matrix.m[1][1] * (matrix.m[2][0] * matrix.m[3][2] - matrix.m[2][2] * matrix.m[3][0]) +
		matrix.m[1][2] * (matrix.m[2][0] * matrix.m[3][1] - matrix.m[2][1] * matrix.m[3][0])));

	result.m[3][1] = invDet * (matrix.m[0][0] * (matrix.m[2][1] * matrix.m[3][2] - matrix.m[2][2] * matrix.m[3][1]) -
		matrix.m[0][1] * (matrix.m[2][0] * matrix.m[3][2] - matrix.m[2][2] * matrix.m[3][0]) +
		matrix.m[0][2] * (matrix.m[2][0] * matrix.m[3][1] - matrix.m[2][1] * matrix.m[3][0]));

	result.m[3][2] = invDet * (-(matrix.m[0][0] * (matrix.m[1][1] * matrix.m[3][2] - matrix.m[1][2] * matrix.m[3][1]) -
		matrix.m[0][1] * (matrix.m[1][0] * matrix.m[3][2] - matrix.m[1][2] * matrix.m[3][0]) +
		matrix.m[0][2] * (matrix.m[1][0] * matrix.m[3][1] - matrix.m[1][1] * matrix.m[3][0])));

	result.m[3][3] = invDet * (matrix.m[0][0] * (matrix.m[1][1] * matrix.m[2][2] - matrix.m[1][2] * matrix.m[2][1]) -
		matrix.m[0][1] * (matrix.m[1][0] * matrix.m[2][2] - matrix.m[1][2] * matrix.m[2][0]) +
		matrix.m[0][2] * (matrix.m[1][0] * matrix.m[2][1] - matrix.m[1][1] * matrix.m[2][0]));

	return result;
}

// 平行移動行列
Matrix4x4 MakeTranslateMatrix(const Vector3& translate) {
	Matrix4x4 translateMatrix;

	translateMatrix.m[0][0] = 1.0f;
	translateMatrix.m[0][1] = 0.0f;
	translateMatrix.m[0][2] = 0.0f;
	translateMatrix.m[0][3] = 0.0f;

	translateMatrix.m[1][0] = 0.0f;
	translateMatrix.m[1][1] = 1.0f;
	translateMatrix.m[1][2] = 0.0f;
	translateMatrix.m[1][3] = 0.0f;

	translateMatrix.m[2][0] = 0.0f;
	translateMatrix.m[2][1] = 0.0f;
	translateMatrix.m[2][2] = 1.0f;
	translateMatrix.m[2][3] = 0.0f;

	translateMatrix.m[3][0] = translate.x;
	translateMatrix.m[3][1] = translate.y;
	translateMatrix.m[3][2] = translate.z;
	translateMatrix.m[3][3] = 1.0f;

	return translateMatrix;
}

// 拡大縮小行列
Matrix4x4 MakeScaleMatrix(const Vector3& scale) {
	Matrix4x4 scaleMatrix;

	scaleMatrix.m[0][0] = scale.x;
	scaleMatrix.m[0][1] = 0.0f;
	scaleMatrix.m[0][2] = 0.0f;
	scaleMatrix.m[0][3] = 0.0f;

	scaleMatrix.m[1][0] = 0.0f;
	scaleMatrix.m[1][1] = scale.y;
	scaleMatrix.m[1][2] = 0.0f;
	scaleMatrix.m[1][3] = 0.0f;

	scaleMatrix.m[2][0] = 0.0f;
	scaleMatrix.m[2][1] = 0.0f;
	scaleMatrix.m[2][2] = scale.z;
	scaleMatrix.m[2][3] = 0.0f;

	scaleMatrix.m[3][0] = 0.0f;
	scaleMatrix.m[3][1] = 0.0f;
	scaleMatrix.m[3][2] = 0.0f;
	scaleMatrix.m[3][3] = 1.0f;

	return scaleMatrix;
}

// 座標変換
Vector3 Transform(const Vector3& vector, const Matrix4x4& matrix) {
	Vector3 result;

	result.x = vector.x * matrix.m[0][0] + vector.y * matrix.m[1][0] + vector.z * matrix.m[2][0] + 1.0f * matrix.m[3][0];
	result.y = vector.x * matrix.m[0][1] + vector.y * matrix.m[1][1] + vector.z * matrix.m[2][1] + 1.0f * matrix.m[3][1];
	result.z = vector.x * matrix.m[0][2] + vector.y * matrix.m[1][2] + vector.z * matrix.m[2][2] + 1.0f * matrix.m[3][2];
	float w = vector.x * matrix.m[0][3] + vector.y * matrix.m[1][3] + vector.z * matrix.m[2][3] + 1.0f * matrix.m[3][3];
	assert(w != 0.0f);
	result.x /= w;
	result.y /= w;
	result.z /= w;

	return result;
}

Matrix4x4 MakeRotateXMatrix(float radian) {
	Matrix4x4 rotationMatrix = {};

	rotationMatrix.m[0][0] = 1.0f;
	rotationMatrix.m[0][1] = 0.0f;
	rotationMatrix.m[0][2] = 0.0f;
	rotationMatrix.m[0][3] = 0.0f;

	rotationMatrix.m[1][0] = 0.0f;
	rotationMatrix.m[1][1] = std::cos(radian);
	rotationMatrix.m[1][2] = std::sin(radian);
	rotationMatrix.m[1][3] = 0.0f;

	rotationMatrix.m[2][0] = 0.0f;
	rotationMatrix.m[2][1] = -std::sin(radian);
	rotationMatrix.m[2][2] = std::cos(radian);
	rotationMatrix.m[2][3] = 0.0f;

	rotationMatrix.m[3][0] = 0.0f;
	rotationMatrix.m[3][1] = 0.0f;
	rotationMatrix.m[3][2] = 0.0f;
	rotationMatrix.m[3][3] = 1.0f;

	return rotationMatrix;
}

Matrix4x4 MakeRotateYMatrix(float radian) {
	Matrix4x4 rotationMatrix = {};

	rotationMatrix.m[0][0] = std::cos(radian);
	rotationMatrix.m[0][1] = 0.0f;
	rotationMatrix.m[0][2] = -std::sin(radian);
	rotationMatrix.m[0][3] = 0.0f;

	rotationMatrix.m[1][0] = 0.0f;
	rotationMatrix.m[1][1] = 1.0f;
	rotationMatrix.m[1][2] = 0.0f;
	rotationMatrix.m[1][3] = 0.0f;

	rotationMatrix.m[2][0] = std::sin(radian);
	rotationMatrix.m[2][1] = 0.0f;
	rotationMatrix.m[2][2] = std::cos(radian);
	rotationMatrix.m[2][3] = 0.0f;

	rotationMatrix.m[3][0] = 0.0f;
	rotationMatrix.m[3][1] = 0.0f;
	rotationMatrix.m[3][2] = 0.0f;
	rotationMatrix.m[3][3] = 1.0f;

	return rotationMatrix;
}

Matrix4x4 MakeRotateZMatrix(float radian) {
	Matrix4x4 rotationMatrix = {};

	rotationMatrix.m[0][0] = std::cos(radian);
	rotationMatrix.m[0][1] = std::sin(radian);
	rotationMatrix.m[0][2] = 0.0f;
	rotationMatrix.m[0][3] = 0.0f;

	rotationMatrix.m[1][0] = -std::sin(radian);
	rotationMatrix.m[1][1] = std::cos(radian);
	rotationMatrix.m[1][2] = 0.0f;
	rotationMatrix.m[1][3] = 0.0f;

	rotationMatrix.m[2][0] = 0.0f;
	rotationMatrix.m[2][1] = 0.0f;
	rotationMatrix.m[2][2] = 1.0f;
	rotationMatrix.m[2][3] = 0.0f;

	rotationMatrix.m[3][0] = 0.0f;
	rotationMatrix.m[3][1] = 0.0f;
	rotationMatrix.m[3][2] = 0.0f;
	rotationMatrix.m[3][3] = 1.0f;

	return rotationMatrix;
}

// 3次元アフィン変換行列
Matrix4x4 MakeAffineMatrix(Vector3 scale, Vector3 rotate, Vector3 translate) {
	Matrix4x4 affineMatrix;

	// 各変換行列を作成
	Matrix4x4 scaleMatrix = MakeScaleMatrix(scale);

	Matrix4x4 rotateXMatrix = MakeRotateXMatrix(rotate.x);
	Matrix4x4 rotateYMatrix = MakeRotateYMatrix(rotate.y);
	Matrix4x4 rotateZMatrix = MakeRotateZMatrix(rotate.z);
	Matrix4x4 rotateXYZMatrix = Multiply(rotateXMatrix, Multiply(rotateYMatrix, rotateZMatrix));

	Matrix4x4 translateMatrix = MakeTranslateMatrix(translate);

	// 各変換行列を合成してアフィン変換行列を作成
	affineMatrix = Multiply(scaleMatrix, Multiply(rotateXYZMatrix, translateMatrix));

	return affineMatrix;
}

// 透視投影行列
Matrix4x4 MakePerspectiveFovMatrix(float fovY, float aspectRatio, float nearClip, float farClip) {

	Matrix4x4 perspectiveFoVMatrix;

	perspectiveFoVMatrix.m[0][0] = 1.0f / (aspectRatio * std::tan(fovY / 2));;
	perspectiveFoVMatrix.m[0][1] = 0.0f;
	perspectiveFoVMatrix.m[0][2] = 0.0f;
	perspectiveFoVMatrix.m[0][3] = 0.0f;

	perspectiveFoVMatrix.m[1][0] = 0.0f;
	perspectiveFoVMatrix.m[1][1] = 1.0f / (std::tan(fovY / 2));
	perspectiveFoVMatrix.m[1][2] = 0.0f;
	perspectiveFoVMatrix.m[1][3] = 0.0f;

	perspectiveFoVMatrix.m[2][0] = 0.0f;
	perspectiveFoVMatrix.m[2][1] = 0.0f;
	perspectiveFoVMatrix.m[2][2] = farClip / (farClip - nearClip);
	perspectiveFoVMatrix.m[2][3] = 1.0f;

	perspectiveFoVMatrix.m[3][0] = 0.0f;
	perspectiveFoVMatrix.m[3][1] = 0.0f;
	perspectiveFoVMatrix.m[3][2] = -nearClip * farClip / (farClip - nearClip);
	perspectiveFoVMatrix.m[3][3] = 0.0f;

	return perspectiveFoVMatrix;
}

// 正射影行列
Matrix4x4 MakeOrthographicMatrix(float left, float top, float right, float bottom, float nearClip, float farClip) {

	Matrix4x4 orthographicMatrix;

	orthographicMatrix.m[0][0] = 2.0f / (right - left);
	orthographicMatrix.m[0][1] = 0.0f;
	orthographicMatrix.m[0][2] = 0.0f;
	orthographicMatrix.m[0][3] = 0.0f;

	orthographicMatrix.m[1][0] = 0.0f;
	orthographicMatrix.m[1][1] = 2.0f / (top - bottom);
	orthographicMatrix.m[1][2] = 0.0f;
	orthographicMatrix.m[1][3] = 0.0f;

	orthographicMatrix.m[2][0] = 0.0f;
	orthographicMatrix.m[2][1] = 0.0f;
	orthographicMatrix.m[2][2] = 1.0f / (farClip - nearClip);
	orthographicMatrix.m[2][3] = 0.0f;

	orthographicMatrix.m[3][0] = (left + right) / (left - right);
	orthographicMatrix.m[3][1] = (top + bottom) / (bottom - top);
	orthographicMatrix.m[3][2] = nearClip / (nearClip - farClip);
	orthographicMatrix.m[3][3] = 1.0f;

	return orthographicMatrix;
}