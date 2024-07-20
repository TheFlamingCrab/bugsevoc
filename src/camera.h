// MUST BE A MULTIPLE OF ITS BIGGEST ELEMENT
// BIGGEST ELEMENTS MUST BE AT AN OFFSET THAT IS A MULTIPLE OF ITS SIZE, SO PUT BIGGEST ELEMENTS FIRST

struct {
    float position[2];
    float zoom;
    float _padding;
} cameraUniform;

// ADD THESE LATER
/*    float zoom = 1.0f;
    float dimensions[2] = {640, 480}; */

void worldToScreenCoordinates(float worldCoordinates[2], float* screenCoordinateBuffer);
void screenToWorldCoordinates(float screenCoordinates[2], float* worldCoordinateBuffer);

void worldToHexCoordinates(float worldCoordinates[2], int* hexCoordinateBuffer);
void hexToWorldCoordinates(int hexCoordinates[2], float* worldCoordinateBuffer);