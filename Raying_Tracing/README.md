# Simple Ray Tracer

In this assignment, I created a simple ray tracer. This ray tracer generates a PPM formated image of three spheres.

<p align="center">
<img src="RayTracing500x500.png" width="600">
</p>


The following is the algorithm for ray tracing using Lambert's Law:
```C++
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
```

### Usage:

Use the following commands on your shell:
```bash
make template
make clean
```
to compile the program and to delete unnecessary files.

Use following commands to run the program:
```bash
./template nx ny outfile.ppm
```
where nx and ny are the width and height of the image to be generated and outfile.ppm is the file name.
