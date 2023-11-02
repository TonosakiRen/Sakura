#include "ViewProjection.h"
#include "WinApp.h"
#include <cassert>
#include "externals/DirectXTex/d3dx12.h"
#include "Input.h"
#include "DirectXCommon.h"
#include "ImGuiManager.h"
#include "Easing.h"

using namespace DirectX;

void ViewProjection::Initialize() {
    CreateConstBuffer();
    Map();
    UpdateMatrix();
}

void ViewProjection::CreateConstBuffer() {
    HRESULT result;
    ID3D12Device* device = DirectXCommon::GetInstance()->GetDevice();
    // ヒーププロパティ
    CD3DX12_HEAP_PROPERTIES heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    // リソース設定
    CD3DX12_RESOURCE_DESC resourceDesc =
        CD3DX12_RESOURCE_DESC::Buffer((sizeof(ConstBufferDataViewProjection) + 0xff) & ~0xff);

    // 定数バッファの生成
    result = device->CreateCommittedResource(
        &heapProps, // アップロード可能
        D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
        IID_PPV_ARGS(&constBuff_));
    assert(SUCCEEDED(result));
}

void ViewProjection::Map() {
    // 定数バッファとのデータリンク
    HRESULT result = constBuff_->Map(0, nullptr, (void**)&constMap);
    assert(SUCCEEDED(result));
}

void ViewProjection::UpdateMatrix() {

    // ビュー行列の生成

    Vector3 tranlation = translation_ + Vector3{ Rand(-shakeValue_.x,shakeValue_.x),Rand(-shakeValue_.y,shakeValue_.y) ,Rand(-shakeValue_.z,shakeValue_.z) };
    matView = MakeViewMatirx(target_, tranlation);

    // 透視投影による射影行列の生成
    matProjection = MakePerspectiveFovMatrix(fovAngleY_, aspectRatio_, nearZ_, farZ_);

    // 定数バッファに書き込み
    constMap->view = matView;
    constMap->projection = matProjection;
    constMap->viewPosition = translation_;
}

bool ViewProjection::Shake(Vector3 shakeValue, int& frame)
{
    frame--;
    if (frame > 0) {
        shakeValue_ = shakeValue;
        return true;
    }
    frame = 0;
    shakeValue_ = { 0.0f,0.0f,0.0f };
    return false;
}

void ViewProjection::DebugMove() {
    Input* input = Input::GetInstance();

    Vector2 mouseMove = input->GetMouseMove();
    float wheel = input->GetWheel();
#ifdef _DEBUG
    ImGui::Begin("Camera");
    ImGui::DragFloat3("target", &target_.x, 0.01f);
    ImGui::DragFloat3("translation", &translation_.x, 0.01f);
    ImGui::End();
#endif // _DEBUG



    if (input->IsPressMouse(1)) {
        float rot = static_cast<float>(M_PI / 180.0f);
        target_.x += rot * mouseMove.y * 0.1f;
        target_.y += rot * mouseMove.x * 0.1f;
    }
    else if (input->IsPressMouse(2)) {
        Matrix4x4 rotMat = MakeRotateXYZMatrix(target_);
        Vector3 cameraX = GetXAxis(rotMat) * static_cast<float>(-mouseMove.x) * 0.01f;
        Vector3 cameraY = GetYAxis(rotMat) * static_cast<float>(mouseMove.y) * 0.01f;
        translation_ += cameraX + cameraY;
    }
    else if (wheel != 0) {
        Matrix4x4 rotMat = MakeRotateXYZMatrix(target_);
        Vector3 cameraZ = GetZAxis(rotMat) * (static_cast<float>(wheel / 120) * 0.5f);
        translation_ += cameraZ;
    }
}

