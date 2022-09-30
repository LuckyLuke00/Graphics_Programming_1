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
			fovAngle{ _fovAngle }
		{
		}

		// Movement speed constants
		static constexpr float moveSpeed{ 10.f };
		static constexpr float maxSpeed{ 25.f };
		static constexpr float turnSpeed{ 0.25f };

		// Ease factor (between 0 and 1)
		static constexpr float ease{ 0.8f };

		Vector3 velocity{};
		Vector3 origin{};
		float fovAngle{ 90.f };

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
			cameraToWorld = Matrix{ right,up,forward,origin };

			return cameraToWorld;
		}

		void Update(Timer* pTimer)
		{
			const float deltaTime = pTimer->GetElapsed();

			//Keyboard Input
			const uint8_t* pKeyboardState = SDL_GetKeyboardState(nullptr);

			//Mouse Input
			int mouseX{};
			int mouseY{};
			const uint32_t mouseState = SDL_GetRelativeMouseState(&mouseX, &mouseY);

			if (mouseState & SDL_BUTTON(SDL_BUTTON_LEFT) || mouseState & SDL_BUTTON(SDL_BUTTON_RIGHT))
			{
				SDL_SetRelativeMouseMode(SDL_TRUE);
				SDL_ShowCursor(SDL_FALSE);
			}
			else
			{
				SDL_SetRelativeMouseMode(SDL_FALSE);
				SDL_ShowCursor(SDL_TRUE);
			}

			if (mouseState & SDL_BUTTON(SDL_BUTTON_LEFT) || mouseState & SDL_BUTTON(SDL_BUTTON_RIGHT))
			{
				if (pKeyboardState[SDL_SCANCODE_W])
				{
					velocity += forward * moveSpeed;
				}
				if (pKeyboardState[SDL_SCANCODE_S])
				{
					velocity -= forward * moveSpeed;
				}
				if (pKeyboardState[SDL_SCANCODE_A])
				{
					velocity -= right * moveSpeed;
				}
				if (pKeyboardState[SDL_SCANCODE_D])
				{
					velocity += right * moveSpeed;
				}
			}

			// Ease in and out the movement
			velocity *= ease;
			if (velocity.Magnitude() < 0.1f)
			{
				velocity = Vector3{};
			}

			// Clamp the velocity
			if (velocity.Magnitude() > maxSpeed)
			{
				velocity.Normalize();
				velocity *= maxSpeed;
			}
			if (mouseState & SDL_BUTTON(SDL_BUTTON_LEFT) && mouseState & SDL_BUTTON(SDL_BUTTON_RIGHT))
			{
				// Move up and down
				origin += up * -static_cast<float>(mouseY) * deltaTime;
			}

			if (mouseState & SDL_BUTTON(SDL_BUTTON_LEFT) && !(mouseState & SDL_BUTTON(SDL_BUTTON_RIGHT)))
			{
				origin += forward * -static_cast<float>(mouseY) * deltaTime;
			}

			// Rotate camera
			if (mouseState & SDL_BUTTON(SDL_BUTTON_RIGHT) && !(mouseState & SDL_BUTTON(SDL_BUTTON_LEFT)))
			{
				// Rotate camera left and right
				totalYaw += turnSpeed * static_cast<float>(mouseX) * deltaTime;

				// Rotate camera up and down
				totalPitch += turnSpeed * static_cast<float>(mouseY) * deltaTime;
			}

			// Update the camera's position
			origin += velocity * deltaTime;

			const Matrix finalRotation{ Matrix::CreateRotationX(totalPitch) * Matrix::CreateRotationY(totalYaw) };
			forward = finalRotation.TransformVector(Vector3::UnitZ);
			forward.Normalize();

			// Prevent the camera from rolling, it can only pitch and yaw
			up = Vector3::UnitY;
		}
	};
}