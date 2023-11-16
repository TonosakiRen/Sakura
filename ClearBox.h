#pragma once
#include "GameObject.h"
#include"Collider.h"


class ClearBox :
    public GameObject {

    /// <summary>
    /// 初期化
    /// </summary>
    /// <param name="name"></param>
    /// <param name="viewProjection"></param>
    /// <param name="directionalLight"></param>
    /// <param name="pWorld"></param>
    void Initialize(const std::string name, ViewProjection* viewProjection, DirectionalLight* directionalLight, WorldTransform pWorld);
    
    /// <summary>
    /// 更新
    /// </summary>
    void Update();

    /// <summary>
    /// コリジョンの判定と処理
    /// </summary>
    /// <param name="otherCollider"></param>
    void Collision(Collider& otherCollider);

    /// <summary>
    /// 描画
    /// </summary>
    void Draw();

private:
};

