#pragma once

#include <array>
#include <Windows.h>
#include <wrl.h>
#include "Mymath.h"

#define DIRECTINPUT_VERSION 0x0800 
#include <dinput.h>
#include <Xinput.h>

class Input {
public:

	struct MouseState {
		DIMOUSESTATE state;
		Vector2 position;
	};

	static Input* GetInstance();

	void Initialize(HINSTANCE hInstance, HWND hwnd);
	void Update();

	bool PushKey(BYTE keyNumber);
	bool TriggerKey(BYTE keyNumber);
	bool ReleaseKey(BYTE keyNumber);
	Vector2 GetMouseMove();
	float GetWheel();
	bool IsPressMouse(int32_t mouseNumber);
	const std::array<BYTE, 256>& GetAllKey() { return key_; }

	bool PushBotton(int gamePadButton) {
		if (xInputState_.Gamepad.wButtons & gamePadButton) {
			return true;
		}
		return false;
	}

	bool TriggerButton(int gamePadButton) {
		if (xInputState_.Gamepad.wButtons & gamePadButton &&
			!(preXInputState_.Gamepad.wButtons & gamePadButton)) {
			return true;
		}
		return false;
	}

	bool ReleaseButton(int gamePadButton) {
		if (!(xInputState_.Gamepad.wButtons & gamePadButton) &&
			preXInputState_.Gamepad.wButtons & gamePadButton) {
			return true;
		}
		return false;
	}

	XINPUT_GAMEPAD GetGamePad() {
		return xInputState_.Gamepad;
	}

	Vector2 GetLStick() {
		return Vector2{ static_cast<float>(xInputState_.Gamepad.sThumbLX),static_cast<float>(xInputState_.Gamepad.sThumbLY) };
	}
	Vector2 GetRStick() {
		return Vector2{ static_cast<float>(xInputState_.Gamepad.sThumbLX),static_cast<float>(xInputState_.Gamepad.sThumbLY) };
	}

	bool DownLStick(SHORT deadZone = -20000) {
		if (xInputState_.Gamepad.sThumbLY < deadZone) {
			return true;
		}
		return false;
	}
	bool UpLStick(SHORT deadZone = 20000) {
		if (xInputState_.Gamepad.sThumbLY < deadZone) {
			return true;
		}
		return false;
	}
	bool DownLeftLStick(SHORT deadZone = -20000) {
		if (xInputState_.Gamepad.sThumbLX < deadZone) {
			return true;
		}
		return false;
	}
	bool DownRightLStick(SHORT deadZone = 20000) {
		if (xInputState_.Gamepad.sThumbLX < deadZone) {
			return true;
		}
		return false;
	}

	bool DownRStick(SHORT deadZone = -20000) {
		if (xInputState_.Gamepad.sThumbRY < deadZone) {
			return true;
		}
		return false;
	}
	bool UpRStick(SHORT deadZone = 20000) {
		if (xInputState_.Gamepad.sThumbRY < deadZone) {
			return true;
		}
		return false;
	}
	bool DownLeftRStick(SHORT deadZone = -20000) {
		if (xInputState_.Gamepad.sThumbRX < deadZone) {
			return true;
		}
		return false;
	}
	bool DownRightRStick(SHORT deadZone = 20000) {
		if (xInputState_.Gamepad.sThumbRX < deadZone) {
			return true;
		}
		return false;
	}
	bool GetIsGamePadConnect() {
		return isGamePadConnect;
	}

private:
	Microsoft::WRL::ComPtr<IDirectInput8> dInput_;
	Microsoft::WRL::ComPtr<IDirectInputDevice8> devKeyboard_;
	Microsoft::WRL::ComPtr<IDirectInputDevice8> devMouse_;
	std::array<BYTE, 256> key_{};
	std::array<BYTE, 256> keyPre_{};
	MouseState mouseState_{};
	MouseState preMouseState_{};
	XINPUT_STATE xInputState_{};
	XINPUT_STATE preXInputState_{};
	bool isGamePadConnect = false;

private:
	Input() = default;
	~Input() = default;
	Input(const Input&) = delete;
	const Input& operator=(const Input&) = delete;
};
