#pragma once

#ifndef MYGLCANVAS_H
#define MYGLCANVAS_H

#include <FL/gl.h>
#include <FL/glut.h>
#include <FL/glu.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <time.h>
#include <iostream>

#include "Shape.h"
#include "Cube.h"
#include "Cylinder.h"
#include "Cone.h"
#include "Sphere.h"

#include "Camera.h"
#include "scene/SceneParser.h"

class MyGLCanvas : public Fl_Gl_Window {
public:
	glm::vec3 rotVec;
	glm::vec3 eyePosition;
	GLubyte* pixels = NULL;

	int isectOnly;
	int segmentsX, segmentsY;
	float scale;

	OBJ_TYPE objType;
	Cube* cube;
	Cylinder* cylinder;
	Cone* cone;
	Sphere* sphere;
	Shape* shape;

	Camera* camera;
	SceneParser* parser;

    // what I added
    vector<ScenePrimitive*> primitives;
    vector<vector<int>> isect_pixels;

	MyGLCanvas(int x, int y, int w, int h, const char *l = 0);
	~MyGLCanvas();
	void renderShape(OBJ_TYPE type);
	void setSegments();
	void loadSceneFile(const char* filenamePath);
	void renderScene();

private:
	void setpixel(GLubyte* buf, int x, int y, int r, int g, int b);

    glm::vec3 generateRay(int pixelX, int pixelY);
	glm::vec3 getEyePoint();
	glm::vec3 getIsectPointWorldCoord(glm::vec3 eye, glm::vec3 ray, float t);
	double intersectSphere (glm::vec3 eyePointP, glm::vec3 rayV, glm::mat4 transformMatrix);

	void draw();

	int handle(int);
	void resize(int x, int y, int w, int h);
	void updateCamera(int width, int height);

	int pixelWidth, pixelHeight;
};

#endif // !MYGLCANVAS_H