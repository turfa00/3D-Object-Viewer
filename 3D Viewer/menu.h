#pragma once
#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#include "camera.h"

class Menu {
private:
	//Camera
	Camera* camera;
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

public:
	// Model Path
	std::string& getObjectpath()  { return Objectpath; }
	void setObjectpath( std::string& path) { Objectpath = path; }

	// Model Material Colors
	glm::vec3& getAmbientMaterialColor()  { return ambientMaterialColor; }
	void setAmbientMaterialColor( glm::vec3& color) { ambientMaterialColor = color; }

	glm::vec3& getDiffuseMaterialColor()  { return diffuseMaterialColor; }
	void setDiffuseMaterialColor( glm::vec3& color) { diffuseMaterialColor = color; }

	glm::vec3& getSpecularMaterialColor()  { return specularMaterialColor; }
	void setSpecularMaterialColor( glm::vec3& color) { specularMaterialColor = color; }

	// Scene Lighting
	glm::vec3& getAmbientLightingColor()  { return ambientLightingColor; }
	void setAmbientLightingColor( glm::vec3& color) { ambientLightingColor = color; }

	glm::vec3& getDiffuseLightingColor()  { return diffuseLightingColor; }
	void setDiffuseLightingColor( glm::vec3& color) { diffuseLightingColor = color; }

	glm::vec3& getSpecularLightingColor()  { return specularLightingColor; }
	void setSpecularLightingColor( glm::vec3& color) { specularLightingColor = color; }

	// Model Transformations
	glm::vec3& getTranslate()  { return translate; }
	void setTranslate( glm::vec3& translation) { translate = translation; }

	glm::vec3& getRotate()  { return rotate; }
	void setRotate( glm::vec3& rotation) { rotate = rotation; }

	float getRotationAngle()  { return rotationAngle; }
	void setRotationAngle(float angle) { rotationAngle = angle; }

	float& getScale()  { return scale; }
	void setScale(float s) { scale = s; }

	// Shininess
	float& getShininess()  { return shininess; }
	void setShininess(float value) { shininess = value; }

	// Input Sensitivity
	float& getMouseSensitivity() { return mouseSensitivity; }
	void setMouseSensitivity(float sensitivity) { mouseSensitivity = sensitivity; }

	float& getZoomSensitivity() { return zoomSensitivity; }
	void setZoomSensitivity(float sensitivity) { zoomSensitivity = sensitivity; }

	float& getFovSensitivity() { return fovSensitivity; }
	void setFovSensitivity(float sensitivity) { fovSensitivity = sensitivity; }

	// Background
	glm::vec3& getBackgroundColor()  { return backgroundColor; }
	void setBackgroundColor( glm::vec3& color) { backgroundColor = color; }

	// Wireframe
	bool& isWireFrame()  { return wireFrame; }
	void setWireFrame(bool state) { wireFrame = state; }
	

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
};