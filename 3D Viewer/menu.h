#pragma once
#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#include "camera.h"

class Menu {
private:


public:
	//Camera
	Camera *camera;
	//Model Path
	std::string Objectpath;
	//Model Material Colors
	glm::vec3 ambientMaterialColor;
	glm::vec3 diffuseMaterialColor;
	glm::vec3 specularMaterialColor;

	//Scene Lighting 
	glm::vec3 ambientLightingColor;
	glm::vec3 diffuseLightingColor;
	glm::vec3 specularLightingColor;

	//Model Transformations
	glm::vec3 translate;
	glm::vec3 rotate;
	float rotationAngle;
	float scale;

	float shininess;

	//Input
	float mouseSensitivity, zoomSensitivity, fovSensitivity;

	//Background
	glm::vec3 backgroundColor;

	bool wireFrame;

	Menu(Camera _camera) {
		//Default values
		this->camera = &_camera;
		//Material
		ambientMaterialColor = glm::vec3(.7f, .7f, .7f);
		diffuseMaterialColor = glm::vec3(.8f, .8f, .8f);
		specularMaterialColor = glm::vec3(.8f, .8f, .8f);

		//Scene
		ambientLightingColor = glm::vec3(.2f, .2f, .2f);
		diffuseLightingColor = glm::vec3(.7f, .7f, .7f);
		specularLightingColor = glm::vec3(.9f, .9f, .9f);

		//Transformations
		translate = glm::vec3(0.f, -1.5f, 0.f);
		rotate = glm::vec3(1.f, 0.f, 0.f);
		rotationAngle = -90.f;
		scale = .2f;

		shininess = 32.f;

		mouseSensitivity = .3f;
		zoomSensitivity = .5f;
		fovSensitivity = .4f;

		backgroundColor = glm::vec3(.2f, .2f, .2f);

		wireFrame = false;
	}

	void setSensitivities(float mouse_sensitivity, float zoom_sensitivity, float fov_sensitivity) {
		camera->setSensitivities(mouse_sensitivity, zoom_sensitivity, fov_sensitivity);
	}
};