#pragma once
#include <cassert>
#include <SDL_keyboard.h>
#include <SDL_mouse.h>

#include <iostream>

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

		//Movement
		Vector3 origin{ Vector3::Zero };
		Vector3 velocity{ Vector3::Zero };

		//Movement constants
		const float moveSpeed{ 10.0f };

		//Input
		bool canMove{ false };
		int mouseX{ 0 };
		int mouseY{ 0 };
		uint32_t mouseState{};
		const uint8_t* pKeyboardState{ SDL_GetKeyboardState(nullptr) };

		//Field of view
		float fovAngle{ 90.f };
		float fov{ tanf((fovAngle * TO_RADIANS) / 2.f) };

		//Rotation
		Vector3 forward{ Vector3::UnitZ };
		Vector3 up{ Vector3::UnitY };
		Vector3 right{ Vector3::UnitX };

		float totalPitch{ 0.f };
		float totalYaw{ 0.f };

		Matrix invViewMatrix{};
		Matrix viewMatrix{};

		void Initialize(float _fovAngle = 90.f, Vector3 _origin = { .0f, .0f, .0f })
		{
			fovAngle = _fovAngle;
			fov = tanf((fovAngle * TO_RADIANS) / 2.f);

			origin = _origin;
		}

		void CalculateViewMatrix()
		{
			viewMatrix = Matrix::CreateLookAtLH(origin, forward, up);
			invViewMatrix = Matrix::Inverse(viewMatrix);
		}

		void CalculateProjectionMatrix()
		{
			//TODO W2

			//ProjectionMatrix => Matrix::CreatePerspectiveFovLH(...) [not implemented yet]
			//DirectX Implementation => https://learn.microsoft.com/en-us/windows/win32/direct3d9/d3dxmatrixperspectivefovlh
		}

		void Update(Timer* pTimer)
		{
			const float deltaTime{ pTimer->GetElapsed() };

			//Camera Update Logic
			UpdateMouse();

			HandleKeyboardMovement(deltaTime);
			HandleMouseMovement(deltaTime);
			UpdateVectors();

			//Update Matrices
			CalculateViewMatrix();
			CalculateProjectionMatrix(); //Try to optimize this - should only be called once or when fov/aspectRatio changes
		}

		void UpdateMouse()
		{
			mouseState = SDL_GetRelativeMouseState(&mouseX, &mouseY);

			canMove =
				mouseState & SDL_BUTTON(SDL_BUTTON_LEFT) ||
				mouseState & SDL_BUTTON(SDL_BUTTON_RIGHT) ||
				mouseState & SDL_BUTTON(SDL_BUTTON_MIDDLE);

			//Hide or show the mouse cursor if the user can move the camera
			SDL_SetRelativeMouseMode(canMove ? SDL_TRUE : SDL_FALSE);
		}

		void HandleKeyboardMovement(const float& dt)
		{
			//Prevent floating point accumulation and allow for smooth movement
			if (!canMove && velocity.SqrMagnitude() < 0.01f) return;

			//Use WASD to move the camera if CanMove() is false (no mouse buttons pressed) the value will be 0.f
			float moveForward{ static_cast<float>(pKeyboardState[SDL_SCANCODE_W] - pKeyboardState[SDL_SCANCODE_S]) };
			float moveRight{ static_cast<float>(pKeyboardState[SDL_SCANCODE_D] - pKeyboardState[SDL_SCANCODE_A]) };

			//Lerp the velocity to the desired direction
			const Vector3 desiredVelocity{ (forward * moveForward + right * moveRight) * moveSpeed * canMove };
			velocity = Vector3::Lerp(velocity, desiredVelocity, dt * 10.f);

			//Add the velocity to the origin
			origin += velocity * dt;
		}

		void HandleMouseMovement(const float& dt)
		{
			if (!canMove) return;

			//Type cast mouse x and y to float
			const float x{ static_cast<float>(mouseX) };
			const float y{ -static_cast<float>(mouseY) };

			//Store mouse button states
			const uint32_t& leftMouse{ mouseState & SDL_BUTTON(SDL_BUTTON_LEFT) };
			const uint32_t& rightMouse{ mouseState & SDL_BUTTON(SDL_BUTTON_RIGHT) };
			const uint32_t& middleMouse{ mouseState & SDL_BUTTON(SDL_BUTTON_MIDDLE) };

			//Camera movement
			//Move the camera up, down, left and right
			//When both left and right or middle mouse buttons are pressed
			origin += (up * y + right * x) * dt * (middleMouse || (leftMouse && rightMouse));

			//Move the camera forward and backward
			//When only the left mouse button is pressed
			origin += forward * y * dt * (leftMouse && !rightMouse);

			//Camera rotation
			//Pitch the camera up and down
			//When only the right mouse button is pressed
			totalPitch += y * dt * static_cast<float>(!leftMouse && rightMouse);

			//Yaw the camera left and right
			//When only the right mouse button or only the left mouse button is pressed
			totalYaw += x * dt * static_cast<float>(rightMouse && !leftMouse || leftMouse && !rightMouse);
		}

		//Function that updates forward, up and right vectors
		void UpdateVectors()
		{
			//Calculate the forward vector
			forward = Matrix::CreateRotation(totalPitch, totalYaw, .0f).TransformVector(Vector3::UnitZ);

			//Calculate the right vector
			right = Matrix::CreateRotation(totalPitch, totalYaw, .0f).TransformVector(Vector3::UnitX);

			//Calculate the up vector
			up = Matrix::CreateRotation(totalPitch, totalYaw, .0f).TransformVector(Vector3::UnitY);
		}
	};
}
