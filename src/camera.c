#include <math.h>

void worldToScreenCoordinates(float worldCoordinates[2], float* screenCoordinateBuffer) {
    // TODO: implement
   
}

void screenToWorldCoordinates(float screenCoordinates[2], float* worldCoordinateBuffer) {
    // TODO: implement
}

void worldToHexCoordinates(float worldCoordinates[2], int* hexCoordinateBuffer) {
    // TODO: Define √3 as a constant, instead of computing it at runtime
    float q = (2.0f/3.0f) * sqrt(3) * worldCoordinates[0];
    float r = worldCoordinates[1] - q / 2.0f;
    hexCoordinateBuffer[0] = round(q);
    hexCoordinateBuffer[1] = round(r);
}

void hexToWorldCoordinates(int hexCoordinates[2], float* worldCoordinateBuffer) {
    worldCoordinateBuffer[0] = hexCoordinates[0] * sqrt(3) / 2.0f;
    worldCoordinateBuffer[1] = hexCoordinates[0] / 2.0f + hexCoordinates[1];
}