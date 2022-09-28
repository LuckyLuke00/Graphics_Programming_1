#pragma once
#include <cassert>
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
		static constexpr float moveSpeed = 10.f;
		static constexpr float turnSpeed = 1.f;

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
			right = Vector3::Cross(up, forward);

			//Get the up vector
			up = Vector3::Cross(forward, right);

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
			int mouseX{}, mouseY{};
			const uint32_t mouseState = SDL_GetRelativeMouseState(&mouseX, &mouseY);

			// Camera Movement: Transforming the camera’s origin (WASD || LMB + MouseY || LMB + RMB + MouseY)
			// Camera Rotation: Transforming the camera’s forward vector (LMB + MouseX || LMR + MouseX/Y)

			//Camera Movement
			if (pKeyboardState[SDL_SCANCODE_W])
			{
				origin += forward * moveSpeed * deltaTime;
			}
			if (pKeyboardState[SDL_SCANCODE_S])
			{
				origin -= forward * moveSpeed * deltaTime;
			}
			if (pKeyboardState[SDL_SCANCODE_A])
			{
				origin -= right * moveSpeed * deltaTime;
			}
			if (pKeyboardState[SDL_SCANCODE_D])
			{
				origin += right * moveSpeed * deltaTime;
			}
		}
	};
}