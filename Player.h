#pragma once
#include "GameObject.h"
#include "Input.h"
#include "Collider.h"
#include"Box.h"

#include<vector>
#include <optional>

//状態
enum class PlayerState {
    kNormal,
    kJump
};


class Player :
    public GameObject
{
public:

    //初期化
    void Initialize(const std::string name, ViewProjection* viewProjection, DirectionalLight* directionalLight, WorldTransform pWorld);
   
    //更新
    void Update();
    
    //描画
    void Draw();

    //コリジョン判定と処理
    void Collision(Collider& otherCollider);

    //ぴったり配置されているか
    bool IsSetPerfect(Collider& otherCollider);
   
    //当たったか否か
    bool IsCollision(Collider& otherCollider);
   
    //下のコライダーのフラグ
    void UnderColliderCollision(Collider& otherCollider);
  
    //↓コライダーに当たったか否か
    bool IsUnderColliderCollision(Collider& otherCollider);

   

    //状態更新
    void SetState(PlayerState state) { 
        //
        if (state_ != state) {
            stateRequest_ = state;
        }
    }

    //初期化なしで状態変更
    void SetStateNoInitialize(PlayerState state) {
        state_ = state;
    }
  
    //初速度0でジャンプ（落下
    void SetFall() {
        state_ = PlayerState::kJump;
        //velocisity_ = { 0.0f,0.0f,0.0f };
    }

   
    //引数のシーンと現在のシーンは同じかチェック
    bool CheckStateSame(PlayerState state) {
        if (state_ != state) {
            return false;
        }
        else {
            return true;
        }
        
    }

public:
    //本体のコライダー
    Collider collider_;
    //プレイヤー真下のコライダー
    Collider underCollider_;

private:
    //状態更新
    void UpdateState();


private:
    //速度
    Vector3 velocisity_;
    //加速量
    Vector3 acceleration_;
    //キー
    Input* input_;

    //移動速度
    const float spd_ = 0.1f;

    

    //状態変数
    PlayerState state_ = PlayerState::kNormal;
    //状態変更時受け取る
    std::optional<PlayerState> stateRequest_ = std::nullopt;

    //前シーンの回転フラグ受け取り
    bool preIsRotating_;

    //埋まって処理を行ったか否か
    bool isBuried_ = false;
};