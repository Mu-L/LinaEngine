
/*
Author: Inan Evin
www.inanevin.com

Copyright 2018 Inan Evin

Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions
and limitations under the License.

Class: Example1Level
Timestamp: 5/6/2019 9:22:56 PM

*/

#include "Examples/Example1/Levels/Example1Level.hpp"
#include "Rendering/RenderConstants.hpp"
#include "ECS/Systems/FreeLookSystem.hpp"
#include "ECS/Components/FreeLookComponent.hpp"
#include "ECS/Components/MeshRendererComponent.hpp"
#include "ECS/Components/CameraComponent.hpp"
#include "ECS/Components/TransformComponent.hpp"
#include "ECS/Components/LightComponent.hpp"
#include <entt/entity/registry.hpp>
#include "Rendering/RenderEngine.hpp"
#include "Core/Application.hpp"
#include "Rendering/Material.hpp"
#include "Rendering/Shader.hpp"

using namespace LinaEngine::Graphics;

ECSSystemList level1Systems;
FreeLookSystem* ecsFreeLookSystem;
CameraComponent cameraComponent;
TransformComponent cameraTransformComponent;
FreeLookComponent cameraFreeLookComponent;

ECSEntity camera;
ECSEntity directionalLight;
ECSEntity quad;
ECSEntity quad2;
ECSEntity quad3;
ECSEntity quad4;

Material* objectLitMaterial;
Material* objectUnlitMaterial;
Material* quadMaterial;
Material* cubemapReflectiveMaterial;
Material* floorMaterial;

TransformComponent object1Transform;
MeshRendererComponent object1Renderer;
MeshRendererComponent smallCubeRenderer;



Example1Level::~Example1Level()
{
	delete ecsFreeLookSystem;
}

void Example1Level::Install()
{
	LINA_CLIENT_WARN("Example level 1 install.");
}

void CreateSingleColorSkybox(RenderEngine* renderEngine)
{
	Material& mat = renderEngine->CreateMaterial("skyboxMaterial", Shaders::SKYBOX_SINGLECOLOR);
	mat.SetColor("material.color", Color::Red);
	renderEngine->SetSkyboxMaterial(mat);
}

void CreateGradientSkybox(RenderEngine* renderEngine)
{
	renderEngine->CreateMaterial("skyboxMaterial", Shaders::SKYBOX_GRADIENT);
	renderEngine->GetMaterial("skyboxMaterial").SetColor("material.startColor", Color::Green);
	renderEngine->GetMaterial("skyboxMaterial").SetColor("material.endColor", Color::White);
	renderEngine->SetSkyboxMaterial(renderEngine->GetMaterial("skyboxMaterial"));
}

void CreateProceduralSkybox(RenderEngine* renderEngine)
{
	Material& mat = renderEngine->CreateMaterial("skyboxMaterial", Shaders::SKYBOX_PROCEDURAL);
	mat.SetColor("material.startColor", Color::Black);
	mat.SetColor("material.endColor", Color::Gray);
	mat.SetVector3("material.sunDirection", Vector3(0.0f, -1.0f, 0.0f));
	renderEngine->SetSkyboxMaterial(mat);
}

void CreateCubemapSkybox(RenderEngine* renderEngine)
{
	Material& mat = renderEngine->CreateMaterial("skyboxMaterial", Shaders::SKYBOX_CUBEMAP);

	const std::string fp[6] = {
		"resources/textures/defaultSkybox/right.png",
		"resources/textures/defaultSkybox/left.png",
		"resources/textures/defaultSkybox/up.png",
		"resources/textures/defaultSkybox/down.png",
		"resources/textures/defaultSkybox/front.png",
		"resources/textures/defaultSkybox/back.png",
	};

	SamplerData data = SamplerData();
	data.minFilter = FILTER_NEAREST;
	Texture& t = renderEngine->CreateTexture(fp, PixelFormat::FORMAT_RGB, PixelFormat::FORMAT_RGB, true, false, data);
	mat.SetTexture(MC_TEXTURE2D_DIFFUSE, &t, TextureBindMode::BINDTEXTURE_CUBEMAP);
	renderEngine->SetSkyboxMaterial(mat);
}


Vector3 cubePositions[] = {
	Vector3(0.0f, 1.0f, -1.0),
	Vector3(3.0f, 1.0f, 1.0),
	Vector3(-2.0f, 1.0f, 2.0),
	Vector3(0.0f, -9.0f, 10.0f),
	Vector3(0.0f, -9.0f, 5.0f),
	Vector3(0.0f, -9.0f, 15.0f),
	Vector3(-5.0f, -4.0f, 10.0f),
	Vector3(0.0f, -4.0f, 5.0f),
	Vector3(5.0f, -4.0f, 15.0f),
};

Vector3 pointLightPositions[]
{
	Vector3(0.0f,  -7.0f,  8.0f),
	Vector3(-5.0f,  -6.5f,  13.5f),
	Vector3(5.0f,  -6.5f,  13.5f),
	Vector3(0.0f,  -7.5f,  13.5f),
};

Vector3 spotLightPositions[]
{
	Vector3(-2.0f, -6.0f, 13.0f)
};

int pLightSize = 0;
int cubeSize =3;
int sLightSize = 0;


ECSEntity cubeEntity;

void Example1Level::Initialize()
{

	LINA_CLIENT_WARN("Example level 1 initialize.");
	// Create, setup & assign skybox material.
	CreateProceduralSkybox(m_RenderEngine);


	camera.entity = m_ECS->reg.create();

	auto& camFreeLook = m_ECS->reg.emplace<FreeLookComponent>(camera.entity);
	auto& camTransform = m_ECS->reg.emplace<TransformComponent>(camera.entity);
	auto& camCamera = m_ECS->reg.emplace<CameraComponent>(camera.entity);
	camTransform.transform.location = Vector3::Zero;
	// Activate a camera component.
	camCamera.isActive = true;

	// Set the properties of our the free look component for the camera.
	camFreeLook.movementSpeedX = camFreeLook.movementSpeedZ = 12.0f;
	camFreeLook.rotationSpeedX = camFreeLook.rotationSpeedY = 3;

	SamplerData s;
	s.minFilter = SamplerFilter::FILTER_LINEAR_MIPMAP_LINEAR;
	s.maxFilter = SamplerFilter::FILTER_LINEAR;
	s.wrapS = SamplerWrapMode::WRAP_CLAMP;
	s.wrapT = SamplerWrapMode::WRAP_CLAMP;

	SamplerData s2;
	s2.minFilter = SamplerFilter::FILTER_LINEAR_MIPMAP_LINEAR;
	s2.maxFilter = SamplerFilter::FILTER_LINEAR;
	s2.wrapS = SamplerWrapMode::WRAP_REPEAT;
	s2.wrapT = SamplerWrapMode::WRAP_REPEAT;


	// Create texture for example mesh.
	Texture& crateTexture = m_RenderEngine->CreateTexture("resources/textures/box.png", PixelFormat::FORMAT_RGBA, PixelFormat::FORMAT_RGB, true, false, SamplerData());
	Texture& crateSpecTexture = m_RenderEngine->CreateTexture("resources/textures/boxSpecular.png", PixelFormat::FORMAT_RGBA, PixelFormat::FORMAT_SRGB, true, false, SamplerData());
	Texture& window = m_RenderEngine->CreateTexture("resources/textures/window.png", PixelFormat::FORMAT_RGBA, PixelFormat::FORMAT_RGBA, true, false, s);
	Texture& wood = m_RenderEngine->CreateTexture("resources/textures/wood.png", PixelFormat::FORMAT_RGBA, PixelFormat::FORMAT_SRGBA, true, false, s2);
	//Texture& cubemap = m_RenderEngine->GetTexture("resources/textures/defaultSkybox/right.png");


	// Load example mesh.
	Mesh& cubeMesh = m_RenderEngine->CreateMesh("resources/meshes/cube.obj");

	// Create material for example mesh.
	objectLitMaterial = &m_RenderEngine->CreateMaterial("object1Material", Shaders::STANDARD_LIT);
	objectUnlitMaterial = &m_RenderEngine->CreateMaterial("object2Material", Shaders::STANDARD_UNLIT);
	quadMaterial = &m_RenderEngine->CreateMaterial("quadMaterial", Shaders::STANDARD_LIT);
	floorMaterial = &m_RenderEngine->CreateMaterial("floor", Shaders::STANDARD_LIT);
	//cubemapReflectiveMaterial = &m_RenderEngine->CreateMaterial("cubemapReflective", Shaders::CUBEMAP_REFLECTIVE);


	objectLitMaterial->SetTexture(MC_TEXTURE2D_DIFFUSE, &crateTexture);
	//objectLitMaterial->SetTexture(MC_TEXTURE2D_SPECULAR, &crateSpecTexture);
	objectLitMaterial->SetSurfaceType(MaterialSurfaceType::Opaque);


	objectUnlitMaterial->SetColor(MC_OBJECTCOLORPROPERTY, Color(1, 1, 1));

	//cubemapReflectiveMaterial->SetTexture(UF_SKYBOXTEXTURE, &cubemap, 0);

	quadMaterial->SetTexture(MC_TEXTURE2D_DIFFUSE, &window);
	quadMaterial->SetSurfaceType(MaterialSurfaceType::Transparent);

	floorMaterial->SetTexture(MC_TEXTURE2D_DIFFUSE, &wood);
	floorMaterial->SetVector2(MC_TILING, Vector2(10, 10));

	object1Renderer.mesh = &cubeMesh;
	object1Renderer.material = objectLitMaterial;
	smallCubeRenderer.mesh = &cubeMesh;
	smallCubeRenderer.material = objectUnlitMaterial;




	directionalLight.entity = m_ECS->reg.create();
	auto& dirLight = m_ECS->reg.emplace<DirectionalLightComponent>(directionalLight.entity);
	dirLight.color = Color(0.4f,0.4,0.4f);
	dirLight.direction = Vector3(0, -0.5f, 1);

	ECSEntity floor;
	floor.entity = m_ECS->reg.create();
	MeshRendererComponent mr;
	mr.mesh = &m_RenderEngine->GetPrimitive(Primitives::PLANE);
	mr.material = floorMaterial;

	object1Transform.transform.location = Vector3(0, 0, 0);
	object1Transform.transform.scale = Vector3(40.0f);
	m_ECS->reg.emplace<TransformComponent>(floor.entity, object1Transform);
	m_ECS->reg.emplace<MeshRendererComponent>(floor.entity, mr);



	for (int i = 0; i < cubeSize; i++)
	{
		ECSEntity entity;
		object1Renderer.material = objectLitMaterial;

		object1Transform.transform.rotation = Quaternion::Euler(Vector3::Zero);
		object1Transform.transform.location = cubePositions[i];
		object1Transform.transform.scale = Vector3::One;
		//object1Transform.transform.location = Vector3(Math::RandF(-100, 100), Math::RandF(-100, 100), Math::RandF(-100, 100));
		entity.entity = m_ECS->reg.create();
		m_ECS->reg.emplace<TransformComponent>(entity.entity, object1Transform);
		m_ECS->reg.emplace<MeshRendererComponent>(entity.entity, object1Renderer);
	}






	for (int i = 0; i < pLightSize; i++)
	{
		smallCubeRenderer.material = objectUnlitMaterial;

		ECSEntity entity;
		object1Transform.transform.location = pointLightPositions[i];
		object1Transform.transform.scale = 1;
		entity.entity = m_ECS->reg.create();
		m_ECS->reg.emplace<TransformComponent>(entity.entity, object1Transform);

		auto& pLight1 = m_ECS->reg.emplace<PointLightComponent>(entity.entity);


		pLight1.color = Color(0.45f, 0.45f, 0.45f);
		pLight1.distance = 100;


		ECSEntity visuals;
		visuals.entity = m_ECS->reg.create();
		object1Transform.transform.scale = 0.1f;
		m_ECS->reg.emplace<TransformComponent>(visuals.entity, object1Transform);
		m_ECS->reg.emplace<MeshRendererComponent>(visuals.entity, smallCubeRenderer);
	}



	for (int i = 0; i < sLightSize; i++)
	{
		smallCubeRenderer.material = objectUnlitMaterial;

		ECSEntity entity;
		object1Transform.transform.location = (spotLightPositions[i]);
		entity.entity = m_ECS->reg.create();
		object1Transform.transform.rotation = (Quaternion::Euler(-25, 40, 0));

		m_ECS->reg.emplace<TransformComponent>(entity.entity, object1Transform);

		auto& sLight1 = m_ECS->reg.emplace<SpotLightComponent>(entity.entity);


		sLight1.color = Color(0.05f, 0.05f, 0.05f);
		sLight1.distance = 150;
		sLight1.cutOff = Math::Cos(Math::ToRadians(12.5f));
		sLight1.outerCutOff = Math::Cos(Math::ToRadians(15.5f));


		ECSEntity visuals;
		visuals.entity = m_ECS->reg.create();
		object1Transform.transform.scale = (0.1f);
		m_ECS->reg.emplace<TransformComponent>(visuals.entity, object1Transform);
		m_ECS->reg.emplace<MeshRendererComponent>(visuals.entity, smallCubeRenderer);
	}
	object1Transform.transform.rotation = (Quaternion::Euler(Vector3::Zero));

	object1Transform.transform.scale = (Vector3(1));
	object1Transform.transform.location = (Vector3(0.7f, 0, 5.5f));
	object1Transform.transform.Rotate(-90, 0, 0);



//quad.entity = m_ECS->reg.create();
//m_ECS->reg.emplace<TransformComponent>(quad.entity, object1Transform);
//MeshRendererComponent quadR;
//quadR.material = quadMaterial;
//quadR.mesh = &m_RenderEngine->GetPrimitive(Primitives::PLANE);
//m_ECS->reg.emplace<MeshRendererComponent>(quad.entity, quadR);
//
//object1Transform.transform.location = (Vector3(-2.5f, 0, 11.0f));
//quad2.entity = m_ECS->reg.create();
//m_ECS->reg.emplace<TransformComponent>(quad2.entity, object1Transform);
//m_ECS->reg.emplace<MeshRendererComponent>(quad2.entity, quadR);
//
//object1Transform.transform.location = (Vector3(-2.5f, 0, 7.5f));
//quad3.entity = m_ECS->reg.create();
//m_ECS->reg.emplace<TransformComponent>(quad3.entity, object1Transform);
//m_ECS->reg.emplace<MeshRendererComponent>(quad3.entity, quadR);
//
//object1Transform.transform.location = (Vector3(-2.1f, 0, 7.0f));
//quad4.entity = m_ECS->reg.create();
//m_ECS->reg.emplace<TransformComponent>(quad4.entity, object1Transform);
//m_ECS->reg.emplace<MeshRendererComponent>(quad4.entity, quadR);
//
	// Create the free look system & push it.
	ecsFreeLookSystem = new FreeLookSystem();
	ecsFreeLookSystem->Construct(*m_ECS, *m_InputEngine);
	level1Systems.AddSystem(*ecsFreeLookSystem);


}

static float t = 0.0f;

void Example1Level::Tick(float delta)
{
	// Update the systems in this level.
	level1Systems.UpdateSystems(delta);
	t += delta;
	//
	//TransformComponent& cube = m_ECS->reg.get<TransformComponent>(quad.entity);
	//cube.transform.location = Vector3(0, 0, 5);
	//cube.transform.Rotate(t*12, 0,0);
	//cube.transform.scale = Vector3(1);
		//	cube.transform.rotation = q;
			//TransformComponent& tSpotLight = m_ECS->reg.get<TransformComponent>(spotLight.entity);
		//	TransformComponent& tCamera = m_ECS->reg.get<TransformComponent>(camera.entity);

		//tSpotLight.transform.SetLocation(tCamera.transform.location);
		//tSpotLight.transform.SetRotation(tCamera.transform.rotation);
		//
		//TransformComponent tc = m_ECS->reg.get<TransformComponent>(object1.entity);
		//t += delta;
		//tc.transform.SetLocation(Vector3F(Math::Sin(t) * 5, 0, 10));
		//	
		//m_ECS->reg.replace<TransformComponent>(object1.entity, tc);
}