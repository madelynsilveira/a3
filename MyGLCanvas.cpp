#define NUM_OPENGL_LIGHTS 8

#include "MyGLCanvas.h"

int Shape::m_segmentsX;
int Shape::m_segmentsY;

MyGLCanvas::MyGLCanvas(int x, int y, int w, int h, const char *l) : Fl_Gl_Window(x, y, w, h, l) {
	mode(FL_RGB | FL_ALPHA | FL_DEPTH | FL_DOUBLE);
	
	rotVec = glm::vec3(0.0f, 0.0f, 0.0f);
	eyePosition = glm::vec3(2.0f, 2.0f, 2.0f);

	pixelWidth = w;
	pixelHeight = h;

	isectOnly = 1;
	segmentsX = segmentsY = 10;
	scale = 1.0f;
	parser = NULL;

	objType = SHAPE_CUBE;
	cube = new Cube();
	cylinder = new Cylinder();
	cone = new Cone();
	sphere = new Sphere();
	shape = cube;

	shape->setSegments(segmentsX, segmentsY);

	camera = new Camera();
	camera->orientLookAt(eyePosition, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
}

MyGLCanvas::~MyGLCanvas() {
	delete cube;
	delete cylinder;
	delete cone;
	delete sphere;
	if (camera != NULL) {
		delete camera;
	}
	if (parser != NULL) {
		delete parser;
	}
	if (pixels != NULL) {
		delete pixels;
	}
}

void MyGLCanvas::renderShape(OBJ_TYPE type) {
	objType = type;
	switch (type) {
	case SHAPE_CUBE:
		shape = cube;
		break;
	case SHAPE_CYLINDER:
		shape = cylinder;
		break;
	case SHAPE_CONE:
		shape = cone;
		break;
	case SHAPE_SPHERE:
		shape = sphere;
		break;
	case SHAPE_SPECIAL1:
	default:
		shape = cube;
	}

	shape->setSegments(segmentsX, segmentsY);
	shape->draw();
}

void MyGLCanvas::loadSceneFile(const char* filenamePath) {
	if (parser != NULL) {
		delete parser;
	}
	parser = new SceneParser(filenamePath);

	bool success = parser->parse();
	cout << "success? " << success << endl;
	if (success == false) {
		delete parser;
		parser = NULL;
	}
	else {
		SceneCameraData cameraData;
		parser->getCameraData(cameraData);

		camera->reset();
		camera->setViewAngle(cameraData.heightAngle);
		if (cameraData.isDir == true) {
			camera->orientLookVec(cameraData.pos, cameraData.look, cameraData.up);
		}
		else {
			camera->orientLookAt(cameraData.pos, cameraData.lookAt, cameraData.up);
		}
	}
}


void MyGLCanvas::setSegments() {
	shape->setSegments(segmentsX, segmentsY);
}

void MyGLCanvas::draw() {
	if (!valid()) {  //this is called when the GL canvas is set up for the first time or when it is resized...
		printf("establishing GL context\n");

		glViewport(0, 0, w(), h());
		updateCamera(w(), h());

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	}
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (parser == NULL) {
		return;
	}

	if (pixels == NULL) {
		return;
	}

	//this just draws the "pixels" to the screen
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glDrawPixels(pixelWidth, pixelHeight, GL_RGB, GL_UNSIGNED_BYTE, pixels);
}

int MyGLCanvas::handle(int e) {
	//printf("Event was %s (%d)\n", fl_eventnames[e], e);
	switch (e) {
	case FL_KEYUP:
		printf("keyboard event: key pressed: %c\n", Fl::event_key());
		break;
	case FL_MOUSEWHEEL:
		break;
	}

	return Fl_Gl_Window::handle(e);
}

void MyGLCanvas::resize(int x, int y, int w, int h) {
	Fl_Gl_Window::resize(x, y, w, h);
	if (camera != NULL) {
		camera->setScreenSize(w, h);
	}
	puts("resize called");
}


void MyGLCanvas::updateCamera(int width, int height) {
	float xy_aspect;
	xy_aspect = (float)width / (float)height;

	camera->setScreenSize(width, height);
}

//Given the pixel (x, y) position, set its color to (r, g, b)
void MyGLCanvas::setpixel(GLubyte* buf, int x, int y, int r, int g, int b) {
	pixelWidth = camera->getScreenWidth();
	buf[(y*pixelWidth + x) * 3 + 0] = (GLubyte)r;
	buf[(y*pixelWidth + x) * 3 + 1] = (GLubyte)g;
	buf[(y*pixelWidth + x) * 3 + 2] = (GLubyte)b;
}

glm::vec3 MyGLCanvas::getEyePoint() {
    glm::mat4 inv = camera->getInverseModelViewMatrix();
    glm::vec4 eye = inv * glm::vec4(camera->getEyePoint(), 0);
	return glm::vec3(eye.x, eye.y, eye.z);
}

glm::vec3 MyGLCanvas::generateRay(int pixelX, int pixelY) {
	glm::vec3 lookAt = glm::vec3(-1.0f + 2.0f * ((float)pixelX / (float)camera->getScreenWidth()),
                                 -1.0f + 2.0f * ((float)pixelY / (float)camera->getScreenHeight()),
                                 -1.0f);

    glm::mat4 inv = camera->getInverseModelViewMatrix();
    glm::vec4 worldFPP = inv * glm::vec4(lookAt, 1);
    glm::vec4 worldEye = inv * glm::vec4(camera->getEyePoint(), 0);

    glm::vec4 dHat = glm::normalize(worldFPP - worldEye);

    // std::cout << "X: " << dHat.x << "," << "Y: " << dHat.y << ',' << "Z: " << dHat.z << std::endl;
    
	return glm::vec3(dHat.x, dHat.y, dHat.z);
}

glm::vec3 MyGLCanvas::getIsectPointWorldCoord(glm::vec3 eye, glm::vec3 ray, float t) {
	glm::vec3 p = eye + (t * ray);
	return p;
}

double MyGLCanvas::intersectSphere (glm::vec3 eyePointP, glm::vec3 rayV, glm::mat4 transformMatrix) {
    glm::mat4 transformInv = glm::inverse(transformMatrix);
    glm::vec3 eyePointObject = transformInv * glm::vec4(eyePointP, 1);
    glm::vec3 rayVObject = transformInv * glm::vec4(rayV, 1);

    double A = glm::dot(rayVObject, rayVObject);
    double B = 2 * glm::dot(eyePointObject, rayVObject);
    double C = glm::dot(eyePointObject, eyePointObject) - pow(0.5, 2);
    double discriminant = pow(B, 2) - (4.0 * A * C);

    if (discriminant < 0) {return -1;}
    else {
        double t1 = (-B + sqrt(discriminant)) / (2.0 * A);
        double t2 = (-B - sqrt(discriminant)) / (2.0 * A);

        if (t1 < 0) {return t2;}
        if (t2 < 0) {return t1;}
        return std::min(t1, t2);
    }
}

// void MyGLCanvas::traverse(SceneNode* root, list<ScenePrimitive*>& primitives, vector<SceneTransformation*>& scenetransformations, vector<pair<ScenePrimitive*, vector<SceneTransformation*>>>& my_scene_vals)
// {
// 	if (root == nullptr)
// 		return;

// 	for (SceneNode* node : root->children) {
// 		for (SceneTransformation* my_trans : node->transformations) {
// 			scenetransformations.push_back(my_trans);
// 		}
		
// 		traverse(node, primitives, scenetransformations, my_scene_vals);
		
// 		for (ScenePrimitive* my_prim : node->primitives) { // hit leaf node
// 			list<SceneTransformation*> curr_scene_trans = {};
// 			my_scene_vals.push_back(pair<ScenePrimitive*, vector<SceneTransformation*>>(my_prim, scenetransformations));
// 			primitives.push_back(my_prim);
// 		}
		
// 		for (SceneTransformation* my_trans : node->transformations) {
// 			scenetransformations.pop_back();
// 		}
// 	}
// }


void MyGLCanvas::renderScene() {
	cout << "render button clicked!" << endl;

	if (parser == NULL) {
		cout << "no scene loaded yet" << endl;
		return;
	}

	pixelWidth = w();
	pixelHeight = h();


	updateCamera(pixelWidth, pixelHeight);

	if (pixels != NULL) {
		delete pixels;
	}
	pixels = new GLubyte[pixelWidth  * pixelHeight * 3];
	memset(pixels, 0, pixelWidth  * pixelHeight * 3);

    //TODO: parse scene data tree
    if (primitives.empty()){
        SceneNode* root = parser->getRootNode();
    }

    glm::vec3 eyePoint = getEyePoint();
    float t;

	// PROCEDURE
    // - loop through all pixels
    // - - generate ray from eye point to pixel
    // - - loop through all objects
    // - - - see if any ray intersects any object by using object-specific intersection func
    
    
    for (int i = 0; i < pixelWidth; i++) {
		for (int j = 0; j < pixelHeight; j++) {
            //TODO: this is where your ray casting will happen!
            
            //generates ray to pixel (i,j)
            glm::vec3 rayV = generateRay(i, j);

            // loop over all objects
            for (ScenePrimitive* prim : primitives) {
                
                //TODO: get location of object?
                glm::vec3 sphereTransV(spherePosition[0], spherePosition[1], spherePosition[2]);

                if (prim->type == 3) {// sphere
                    // gets t value if ray from eye-point intersects object
                    t = intersectSphere(eyePoint, rayV, glm::translate(glm::mat4(1.0), sphereTransV));
                
                    if (t > 0) {
                        // gets intersection point on object
                        // TODO: what do we use 'isectPointWorldCoord' for?
                        glm::vec3 isectPointWorldCoord = getIsectPointWorldCoord(eyePoint, rayV, t);

                        // stores i,j in vector that holds all pixels that intersected an object
                        vector<int> hit = {i, j};
                        isect_pixels.push_back(hit);

                    }
                }
            }


			if (isectOnly == 1) {
				setpixel(pixels, i, j, 125, 125, 125);
            }
			else {
				setpixel(pixels, i, j, 255, 255, 255);
			}
		}
	}
	cout << "render complete" << endl;
	redraw();
}