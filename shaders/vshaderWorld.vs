#version 330 core
layout(location = 0) in uvec2 vVertex;
flat out uvec2 gData;
void main() { gData = vVertex; }
