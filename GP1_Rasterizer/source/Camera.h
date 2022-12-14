#pragma once

#include "Math.h"
#include "Timer.h"

namespace dae
{
	struct Camera
	{
		// Constructor
		Camera() = default;

		Camera(const Camera&) = delete;
		Camera(Camera&&) noexcept = delete;
		Camera& operator=(const Camera&) = delete;
		Camera& operator=(Camera&&) noexcept = delete;

		// Destructor
		~Camera() = default;

		// Transform matrices
		Matrix projectionMatrix{};
		Matrix viewMatrix{};

		// Camera position and orientation
		Vector3 origin{ Vector3::Zero };

		Vector3 forward{ Vector3::UnitZ };
		Vector3 right{ Vector3::UnitX };
		Vector3 up{ Vector3::UnitY };

		// Camera orientation
		float pitch{};
		float yaw{};

		// Camera movement
		Vector3 velocity{ Vector3::Zero };

		// Field of view and aspect ratio
		float aspectRatio{};
		float lastAspectRatio{};

		float fov{};
		float fovAngle{};
		float lastFovAngle{};

		// Time
		float deltaTime{};

		// Mouse input
		Int2 mousePos{};

		Uint32 mouseLeft{};
		Uint32 mouseMiddle{};
		Uint32 mouseRight{};
		Uint32 mouseState{};

		// Camera behavior constants
		static constexpr float far{ 100.f };
		static constexpr float near{ .1f };

		static constexpr float moveSpeed{ 50.f };
		static constexpr float sensitivity{ .0025f };

		void Initialize(const float _aspectRatio = 1.f, const float _fovAngle = 90.f, const Vector3& _origin = Vector3::Zero)
		{
			// Set the camera's initial position and orientation
			origin = _origin;

			// Set the camera's initial field of view and aspect ratio
			fovAngle = _fovAngle;
			lastFovAngle = _fovAngle;
			fov = tanf((_fovAngle * .5f) * TO_RADIANS);

			aspectRatio = _aspectRatio;
			lastAspectRatio = _aspectRatio;

			// Calculate the initial view and projection matrices
			CalculateViewMatrix();
			CalculateProjectionMatrix();
		}

		void Update(const Timer* pTimer)
		{
			// Update the camera's elapsed time
			deltaTime = pTimer->GetElapsed();

			// Update the camera's mouse state
			UpdateMouseState();

			// Handle mouse and keyboard input for camera movement
			HandleMouseMovement();
			HandleKeyboardMovement();

			// Update the camera's orientation vectors
			UpdateCameraVectors();

			// Calculate the updated view and projection matrices
			CalculateViewMatrix();
			if (fovAngle != lastFovAngle || aspectRatio != lastAspectRatio)
			{
				CalculateProjectionMatrix();
			}
		}

		void CalculateProjectionMatrix()
		{
			projectionMatrix = Matrix::CreatePerspectiveFovLH(fov, aspectRatio, near, far);
		}

		void CalculateViewMatrix()
		{
			viewMatrix = Matrix::CreateLookAtLH(origin, forward, up);
		}

		void UpdateMouseState()
		{
			// Update the current mouse state
			mouseState = SDL_GetRelativeMouseState(&mousePos.x, &mousePos.y);

			// Hide the mouse cursor when any mouse button is pressed
			SDL_SetRelativeMouseMode(SDL_bool(mouseState));

			// Return if no mouse buttons are pressed
			if (!mouseState) return;

			// Update the state of each mouse button
			mouseLeft = mouseState & SDL_BUTTON(SDL_BUTTON_LEFT);
			mouseMiddle = mouseState & SDL_BUTTON(SDL_BUTTON_MIDDLE);
			mouseRight = mouseState & SDL_BUTTON(SDL_BUTTON_RIGHT);
		}

		void HandleKeyboardMovement()
		{
			// Return if the player is not moving the mouse and is already standing still
			if (!mouseState && IsStandingStill()) return;

			// Get the current state of the keyboard
			const Uint8* pKeyboardState{ SDL_GetKeyboardState(nullptr) };

			// Calculate the forward and right movement deltas based on the keyboard input
			const float forwardDelta{ static_cast<float>(pKeyboardState[SDL_SCANCODE_W] - pKeyboardState[SDL_SCANCODE_S]) };
			const float rightDelta{ static_cast<float>(pKeyboardState[SDL_SCANCODE_D] - pKeyboardState[SDL_SCANCODE_A]) };

			// Calculate the desired velocity based on the mouse state and keyboard input
			const Vector3 desiredVelocity{ (mouseState ? (forward * forwardDelta + right * rightDelta) * moveSpeed : Vector3::Zero) };

			// Update the velocity using a linear interpolation to smoothly transition to the desired velocity
			constexpr float lerpSpeed{ 5.f };
			velocity = Lerp(velocity, desiredVelocity, deltaTime * lerpSpeed);

			// Update the player's position based on the calculated velocity
			origin += velocity * deltaTime;
		}

		// Returns true if the player is not moving (i.e. their velocity is below a certain threshold)
		bool IsStandingStill(const float _tolerance = .1f) const
		{
			return velocity.SqrMagnitude() < _tolerance;
		}

		void HandleMouseMovement()
		{
			// Return if the no mouse buttons are pressed
			if (!mouseState) return;

			// Calculate the x and y movement deltas based on the current mouse position and sensitivity
			const float x{ static_cast<float>(mousePos.x) * sensitivity };
			const float y{ -static_cast<float>(mousePos.y) * sensitivity };

			// Update the player's position based on the mouse movement and button states
			origin += (forward * y * (moveSpeed * .5f)) * (mouseLeft && !mouseRight);
			origin += (right * x * (moveSpeed * .5f)) * (mouseMiddle || (mouseLeft && mouseRight));
			origin += (up * y * (moveSpeed * .5f)) * (mouseMiddle || (mouseLeft && mouseRight));

			// Update the player's pitch and yaw based on the mouse movement and button states
			pitch += y * static_cast<float>(!mouseLeft && mouseRight);
			yaw += x * static_cast<float>(mouseRight && !mouseLeft || mouseLeft && !mouseRight);
		}

		void UpdateCameraVectors()
		{
			forward = Matrix::CreateRotation(pitch, yaw, .0f).TransformVector(Vector3::UnitZ);
			right = Matrix::CreateRotation(pitch, yaw, .0f).TransformVector(Vector3::UnitX);
			up = Matrix::CreateRotation(pitch, yaw, .0f).TransformVector(Vector3::UnitY);
		}
	};
}
