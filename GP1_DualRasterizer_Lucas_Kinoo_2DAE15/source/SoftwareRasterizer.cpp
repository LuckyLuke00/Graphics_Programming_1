#include "pch.h"
#include "SoftwareRasterizer.h"

#include "DataTypes.h"
#include "Mesh.h"
#include "Texture.h"

namespace dae {
	SoftwareRasterizer::SoftwareRasterizer(SDL_Window* pWindow)
		: m_pWindow{ pWindow }
	{
		//Initialize
		SDL_GetWindowSize(pWindow, &m_Width, &m_Height);
		m_fWidth = static_cast<float>(m_Width);
		m_fHeight = static_cast<float>(m_Height);

		//Create Buffers
		m_pFrontBuffer = SDL_GetWindowSurface(pWindow);
		m_pBackBuffer = SDL_CreateRGBSurface(0, m_Width, m_Height, 32, 0, 0, 0, 0);
		m_pBackBufferPixels = static_cast<uint32_t*>(m_pBackBuffer->pixels);
		m_pDepthBufferPixels = new float[m_Width * m_Height];

		ClearDepthBuffer();
	}

	SoftwareRasterizer::~SoftwareRasterizer()
	{
		delete[] m_pDepthBufferPixels;
		m_pDepthBufferPixels = nullptr;
	}

	void SoftwareRasterizer::Render(const ColorRGB& clearColor)
	{
		//@START
		//Lock BackBuffer
		SDL_LockSurface(m_pBackBuffer);

		ClearDepthBuffer();
		ClearBackBuffer(clearColor);

		VertexTransformationFunction(m_pMeshes);
		for (int idx{ 0 }; const auto & mesh : m_pMeshes)
		{
			m_CurrentMeshIndex = idx;

			RenderMesh(mesh);

			++idx;
		}

		//@END
		//Update SDL Surface
		SDL_UnlockSurface(m_pBackBuffer);
		SDL_BlitSurface(m_pBackBuffer, nullptr, m_pFrontBuffer, nullptr);
		SDL_UpdateWindowSurface(m_pWindow);
	}

	bool SoftwareRasterizer::SaveBufferToImage() const
	{
		return SDL_SaveBMP(m_pBackBuffer, "Rasterizer_ColorBuffer.bmp");
	}

	void SoftwareRasterizer::CycleShadingMode()
	{
		static constexpr int enumSize{ sizeof(ShadingMode) };
		m_ShadingMode = static_cast<ShadingMode>((static_cast<int>(m_ShadingMode) + 1) % enumSize);
	}

	void SoftwareRasterizer::ClearDepthBuffer() const
	{
		std::fill_n(m_pDepthBufferPixels, m_Width * m_Height, FLT_MAX);
	}

	void SoftwareRasterizer::ClearBackBuffer(const ColorRGB& clearColor) const
	{
		SDL_FillRect
		(
			m_pBackBuffer,
			nullptr,
			SDL_MapRGB(
				m_pBackBuffer->format,
				static_cast<Uint8>(clearColor.r * 255),
				static_cast<Uint8>(clearColor.g * 255),
				static_cast<Uint8>(clearColor.b * 255)
			)
		);
	}

	void SoftwareRasterizer::RenderMesh(const Mesh* pMesh) const
	{
		const bool isTriangleList{ pMesh->GetPrimitiveTopology() == PrimitiveTopology::TriangleList };

		const int increment{ isTriangleList ? 3 : 1 };
		const size_t size{ isTriangleList ? pMesh->GetIndices().size() : pMesh->GetIndices().size() - 2 };

		for (int i{ 0 }; i < size; i += increment)
		{
			const uint32_t& idx0{ pMesh->GetIndices()[i] };
			const uint32_t& idx1{ pMesh->GetIndices()[i + 1] };
			const uint32_t& idx2{ pMesh->GetIndices()[i + 2] };

			// If any of the indexes are equal skip
			if (idx0 == idx1 || idx1 == idx2 || idx2 == idx0) continue;

			const Vertex_Out& v0{ pMesh->GetVerticesOut()[idx0] };
			const Vertex_Out& v1{ pMesh->GetVerticesOut()[idx1] };
			const Vertex_Out& v2{ pMesh->GetVerticesOut()[idx2] };

			if (isTriangleList)
			{
				RenderTriangle(v0, v1, v2);
				continue;
			}
			RenderTriangle(i % 2 == 0 ? v0 : v2, v1, i % 2 == 0 ? v2 : v0);
		}
	}

	void SoftwareRasterizer::RenderTriangle(const Vertex_Out& v0, const Vertex_Out& v1, const Vertex_Out& v2) const
	{
		if (IsOutsideViewFrustum(v0) || IsOutsideViewFrustum(v1) || IsOutsideViewFrustum(v2)) return;

		const Vector2& v0Pos{ v0.pos.GetXY() };
		const Vector2& v1Pos{ v1.pos.GetXY() };
		const Vector2& v2Pos{ v2.pos.GetXY() };

		// Calculate the bounding box - but make sure the triangle is inside the screen
		Int2 min;
		Int2 max;
		CalculateBoundingBox(v0, v1, v2, min, max);

		const float area{ EdgeFunction(v0Pos, v1Pos, v2Pos) };

		// Cullmode checks
		const bool isAreaNegative{ area < FLT_EPSILON };
		if (isAreaNegative && m_CullMode == CullMode::Back) return;
		else if (!isAreaNegative && m_CullMode == CullMode::Front) return;

		const float invArea{ Inverse(area) };

		// Pre-calculate the inverse z
		const float z0{ Inverse(v0.pos.z) };
		const float z1{ Inverse(v1.pos.z) };
		const float z2{ Inverse(v2.pos.z) };

		// Pre-calculate the inverse w
		const float w0V{ Inverse(v0.pos.w) };
		const float w1V{ Inverse(v1.pos.w) };
		const float w2V{ Inverse(v2.pos.w) };

		// Pre calculate the uv coordinates
		const Vector2& uv0{ v0.uv / v0.pos.w };
		const Vector2& uv1{ v1.uv / v1.pos.w };
		const Vector2& uv2{ v2.uv / v2.pos.w };

		// Pre calculate the color coordinates
		const ColorRGB& c0{ v0.col / v0.pos.w };
		const ColorRGB& c1{ v1.col / v1.pos.w };
		const ColorRGB& c2{ v2.col / v2.pos.w };

		// Loop over the bounding box
		for (int py{ min.y }; py < max.y; ++py)
		{
			for (int px{ min.x }; px < max.x; ++px)
			{
				// Check if the pixel is inside the triangle
				// If so, draw the pixel
				const Vector2 pixel{ static_cast<float>(px) + .5f, static_cast<float>(py) + .5f };
				const int zBufferIdx{ py * m_Width + px };

				if (m_RenderBoundingBox)
				{
					m_pBackBufferPixels[zBufferIdx] = SDL_MapRGB(m_pBackBuffer->format, 255, 255, 255);
					continue;
				}

				const float w0{ EdgeFunction(v1Pos, v2Pos, pixel) * invArea };
				if (w0 < .0f) continue;

				const float w1{ EdgeFunction(v2Pos, v0Pos, pixel) * invArea };
				if (w1 < .0f) continue;

				// Optimize by not calculating the cross product for the last edge
				const float w2{ 1.f - w0 - w1 };
				if (w2 < .0f) continue;

				// Calculate the depth account for perspective interpolation
				const float z{ Inverse(z0 * w0 + z1 * w1 + z2 * w2) };
				float& zBuffer{ m_pDepthBufferPixels[zBufferIdx] };

				//Check if pixel is in front of the current pixel in the depth buffer
				if (z >= zBuffer) continue;

				//Update depth buffer
				zBuffer = z;

				ColorRGB finalColor{ colors::Black };

				if (m_RenderDepthBuffer)
				{
					const float depthColor{ Remap(z, .997f, 1.f) };
					finalColor = colors::White * depthColor;
				}
				else
				{
					// Interpolated w
					const float w{ Inverse(w0V * w0 + w1V * w1 + w2V * w2) };

					const Vertex_Out interpolatedVertex
					{
						{ pixel.x, pixel.y, z, w },
						((v0.norm * w0 + v1.norm * w1 + v2.norm * w2) * w).Normalized(),
						((v0.tan * w0 + v1.tan * w1 + v2.tan * w2) * w).Normalized(),
						(uv0 * w0 + uv1 * w1 + uv2 * w2) * w,
						c0 * w0 + c1 * w1 + c2 * w2,
						((v0.view * w0 + v1.view * w1 + v2.view * w2) * w).Normalized()
					};

					finalColor = PixelShading(interpolatedVertex);
				}

				//Update Color in Buffer
				finalColor.MaxToOne();

				m_pBackBufferPixels[zBufferIdx] = SDL_MapRGB(m_pBackBuffer->format,
					static_cast<uint8_t>(finalColor.r * 255),
					static_cast<uint8_t>(finalColor.g * 255),
					static_cast<uint8_t>(finalColor.b * 255));
			}
		}
	}

	ColorRGB SoftwareRasterizer::PixelShading(const Vertex_Out& v) const
	{
		//const Texture* pDiffuse{ m_pMeshes[m_CurrentMeshIndex]->GetDiffuse() };
		//const Texture* pGloss{ m_pMeshes[m_CurrentMeshIndex]->GetGloss() };
		//const Texture* pNormal{ m_pMeshes[m_CurrentMeshIndex]->GetNormal() };
		//const Texture* pSpecular{ m_pMeshes[m_CurrentMeshIndex]->GetSpecular() };

		////// Sampled texture colors
		//const ColorRGB sampledColor{ (pDiffuse ? pDiffuse->Sample(v.uv) : colors::Black) };
		//const ColorRGB sampledNormal{ (pNormal ? pNormal->Sample(v.uv) : colors::Black) };
		//const ColorRGB sampledSpecular{ (pSpecular ? pSpecular->Sample(v.uv) : colors::Black) };
		//const ColorRGB sampledGloss{ (pGloss ? pGloss->Sample(v.uv) : colors::Black) };

		const ColorRGB sampledColor{ colors::White };
		const ColorRGB sampledNormal{ colors::White };
		const ColorRGB sampledSpecular{ colors::White };
		const ColorRGB sampledGloss{ colors::White };

		// Normal mapping
		const Vector3 binormal{ Vector3::Cross(v.norm, v.tan) };
		const Matrix tangentSpaceAxis{ v.tan, binormal, v.norm, Vector3::Zero };
		const Vector3 normal{ m_RenderNormalMap ? (tangentSpaceAxis.TransformVector(2.f * sampledNormal.ToVector3() - Vector3::One).Normalized()) : v.norm };

		const ColorRGB diffuse{ m_LightingData.intensity * sampledColor / PI };

		const float observedArea{ Vector3::Dot(normal, -m_LightingData.direction) };
		if (observedArea < .0f) return colors::Black;

		// Calculate diffuse and specular lighting
		const float exp{ sampledGloss.r * m_LightingData.shininess };
		const ColorRGB specular{ (colors::White * sampledSpecular * powf(std::max(Vector3::Dot(Vector3::Reflect(-m_LightingData.direction, normal), v.view), .0f), exp)) };

		switch (m_ShadingMode)
		{
			using enum ShadingMode;
		case ObservedArea:
			return { observedArea, observedArea, observedArea };
		case Diffuse:
			return observedArea * diffuse;
		case Specular:
			return specular;
		case Combined:
			return (diffuse + specular + m_LightingData.ambient) * observedArea;
		}

		return colors::Black;
	}

	float SoftwareRasterizer::EdgeFunction(const Vector2& a, const Vector2& b, const Vector2& c)
	{
		return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
	}

	bool SoftwareRasterizer::IsOutsideViewFrustum(const Vertex_Out& v) const
	{
		return
			v.pos.x < .0f || v.pos.x > m_fWidth ||
			v.pos.y < .0f || v.pos.y > m_fHeight ||
			v.pos.z < .0f || v.pos.z > 1.f;
	}

	void SoftwareRasterizer::CalculateBoundingBox(const Vertex_Out& v0, const Vertex_Out& v1, const Vertex_Out& v2, Int2& min, Int2& max) const
	{
		// Compute the minimum and maximum x and y coordinates of the triangle.
		min.x = static_cast<int>(std::floor(std::max(.0f, std::min(v0.pos.x, std::min(v1.pos.x, v2.pos.x)))));
		min.y = static_cast<int>(std::floor(std::max(.0f, std::min(v0.pos.y, std::min(v1.pos.y, v2.pos.y)))));
		max.x = static_cast<int>(std::ceil(std::min(m_fWidth - 1.f, std::max(v0.pos.x, std::max(v1.pos.x, v2.pos.x)))));
		max.y = static_cast<int>(std::ceil(std::min(m_fHeight - 1.f, std::max(v0.pos.y, std::max(v1.pos.y, v2.pos.y)))));
	}

	void SoftwareRasterizer::VertexTransformationFunction(const std::vector<Mesh*>& pMeshes) const
	{
		// Compute half the width and height of the screen
		const float halfWidth{ m_fWidth * .5f };
		const float halfHeight{ m_fHeight * .5f };

		// Iterate over each mesh
		for (auto& mesh : pMeshes)
		{
			// Precompute the viewProjectionMatrix for this mesh.
			const Matrix& worldMatrix{ mesh->GetWorldMatrix() };
			const Matrix worldViewProjMatrix{ worldMatrix * mesh->GetViewProjMatrix() };

			// Use a reference to the mesh vertices and vertices_out vectors
			// to avoid repeated calls to the mesh accessor functions.
			const std::vector<Vertex_In>& vertices{ mesh->GetVertices() };
			std::vector<Vertex_Out>& verticesOut{ mesh->GetVerticesOut() };

			// If the vertices_out vector is empty, initialize it to be the same size as the vertices vector
			// and copy the vertex color, UV, normal, and tangent data into the vertices_out vector.
			if (verticesOut.empty())
			{
				verticesOut.reserve(vertices.size());
				for (const Vertex_In& vertex : vertices)
				{
					verticesOut.emplace_back(Vertex_Out{ {}, vertex.norm, vertex.tan, vertex.uv, vertex.col });
				}
			}

			// Iterate over the vertices of the mesh using a range-based for loop.
			for (int i{ 0 }; Vertex_Out & vertex : verticesOut)
			{
				// Transform the vertex position using the precomputed viewProjectionMatrix
				vertex.pos = worldViewProjMatrix.TransformPoint({ vertices[i].pos, 1.f });

				// Transform the normal and tangent vectors using the world matrix of the mesh
				vertex.norm = worldMatrix.TransformVector(vertices[i].norm);
				vertex.tan = worldMatrix.TransformVector(vertices[i].tan);

				// Compute the view direction vector as the difference between the transformed vertex position
				// and the origin of the camera.
				vertex.view = vertex.pos.GetXYZ();

				// Divide the x, y, and z coordinates of the position by the w coordinate.
				vertex.pos.x /= vertex.pos.w;
				vertex.pos.y /= vertex.pos.w;
				vertex.pos.z /= vertex.pos.w;

				// Transform the x and y coordinates of the position from normalized device coordinates
				// to screen space coordinates.
				vertex.pos.x = (vertex.pos.x + 1.f) * halfWidth;
				vertex.pos.y = (1.f - vertex.pos.y) * halfHeight;

				++i;
			}
		}
	}
}
