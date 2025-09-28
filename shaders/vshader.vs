#version 330 core

layout(location = 0) in uint vVertex;

const uint REF_BLOCK_X  = (6u) | (6u << 4u) | (6u << 8u) | (6u << 12u) | (6u << 16u) | (6u << 20u);
const uint REF_BLOCK_Y  = (2u) | (2u << 4u) | (2u << 8u) | (2u << 12u) | (2u << 16u) | (2u << 20u);
const uint DIRT_BLOCK_X  = (2u) | (2u << 4u) | (2u << 8u) | (2u << 12u) | (2u << 16u) | (2u << 20u);
const uint DIRT_BLOCK_Y  = (0u) | (0u << 4u) | (0u << 8u) | (0u << 12u) | (0u << 16u) | (0u << 20u);
const uint STONE_BLOCK_X  = (1u) | (1u << 4u) | (1u << 8u) | (1u << 12u) | (1u << 16u) | (1u << 20u);
const uint STONE_BLOCK_Y  = (0u) | (0u << 4u) | (0u << 8u) | (0u << 12u) | (0u << 16u) | (0u << 20u);
const uint TREE_BLOCK_X  = (4u) | (4u << 4u) | (4u << 8u) | (4u << 12u) | (5u << 16u) | (5u << 20u);
const uint TREE_BLOCK_Y  = (1u) | (1u << 4u) | (1u << 8u) | (1u << 12u) | (1u << 16u) | (1u << 20u);
const uint LEAF_BLOCK_X  = (5u) | (5u << 4u) | (5u << 8u) | (5u << 12u) | (5u << 16u) | (5u << 20u);
const uint LEAF_BLOCK_Y  = (3u) | (3u << 4u) | (3u << 8u) | (3u << 12u) | (3u << 16u) | (3u << 20u);
const uint GRASS_BLOCK_X = (3u) | (3u << 4u) | (3u << 8u) | (3u << 12u) | (0u << 16u) | (2u << 20u);
const uint GRASS_BLOCK_Y = (0u) | (0u << 4u) | (0u << 8u) | (0u << 12u) | (0u << 16u) | (0u << 20u);
const uint IRON_BLOCK_X  = (6u) | (6u << 4u) | (6u << 8u) | (6u << 12u) | (6u << 16u) | (6u << 20u);
const uint IRON_BLOCK_Y  = (1u) | (1u << 4u) | (1u << 8u) | (1u << 12u) | (1u << 16u) | (1u << 20u);
const uint WOOD_BLOCK_X  = (4u) | (4u << 4u) | (4u << 8u) | (4u << 12u) | (4u << 16u) | (4u << 20u);
const uint WOOD_BLOCK_Y  = (0u) | (0u << 4u) | (0u << 8u) | (0u << 12u) | (0u << 16u) | (0u << 20u);
const uint WATER_BLOCK_X  = (14u) | (14u << 4u) | (14u << 8u) | (14u << 12u) | (14u << 16u) | (14u << 20u);
const uint WATER_BLOCK_Y  = (12u) | (12u << 4u) | (12u << 8u) | (12u << 12u) | (12u << 16u) | (12u << 20u);
const uint GRAVEL_BLOCK_X  = (3u) | (3u << 4u) | (3u << 8u) | (3u << 12u) | (3u << 16u) | (3u << 20u);
const uint GRAVEL_BLOCK_Y  = (1u) | (1u << 4u) | (1u << 8u) | (1u << 12u) | (1u << 16u) | (1u << 20u);
const uint SAND_BLOCK_X  = (2u) | (2u << 4u) | (2u << 8u) | (2u << 12u) | (2u << 16u) | (2u << 20u);
const uint SAND_BLOCK_Y  = (1u) | (1u << 4u) | (1u << 8u) | (1u << 12u) | (1u << 16u) | (1u << 20u);
const uint BEDROCK_BLOCK_X  = (1u) | (1u << 4u) | (1u << 8u) | (1u << 12u) | (1u << 16u) | (1u << 20u);
const uint BEDROCK_BLOCK_Y  = (1u) | (1u << 4u) | (1u << 8u) | (1u << 12u) | (1u << 16u) | (1u << 20u);

// Pack block arrays as const arrays (faster, not re-written per-vertex).
const uint blocks_X[12] = uint[12](REF_BLOCK_X, DIRT_BLOCK_X, STONE_BLOCK_X, TREE_BLOCK_X, LEAF_BLOCK_X, GRASS_BLOCK_X, IRON_BLOCK_X, WOOD_BLOCK_X, WATER_BLOCK_X, GRAVEL_BLOCK_X, SAND_BLOCK_X, BEDROCK_BLOCK_X);
const uint blocks_Y[12] = uint[12](REF_BLOCK_Y, DIRT_BLOCK_Y, STONE_BLOCK_Y, TREE_BLOCK_Y, LEAF_BLOCK_Y, GRASS_BLOCK_Y, IRON_BLOCK_Y, WOOD_BLOCK_Y, WATER_BLOCK_Y, GRAVEL_BLOCK_Y, SAND_BLOCK_Y, BEDROCK_BLOCK_Y);

uniform float side;
uniform vec3 chunkpos;
uniform mat4 vModel;
uniform mat4 vView;
uniform mat4 vProjection;

// uniform vec4 vColor;

out vec2 tile;
out float isoscale;
out vec3 NormalDir;
out vec2 TexCoord; 
out float aoFactor;

vec3 Center(vec3 pos, uint centeroff){
	vec3 center = pos;

	if((centeroff & 1u) == 1u){
		center.z += side / 2;
	} else{
		center.z -= side / 2;
	}

	if((centeroff & 2u) == 2u){
		center.y += side / 2;
	} else{
		center.y -= side / 2;
	}

	if((centeroff & 4u) == 4u){
		center.x += side / 2;
	} else{
		center.x -= side / 2;
	}

	return center;
}

ivec3 Normal(uint normaldir){
	if(normaldir == 0u){
		return ivec3(0, 0, -1);
	}

	if(normaldir == 1u){
		return ivec3(0, 0, 1);
	}

	if(normaldir == 2u){
		return ivec3(-1, 0, 0);
	}

	if(normaldir == 3u){
		return ivec3(1, 0, 0);
	}

	if(normaldir == 4u){
		return ivec3(0, 1, 0);
	}

	if(normaldir == 5u){
		return ivec3(0, -1, 0);
	}
}

void setup_textures(ivec3 _NormalDir, uint vVertex, uint centeroff){
	// Block Faces
	uint blktype = (vVertex >> 24u) & 31u;
	uint frontx = (blocks_X[blktype] >> 8u) & 15u;
	uint fronty = (blocks_Y[blktype] >> 8u) & 15u;
	uint backx = (blocks_X[blktype] >> 0u) & 15u;
	uint backy = (blocks_Y[blktype] >> 0u) & 15u;
	uint topx = (blocks_X[blktype] >> 16u) & 15u;
	uint topy = (blocks_Y[blktype] >> 16u) & 15u;
	uint bottomx = (blocks_X[blktype] >> 20u) & 15u;
	uint bottomy = (blocks_Y[blktype] >> 20u) & 15u;
	uint leftx = (blocks_X[blktype] >> 4u) & 15u;
	uint lefty = (blocks_Y[blktype] >> 4u) & 15u;
	uint rightx = (blocks_X[blktype] >> 12u) & 15u;
	uint righty = (blocks_Y[blktype] >> 12u) & 15u;

	vec2 uv;
	if (_NormalDir.z != 0) {
		// Z face → use (x,y)
		uv = vec2((centeroff & 4u) >> 2, (centeroff & 2u) >> 1);
		if(_NormalDir.z > 0){ // Front Face
			tile.x = frontx;
			tile.y = fronty;
		} else { // Back Face
			tile.x = backx;
			tile.y = backy;
			uv.x = 1.0 - uv.x;
		}
	}
	else if (_NormalDir.y != 0) {
		// Y face → use (x,z)
		uv = vec2((centeroff & 4u) >> 2, (centeroff & 1u));
		if(_NormalDir.y > 0){ // Top Face
			tile.x = topx;
			tile.y = topy;
		} else { // Bottom Face
			tile.x = bottomx;
			tile.y = bottomy;
			uv.x = 1.0 - uv.x;
		}
	}
	else {
		// X face → use (z,y)
		uv = vec2((centeroff & 1u), (centeroff & 2u) >> 1);
		if(_NormalDir.x < 0){ // Left Face
			tile.x = leftx;
			tile.y = lefty;
		} else { // Right Face	
			tile.x = rightx;
			tile.y = righty;
			uv.x = 1.0 - uv.x;
		}
	}
	TexCoord = uv;
}

void main() {
	uint positionX = (vVertex)        & 63u;
	uint positionY = (vVertex >> 6u)  & 63u;
	uint positionZ = (vVertex >> 12u) & 63u;
	uint centeroff = (vVertex >> 18u) & 7u;
	uint normaldir = (vVertex >> 21u) & 7u;	
	uint ac = (vVertex >> 29u) & 7u; // 3 bits: x, z, diag (example layout)

	uint side1 = ((ac & 1u) != 0u) ? 1u : 0u;   // bit 0
	uint diag  = ((ac & 2u) != 0u) ? 1u : 0u;   // bit 1
	uint side2 = ((ac & 4u) != 0u) ? 1u : 0u;   // bit 2

	uint occ;
	if ((side1 != 0u) && (side2 != 0u)) {
		// both neighbors present → darkest (ignore diagonal)
		occ = 3u;
	} else {
		occ = side1 + side2 + diag;
	}
	aoFactor = 1.0f - (occ * 0.25f);  // 1.0, 0.75, 0.5, 0.25
	vec3 pos = vec3(side * positionX, side * positionY, side * positionZ);
	vec3 centercord = Center(pos, centeroff);
	gl_Position = vProjection * vView * vModel * vec4(pos + chunkpos, 1.0);

	ivec3 _NormalDir = Normal(normaldir);

	NormalDir = _NormalDir;

	setup_textures(_NormalDir, vVertex, centeroff);
	// figure out which 2D coords to use based on face
	if (_NormalDir.z != 0) {
		isoscale = 0.980;
	}
	else if (_NormalDir.y != 0) {
		isoscale = 0.800;
	}
	else {
		isoscale = 0.608;
	}
}
