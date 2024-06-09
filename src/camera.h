float cameraPosition[2] = {0.0f, 0.0f};
float cameraZoom = 1.0f;
float screenDimensions[2] = {640, 480};

void worldToScreenCoordinates(float worldCoordinates[2], float* screenCoordinateBuffer);
void screenToWorldCoordinates(float screenCoordinates[2], float* worldCoordinateBuffer);