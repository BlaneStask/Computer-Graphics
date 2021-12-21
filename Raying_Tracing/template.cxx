#include <cmath>
#include <limits>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cfloat>
#include <vector>
#include <glm/glm.hpp>

using namespace std;

using vec3 = glm::dvec3;

struct Ray
{
    // origin and direction of this ray
    vec3 o, d; 

    // arg d should always be normalized vector
    Ray (vec3 o, vec3 d) : o(o), d(d) {} 
};

// types of the surface
// - DIFFuse, SPECular, REREective
enum Refl_t { DIFF, SPEC, REFR };

// small constant
const double eps = 1e-4;

struct Sphere
{
    double r;      // radius
    vec3 p;        // position (center)
    vec3 e;        // emission
    vec3 c;        // color
    Refl_t refl;   // reflection type (DIFFuse, SPECular, REFRactive)

    Sphere(double r, vec3 p, vec3 e, vec3 c, Refl_t refl)
        : r{r}, p{p}, e{e}, c{c}, refl{refl} {}

    double intersect(const Ray& ray) const
    {
	// intersects with t > eps, return t
	// otherwise, return 0
	    
	vec3 op = ray.o - p;
	double b = glm::dot(op, 2.0 * ray.d);
        double a = glm::dot(ray.d, ray.d);
        double c = glm::dot(op, op) - (r * r);
        double t =  (-b + sqrt((b * b) - (4.0 * a * c))) / (2.0 * a);
        double t2 = (-b - sqrt((b * b) - (4.0 * a * c))) / (2.0 * a);
	if (t > eps && t2 > eps) {
		return min(t, t2);
	}
	else if (t <= eps && t2 <= eps) return 0.0;
	else if (t > eps) return t;
	else return t2;
    }


    vec3 normal(vec3& v)
    {
	return glm::normalize(v - p);
    }
};

vector<Sphere> spheres = {
    Sphere(200, vec3(  0, -300, -1200), vec3(), vec3(.8, .8, .8), DIFF),
    Sphere(200, vec3(-80, -150, -1200), vec3(), vec3(.7, .7, .7), DIFF),
    Sphere(200, vec3( 70, -100, -1200), vec3(), vec3(.9, .9, .9), DIFF)
};

vec3 eye(0, 0, 200);      // camera position
vec3 light(0, 0, 200);    // light source position

bool hit(const Ray& ray, double& t, int& surface_idx)
{
    double valt = INT_MAX;
    bool hit = false;
    for (int i = 0; i < 3; i++) {
    	double tmp = spheres[i].intersect(ray);
	if (tmp > eps && tmp < valt) {
   	    hit = true;
	    valt = tmp;
	    t = valt;
	    surface_idx = i;
	}
    }

    return hit;
}

// Calculating the intensity using Lambert's law
double lambert(int surface_idx, Ray& ray, double t)
{
    vec3 Pn = eye + (t * ray.d);
    vec3 n_hat = spheres[surface_idx].normal(Pn);
    vec3 l_hat = glm::normalize(light - Pn);
    double lambC = glm::dot(n_hat, l_hat);
    if (lambC > 0) return lambC;
    else return 0;
}


// Calculating the color of the ray
vec3 ray_color(Ray& ray)
{
    double t;
    int surface_idx;

    bool is_hit = hit(ray, t, surface_idx);
    if (is_hit) {
        return spheres[surface_idx].c * lambert(surface_idx, ray, t);
    }
    else {
        return vec3{0.0, 0.0, 0.0};
    }
}

// Simple ray tracer
void tracer(int nx, int ny, int d, double theta, ofstream& fout)
{
    double h = (2.0 * d * (tan(theta / 2.0)));
    double w =  (nx / (double) ny) * h;
    double scalef = w / (double) nx;

    fout << "P3\n" << nx << " " << ny << "\n" << "255\n";

    for (int j = 0; j < ny; j++) {
        for (int i = 0; i < nx; i++) {
            vec3 point;
		
	    // transform: translate, scale
            point[0] = ((double) i + (-nx / 2.0)) * scalef;
            point[1] = ((double) j + (-ny / 2.0)) * scalef;
            point[2] = d - d;
		
            vec3 worldn = glm::normalize(point - eye);
            Ray r = Ray(eye, worldn);
            vec3 color = ray_color(r);
		
            fout << (int) (color[0] * 255) << " " << (int) (color[1] * 255) << " " << (int) (color[2] * 255) << " ";

      }
      fout<<"\n";
    }
}


int main(int argc, char* argv[])
{
    if (argc != 4) {
	cerr << "Usage:  template nx ny outfile.ppm";
	exit(1);
    }

    int nx = std::stoi(argv[1], nullptr);
    int ny = std::stoi(argv[2], nullptr);
    char *fname = argv[3];

    ofstream fout(fname);
    if (!fout) {
        cerr << "tracer: cannot open input file " << fname << endl;
        exit(1);
    }

    // trace the ray to generate nx x ny image using
    //   the virtual film placed at the distance of 200 in z-axis (negative z direction) from the eye
    //   vfov of 120
    tracer(nx, ny, 200, 120, fout);
    fout.close();

    return 0;
}
