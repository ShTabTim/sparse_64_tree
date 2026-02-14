#ifndef SHADERS_SOURCE_HPP
#define SHADERS_SOURCE_HPP
// Этот файл сгенерирован автоматически. Не редактировать вручную.

const char* shader_source_comp = R"(#version 430 core

//#define USE_EMPTY

layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;
layout(rgba32f, binding = 0) uniform image2D imgOutput;

struct tree_node {
    uint leaf_mask0;
    uint leaf_mask1;
#ifdef USE_EMPTY
    uint empty_mask0;
    uint empty_mask1;
#endif//USE_EMPTY
    uint child[64];
};

layout(std430, binding = 0) buffer TreeBuffer {
    tree_node voxels[];
};

uniform vec3 pos;
uniform vec4 ang;

uniform vec2 pyramid;//wh, n
uniform float rand;
uniform float time;

//re = w
mat3 quat(in vec4 q) {
    return mat3(
        q.w*q.w+q.x*q.x-q.y*q.y-q.z*q.z, 2.0*(q.x*q.y-q.z*q.w), 2.0*(q.x*q.z+q.y*q.w),
        2.0*(q.x*q.y+q.z*q.w), q.w*q.w-q.x*q.x+q.y*q.y-q.z*q.z, 2.0*(q.y*q.z-q.x*q.w),
        2.0*(q.x*q.z-q.y*q.w), 2.0*(q.y*q.z+q.x*q.w), q.w*q.w-q.x*q.x-q.y*q.y+q.z*q.z
    );
}

struct path_hit {
    vec3 neg_norm;
    vec3 start;
    vec3 pt;
    vec3 del;
    float l;

    vec3 inv_rd;
    vec3 abs_inv_rd;
    vec3 step;
    vec3 tmax;

    uint voxel_index;
    int child_index;
};

struct ray_hit {
    vec4 col;
};

float MAX(in vec3 v) { return max(max(v.x, v.y), v.z); }
float MIN(in vec3 v) { return min(min(v.x, v.y), v.z); }
bool is_not_in(in float v, in float plus, in float minus) { return (v < minus) || (v >= plus); }
bool is_not_in(in vec3 v, in vec3 plus, in vec3 minus) { return is_not_in(v.x, plus.x, minus.x) || is_not_in(v.y, plus.y, minus.y) || is_not_in(v.z, plus.z, minus.z); }

int index64(in vec3 v) {
    return int(dot(vec3(1.0, 4.0, 16.0), floor(v)));
}

#ifndef USE_EMPTY
void make_empty(out uint empty_mask1, out uint empty_mask0, in tree_node childing) {
    empty_mask0 = 0;
    empty_mask1 = 0;
    for(uint j = 0; j < 32; j++)
        if (bool(1&(childing.leaf_mask0>>j)) && ((childing.child[j]&0xFF000000) != 0xFF000000))
            empty_mask0 |= 1<<j;
    for(uint j = 0; j < 32; j++)
        if (bool(1&(childing.leaf_mask1>>j)) && ((childing.child[32+j]&0xFF000000) != 0xFF000000))
            empty_mask1 |= 1<<j;
}
#endif//USE_EMPTY

bool mini_voxel(in uint empty_mask1, in uint empty_mask0, in uint index) {
    return (index < 32) ? bool(1&(empty_mask0 >> index)) : bool(1&(empty_mask1 >> (index - 32)));
}

int inter64(uint mask1, uint mask0, inout vec3 start, inout vec3 tmax, inout vec3 neg_norm, in vec3 s, in vec3 i) {
    int index = index64(start);
    for(uint j = 0; (j < 64) && (
        is_not_in(start, vec3(4.0), vec3(0.0)) || 
        mini_voxel(mask1, mask0, index)
    ) && !is_not_in(start, vec3(5.0), vec3(-1.0)); j++ ) {
        neg_norm = s * step(tmax, vec3(MIN(tmax)));
        index += int(dot(vec3(1.0, 4.0, 16.0), neg_norm));
        start += neg_norm;
        tmax += i * neg_norm;
    }
    if ( is_not_in(start, vec3(4.0), vec3(0.0)) ) return -1;
    return index;
}

ivec3 point_from_index(int index) {
    return ivec3(index%4, (index%16)/4, index/16);
}

struct stack_node {
    vec3 local_start;
    vec3 local_tmax;
    vec3 local_neg_norm;
    vec3 local_pt;

    uint voxel_index;
    int child_index;

    uint local_empty_mask0;
    uint local_empty_mask1;
    uint local_leaf_mask0;
    uint local_leaf_mask1;
};

#define MAX_LAYER 30

bool cast_eye(in vec3 ro, in vec3 rd, out path_hit p_hit) {
    
    p_hit.step = sign(rd);
    p_hit.abs_inv_rd = p_hit.step/rd;
    p_hit.inv_rd = p_hit.step*p_hit.abs_inv_rd;
    
    p_hit.pt = ro;

    p_hit.del = 0.5 * (p_hit.inv_rd - p_hit.abs_inv_rd);
    p_hit.l = max(0.0, MAX(4.0 * p_hit.del - ro * p_hit.inv_rd) - 0.0001);

    p_hit.pt += rd * p_hit.l;

    p_hit.start = floor(p_hit.pt);
    p_hit.tmax = (p_hit.start - p_hit.pt) * p_hit.inv_rd + p_hit.del + p_hit.abs_inv_rd;



    stack_node stack[MAX_LAYER];
    int stack_index = 0;

    stack[stack_index].local_start = p_hit.start;
    stack[stack_index].local_tmax = p_hit.tmax;
    stack[stack_index].local_pt = p_hit.pt;

    stack[stack_index].voxel_index = 0;
    tree_node n = voxels[stack[stack_index].voxel_index];
#ifdef USE_EMPTY
    stack[stack_index].local_empty_mask1 = n.empty_mask1;
    stack[stack_index].local_empty_mask0 = n.empty_mask0;
#else
    make_empty(stack[stack_index].local_empty_mask1, stack[stack_index].local_empty_mask0, n);
#endif//USE_EMPTY
    stack[stack_index].local_leaf_mask1 = n.leaf_mask1;
    stack[stack_index].local_leaf_mask0 = n.leaf_mask0;

    stack[stack_index].child_index = inter64(
    stack[stack_index].local_empty_mask1, stack[stack_index].local_empty_mask0, 
    stack[stack_index].local_start, stack[stack_index].local_tmax, stack[stack_index].local_neg_norm, 
    p_hit.step, p_hit.inv_rd);

    while (stack_index >= 0) {
        stack_node vox = stack[stack_index];
        
        if (vox.child_index < 0) {
            if (stack_index == 0) return false;
            else {
                stack_index--;

                if (stack[stack_index].child_index < 32)
                    stack[stack_index].local_empty_mask0 |= 1<<(stack[stack_index].child_index);
                else
                    stack[stack_index].local_empty_mask1 |= 1<<(stack[stack_index].child_index-32);

                stack[stack_index].child_index = inter64(
                stack[stack_index].local_empty_mask1, stack[stack_index].local_empty_mask0, 
                stack[stack_index].local_start, stack[stack_index].local_tmax, stack[stack_index].local_neg_norm, 
                p_hit.step, p_hit.inv_rd);

                continue;
            }
        }

        if (mini_voxel(vox.local_leaf_mask1, vox.local_leaf_mask0, vox.child_index)) {
            p_hit.voxel_index = vox.voxel_index;
            p_hit.child_index = vox.child_index;
            
//todo adding depth

            return true;
        } else {
            if (stack_index==MAX_LAYER) {
                p_hit.voxel_index = 0;
                p_hit.child_index = -3;
                return true;
            }

            stack_index++;

            stack[stack_index].local_pt = (vox.local_pt + rd*max(0, MAX(vox.local_tmax - p_hit.abs_inv_rd)) - point_from_index(vox.child_index))*4.0;
            stack[stack_index].local_start = floor(stack[stack_index].local_pt);
            stack[stack_index].local_tmax = (stack[stack_index].local_start - stack[stack_index].local_pt) * p_hit.inv_rd + p_hit.del + p_hit.abs_inv_rd;

            stack[stack_index].voxel_index = voxels[vox.voxel_index].child[vox.child_index];
            n = voxels[stack[stack_index].voxel_index];
#ifdef USE_EMPTY
            stack[stack_index].local_empty_mask1 = n.empty_mask1;
            stack[stack_index].local_empty_mask0 = n.empty_mask0;
#else
            make_empty(stack[stack_index].local_empty_mask1, stack[stack_index].local_empty_mask0, n);
#endif//USE_EMPTY
            stack[stack_index].local_leaf_mask1 = n.leaf_mask1;
            stack[stack_index].local_leaf_mask0 = n.leaf_mask0;

            stack[stack_index].child_index = inter64(
            stack[stack_index].local_empty_mask1, stack[stack_index].local_empty_mask0, 
            stack[stack_index].local_start, stack[stack_index].local_tmax, stack[stack_index].local_neg_norm, 
            p_hit.step, p_hit.inv_rd);
        }

    }
    p_hit.voxel_index = 0;
    p_hit.child_index = -2;
    return true;
}

void main() {
    ivec2 pixelCoords = ivec2(gl_GlobalInvocationID.xy);
    vec2 resolution = vec2(imageSize(imgOutput));

    vec2 uv = (2.0 * vec2(pixelCoords) - resolution)/ resolution.y;
    uv.x*=pyramid.x;

    vec3 ro = pos;
    vec3 rd = vec3(uv, pyramid.y);
    rd = quat(ang)*rd;
    rd = normalize(rd);

    path_hit p_hit;

    bool intersect = cast_eye(ro, rd, p_hit);

    if (!intersect) {
        imageStore(imgOutput, pixelCoords, vec4(0.5, 0.5, 0.5, 1.0));//vec4(sin(time), cos(time), cos(time)*sin(time), 1.0));
        return;
    }

    if (p_hit.child_index == -2) { imageStore(imgOutput, pixelCoords, vec4(sin(time), 0.0, 0.0, 1.0)); return;}
    if (p_hit.child_index == -3) { imageStore(imgOutput, pixelCoords, vec4(0.0, sin(time), 0.0, 1.0)); return;}

    if (mini_voxel(voxels[p_hit.voxel_index].leaf_mask1, voxels[p_hit.voxel_index].leaf_mask0, p_hit.child_index))
        imageStore(imgOutput, pixelCoords, unpackUnorm4x8(voxels[p_hit.voxel_index].child[p_hit.child_index]));
    else
        imageStore(imgOutput, pixelCoords, vec4(1.0));
})";

const char* shader_source_cursor_frag = R"(#version 330 core

out vec4 FragColor;

uniform vec4 color;

void main() {
    FragColor = color;
})";

const char* shader_source_cursor_vert = R"(#version 330 core

uniform vec2 position;
uniform vec2 window;

const vec2 delays[3] = vec2[](
    vec2(0, 0),
    vec2(18, -7),
    vec2(8, -18)
);

void main() {
    gl_Position = vec4(2.0*(position + delays[gl_VertexID])/window-1.0, 0.0, 1.0);
})";

const char* shader_source_frag = R"(#version 330 core

out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D screenTexture;

void main() {
    FragColor = texture(screenTexture, TexCoords);
})";

const char* shader_source_vert = R"(#version 330 core

uniform sampler2D screenTexture;

out vec2 TexCoords;

void main() {
    vec2 size = textureSize(screenTexture, 0);
    size = size/max(1,max(size.x, size.y));
    TexCoords = vec2((gl_VertexID&2)>>1, gl_VertexID&1);
    gl_Position = vec4((2.0*TexCoords-1.0)*size, 0.0, 1.0);
})";

#endif//SHADERS_SOURCE_HPP

