#vert
#import "_image.glsl"

#frag
in      lowp  vec2      coord_f;
uniform lowp  float     opacity;
uniform       sampler2D image;
uniform lowp  vec4      color;

void main() {
	fragColor = color * vec4(1.0,1.0,1.0,texture(image, coord_f).a);
}