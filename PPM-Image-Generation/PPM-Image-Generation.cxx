#include <iostream>
#include <fstream>
using namespace std;

struct RGB {
    float r;
    float g;
    float b;
};

/**
 * <h1>CMPSC 457 - Homework 1</h1>
 * Program generates a w × h ppm file containing the resulting rasterized 
 * image when you set the color (R, G, B) of the pixel (w, h).
 */
int main(int argc, char* argv[]){
    // check for enough argments
    if(argc <= 3){
        printf("Error: You need more arguments\n");
        return 0;
    } 

    // get h, w values and ppm file from arguments
    string w_tmp = "";
    string h_tmp = "";
    string filename = "";
    for(int i = 1; i < argc; i++){
        for(int j = 0; j < strlen(argv[i]); j++){
            if(i == 1) w_tmp.push_back(argv[i][j]);
            else if(i == 2) h_tmp.push_back(argv[i][j]);
            else filename.push_back(argv[i][j]);
        }
    }
    int w = stoi(w_tmp);
    int h = stoi(h_tmp);

    // initalize the raster dynamically
    RGB** raster = new RGB*[h];
    for(int i = 0; i < h; i++)
        raster[i] = new RGB[w];

    // iterate values into raster
    for(int i = 0; i < h; i++){
        for(int j = 0; j < w; j++){
            int col = ((i & 0x08) == 0) ^ ((j & 0x08) == 0);
            raster[i][j].r = static_cast<float>(col);
            raster[i][j].g = static_cast<float>(col & 0x00);
            raster[i][j].b = static_cast<float>(col & 0x11);
        }
    }

    // make ppm img
    ofstream img(filename);
    img << "P3" << endl << w << ' ' << h << endl << "255" << endl;
    
    // iterate values into file
    for(int i = h - 1; i >= 0; i--){
        for(int j = 0; j < w; j++){
            img << raster[i][j].r * 255 << ' ' << raster[i][j].g * 255 << ' ' << raster[i][j].b * 255 << ' ';
        }
    }
    img.close();

    // free memory
    for(int i = 0; i < h; ++i)
        delete [] raster[i];
    delete [] raster;
    
    return 0;
}
