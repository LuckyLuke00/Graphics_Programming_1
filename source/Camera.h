#pragma once
#include <iostream>
#include <SDL_keyboard.h>
#include <SDL_mouse.h>

#include "Math.h"
#include "Timer.h"

namespace dae
{
	struct Camera
	{
		Camera() = default;

		Camera(const Vector3& _origin, float _fovAngle) :
			origin{ _origin },
			fovAngle{ _fovAngle },
			lastFovAngle{ _fovAngle }
		{
		}

		// Movement speed constants
		static constexpr float moveSpeed{ 10.f };
		static constexpr float maxSpeed{ 25.f };
		static constexpr float turnSpeed{ 0.25f };
		static constexpr float shiftMultiplier{ 4.f };

		// Ease factor (between 0 and 1)
		static constexpr float ease{ 0.8f };

		Vector3 velocity{};
		Vector3 origin{};

		float fovAngle{ 90.f };
		float lastFovAngle{ fovAngle };
		float fov{ tanf(TO_RADIANS * fovAngle / 2.f) };
		static constexpr float maxFovAngle{ 170.f };
		static constexpr float minFovAngle{ 5.f };
		static constexpr float fovSpeed{ 100.f };

		Vector3 forward{ Vector3::UnitZ };
		Vector3 up{ Vector3::UnitY };
		Vector3 right{ Vector3::UnitX };

		float totalPitch{ 0.f };
		float totalYaw{ 0.f };

		Matrix cameraToWorld{};

		Matrix CalculateCameraToWorld()
		{
			//This function should return the Camera ONB matrix
			//Calculate the right & up vector using the forward camera vector
			//Combine to a matrix (also include origin) and return

			//Get the right vector
			right = Vector3::Cross(up, forward).Normalized();

			//Get the up vector
			up = Vector3::Cross(forward, right).Normalized();

			//Make the matrix
			return cameraToWorld = Matrix{ right,up,forward,origin };
		}

		void Update(Timer* pTimer)
		{
			const float deltaTime{ pTimer->GetElapsed() };

			//Keyboard Input
			const uint8_t* pKeyboardState{ SDL_GetKeyboardState(nullptr) };

			//Mouse Input
			int mouseX{};
			int mouseY{};
			const uint32_t mouseState{ SDL_GetRelativeMouseState(&mouseX, &mouseY) };

			// Float safe NEQ check
			if (!AreEqual(fovAngle, lastFovAngle))
			{
				fov = tanf(TO_RADIANS * fovAngle / 2.f);
			}

			// Multiplier if shift is held
			const float shift{ 1.f + (shiftMultiplier - 1.f) * static_cast<float>(pKeyboardState[SDL_SCANCODE_LSHIFT]) };

			// Ease in and out the velocity
			velocity *= ease;
			if (velocity.Magnitude() < 0.1f)
			{
				velocity = Vector3::Zero;
			}

			// Clamp the velocity
			if (velocity.Magnitude() > maxSpeed * shift)
			{
				velocity.Normalize();
				velocity *= maxSpeed * shift;
			}

			// Update the camera's position
			origin += velocity * deltaTime;

			const unsigned leftMousePressed{ mouseState & SDL_BUTTON(SDL_BUTTON_LEFT) };
			const unsigned rightMousePressed{ mouseState & SDL_BUTTON(SDL_BUTTON_RIGHT) };

			// If right mouse button is not pressed return
			if (!(leftMousePressed || rightMousePressed))
			{
				// Lock the mouse to the window
				SDL_SetRelativeMouseMode(SDL_FALSE);
				return;
			}

			SDL_SetRelativeMouseMode(SDL_TRUE);

			// FOV
			fovAngle += static_cast<float>(pKeyboardState[SDL_SCANCODE_RIGHT]) * fovSpeed * deltaTime;
			fovAngle -= static_cast<float>(pKeyboardState[SDL_SCANCODE_LEFT]) * fovSpeed * deltaTime;
			Clampf(fovAngle, minFovAngle, maxFovAngle);

			// WASD movement
			velocity += forward * moveSpeed * pKeyboardState[SDL_SCANCODE_W] * shift;
			velocity -= forward * moveSpeed * pKeyboardState[SDL_SCANCODE_S] * shift;

			velocity -= right * moveSpeed * pKeyboardState[SDL_SCANCODE_A] * shift;
			velocity += right * moveSpeed * pKeyboardState[SDL_SCANCODE_D] * shift;

			// Move camera up and down when right and left mouse buttons are pressed
			origin += up * -static_cast<float>(mouseY) * (leftMousePressed && rightMousePressed) * deltaTime;

			// Move camera forward and backward when only the left mouse button is pressed
			origin += forward * -static_cast<float>(mouseY) * (leftMousePressed && !rightMousePressed) * deltaTime;

			// Yaw and pitch the camera when only the right mouse button is pressed
			// Rotate camera left and right
			totalYaw += turnSpeed * static_cast<float>(mouseX) * static_cast<float>(!leftMousePressed && rightMousePressed) * deltaTime;

			// Rotate camera up and down
			totalPitch += turnSpeed * static_cast<float>(mouseY) * static_cast<float>(!leftMousePressed && rightMousePressed) * deltaTime;

			const Matrix finalRotation{ Matrix::CreateRotationX(totalPitch) * Matrix::CreateRotationY(totalYaw) };
			forward = finalRotation.TransformVector(Vector3::UnitZ).Normalized();

			// Prevent the camera from rolling, it can only pitch and yaw
			up = Vector3::UnitY;
		}
	};
}