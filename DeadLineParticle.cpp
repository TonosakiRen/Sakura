#include "DeadLineParticle.h"
#include "ImGuiManager.h"

DeadLineParticle::DeadLineParticle()
{
	particle_ = std::make_unique<Particle>(kParticleNum);
}

void DeadLineParticle::Initialize(Vector3 minDirection, Vector3 maxDirection)
{

	particle_->Initialize();
	emitterWorldTransform_.SetIsScaleParent(false);
	SetDirection(minDirection, maxDirection);
	emitBox_ = MakeOBB(emitterWorldTransform_.matWorld_);
}

void DeadLineParticle::Update() {


	if (isEmit_) {
		for (size_t i = 0; i < EmitNum_; i++) {
			for (size_t i = 0; i < kParticleNum; i++) {
				if (particles[i].isActive_ == false) {
					particles[i].isActive_ = true;

					emitterWorldTransform_.translation_.y = -27.0f;
					emitterWorldTransform_.translation_.x = mapCenter_.x;
					SetDirection({ 0.0f,0.0f }, { 0.0f,1.0f });
					emitterWorldTransform_.UpdateMatrix();
					emitBox_ = MakeOBB(emitterWorldTransform_.matWorld_);
					emitBox_.size = { 30.0f,1.0f,1.0f };
					
					if (emitterWorldTransform_.GetParent()) {
						particles[i].direction_ = Normalize(Vector3{ Rand(minDirection_.x, maxDirection_.x) ,Rand(minDirection_.y,maxDirection_.y) ,Rand(minDirection_.z,maxDirection_.z) } *NormalizeMakeRotateMatrix(emitterWorldTransform_.GetParent()->matWorld_));
					}
					else {
						particles[i].direction_ = Normalize({ Rand(minDirection_.x, maxDirection_.x) ,Rand(minDirection_.y,maxDirection_.y) ,Rand(minDirection_.z,maxDirection_.z) });
					}
					particles[i].worldTransform_.translation_ = MakeRandVector3(emitBox_);
					particles[i].worldTransform_.rotation_ = { 0.0f,0.0f,0.0f };
					particles[i].worldTransform_.scale_ = emitterWorldTransform_.scale_;
					break;
				}
			}
		}
	}

	for (size_t i = 0; i < kParticleNum; i++) {
		float rotationSpeed = Radian(2.0f) * (float(i % 2) * 2.0f - 1.0f);
		if (particles[i].isActive_ == true) {
			particles[i].worldTransform_.rotation_.z = particles[i].worldTransform_.rotation_.z + rotationSpeed;
			particles[i].worldTransform_.translation_ += particles[i].direction_ * speed_;
			particles[i].worldTransform_.scale_ = particles[i].worldTransform_.scale_ - scaleSpeed_;
			if (particles[i].worldTransform_.scale_.x <= 0.0f) {
				particles[i].isActive_ = false;
			}
		}

	}

}

void DeadLineParticle::Draw(ViewProjection* viewProjection, Vector4 color,uint32_t textureHandle)
{

	emitterWorldTransform_.UpdateMatrix();

	std::vector<Particle::InstancingBufferData> instancingBufferDatas;
	instancingBufferDatas.reserve(kParticleNum);

	Matrix4x4 billbordMatrix =  viewProjection->matView;
	billbordMatrix.m[3][0] = 0.0f;
	billbordMatrix.m[3][1] = 0.0f;
	billbordMatrix.m[3][2] = 0.0f;

	billbordMatrix = Inverse(billbordMatrix);

	for (size_t i = 0; i < kParticleNum; i++)
	{
		if (particles[i].isActive_) {
			particles[i].worldTransform_.UpdateMatrix();
			particles[i].worldTransform_.matWorld_ *= billbordMatrix;
			instancingBufferDatas.emplace_back(particles[i].worldTransform_.matWorld_);
		}
	}

	if (!instancingBufferDatas.empty()) {
		particle_->Draw(instancingBufferDatas, *viewProjection, color, textureHandle);
	}
}
