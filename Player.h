#pragma once
#include "GameObject.h"
#include "Input.h"
#include "Collider.h"

#include <optional>

class Player :
    public GameObject
{
public:

    void Initialize(const std::string name, ViewProjection* viewProjection, DirectionalLight* directionalLight, WorldTransform pWorld);
    void Update();
    void Collision(Collider& otherCollider);
    void Draw();
public:
    Collider collider_;


private:
    void UpdateState();


private:
    Vector3 velocisity_;
    Vector3 acceleration_;
    Input* input_;

    const float spd_ = 0.1f;

    enum class State{
        kNormal,
        kJump
    };

    State state_ = State::kNormal;

    std::optional<State> stateRequest_ = std::nullopt;




    bool preIsRotating_;

    bool isRotateChasing_ = false;

    void UpdateMapRotating();

    //マップ中心点のTransforation
    WorldTransform* mapCenterTranslation_;
};