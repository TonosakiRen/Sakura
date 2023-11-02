#pragma once
#include "GameObject.h"
#include "Input.h"
#include "Collider.h"
class Player :
    public GameObject
{
public:

    void Initialize(const std::string name, ViewProjection* viewProjection, DirectionalLight* directionalLight);
    void Update();
    void Collision(Collider& otherCollider);
    void Draw();
public:
    Collider collider_;
private:
    Vector3 velocisity_;
    Vector3 acceleration_;
    Input* input_;
};