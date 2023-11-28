#pragma once
#include "GameObject.h"
class Collider
{
public:

	static bool isDrawCollider;
	static void SwitchIsDrawCollider();

	void Initialize(WorldTransform* objectWorldTransform, const std::string name, ViewProjection* viewProjection, DirectionalLight* directionalLight, Vector3 initialScale = { 1.0f,1.0f,1.0f }, Vector3 initialPos = { 0.0f,0.0f,0.0f });
	//ただの四角形用
	void Initialize(const std::string name, ViewProjection* viewProjection, DirectionalLight* directionalLight);
	bool Collision(Collider& colliderB);
	bool Collision(Collider& colliderB, Vector3& minAxis, float& minOverlap);
	bool Collision(Collider& colliderB, Vector3& pushBuckVector);
	bool Collision(Collider& colliderB, Vector3& pushBuckVector, const Vector3& priorityAxis);

	void AdjustmentScale();
	void MatrixUpdate();
	void Draw(Vector4 color = { 1.0f,1.0f,1.0f,1.0f });

	bool ObbCollision(const OBB& obb1, const OBB& obb2);
	bool ObbCollision(const OBB& obb1, const OBB& obb2, Vector3& minAxis, float& minOverlap);
	bool ObbCollision(const OBB& obb1, const OBB& obb2, Vector3& minAxis, float& minOverlap,const Vector3& priorityAxis);

	void prePosUpdate();

public:
	WorldTransform worldTransform_;
	Vector2 preLeftTop_;
	Vector2 preRightBottom_;
private:
	GameObject cube_;
	std::string name_;

};