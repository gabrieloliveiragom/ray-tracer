#include "Camera.hpp"
#include <math.h>

Camera::Camera(){}

Camera::Camera(Vector3D e, Vector3D t, Vector3D u, double w, double h){
	eye = e;
	target = t;
	up = u.normalized();
	width = w;
	height = h;

	right = (target - eye) * up;
	right.normalize();
	pixelsW = 3000;
	pixelsH = 3000;

	image.resize(pixelsW, pixelsH);
}

void Camera::getImageFromScene(Scene& scene, std::vector<Light>& lights){
	Vector3D top = target + ((height / 2.0) * up) - ((width / 2.0) * right);

	Vector3D rayDirection;

	Vector3D stepRight = ((1.0 * width) / pixelsW) * right;
	Vector3D stepDown = ((-1.0 * height) / pixelsH) * up;

	Ray ray(eye, rayDirection);

	for(int i = 0; i < pixelsH; i++){
		rayDirection = top - eye + (i * stepDown);

		for(int j = 0; j < pixelsW; j++, rayDirection += stepRight){
			ray.setDirection(rayDirection);
			
			double closestSphereDistance = -1.0;
			bool intersected = false;
			Sphere closestSphere;
			Vector3D closestIntersection;

			for(Sphere s : scene.set){
				if(ray.intersect(s)){
					Vector3D intersection = ray.intersection(s);
					
					if(!intersected || closestSphereDistance > eye.distance(intersection)){
						closestSphereDistance = eye.distance(intersection);
						closestSphere = s;
						closestIntersection = intersection;
						intersected = true;
					}
				}
			}

			if(intersected){
				// image.setPixelColor(i, j, sphereColor(closestSphere, closestIntersection, scene, lights));
				image.setPixelColor(i, j, sphereColorShadow(closestSphere, closestIntersection, scene, lights));
			}

		}
	}
}

Color Camera::sphereColor(Sphere& sphere, Vector3D& point, Scene& scene, std::vector<Light>& lights){
	Color ambientLight(0.5);

	Color totalIlumination = sphere.ambient * ambientLight;


	for(Light l : lights){
		totalIlumination += sphereIlumination(sphere, point, l);
	}

	return totalIlumination;
}

Color Camera::sphereColorShadow(Sphere& sphere, Vector3D& point, Scene& scene, std::vector<Light>& lights){
	Color ambientLight(0.5);

	Color totalIlumination = sphere.ambient * ambientLight;


	for(Light l : lights){
		
		Ray ray(l.source, point - l.source);
		bool shadow = false;

		for(Sphere s : scene.set){
			if(sphere != s && ray.intersect(s)){
				Vector3D intersection = ray.intersection(s);
				if(l.source.distance(intersection) < l.source.distance(point)){
					shadow = true;
					break;
				}
			}
		}

		if(!shadow)
			totalIlumination += sphereIlumination(sphere, point, l);
	}

	return totalIlumination;
}

Color Camera::sphereIlumination(Sphere& sphere, Vector3D& point, Light& light){
	// Phong Reflection Model

	Vector3D N = point - sphere.center;
	N.normalize();

	Vector3D L = light.source - point;
	L.normalize();

	Vector3D V = eye - point;
	V.normalize();

	Vector3D R = 2 * L.scalar(N) * N - L;

	Color ilumination = (L.scalar(N) * (sphere.diffuse * light.diffuse)) + (pow(R.scalar(V), sphere.shininess) * (sphere.specular * light.specular));

	return ilumination;
}

std::ostream& operator<<(std::ostream& out, const Camera& c){
	out << "Camera[" << c.eye << "; " << c.target << "; " << c.up << "; " << c.width << "x" << c.height << "]";
	return out;
}
