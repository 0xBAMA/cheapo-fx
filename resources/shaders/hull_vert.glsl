#version 330

in  vec3 vPosition;
in  vec2 vTexCoord;
in  vec3 vNormal;
in  vec4 vColor;

varying vec4 color;
varying vec3 vpos;
varying vec2 texcoord;
varying vec3 normal;

uniform vec3 yawpitchroll;  //in that order
uniform mat4 proj;
uniform mat4 view;
uniform float scale;
uniform int t;

uniform sampler2D height_tex;


mat4 rotationMatrix(vec3 axis, float angle)
{//thanks to Neil Mendoza via http://www.neilmendoza.com/glsl-rotation-about-an-arbitrary-axis/
    axis = normalize(axis);
    float s = sin(angle);
    float c = cos(angle);
    float oc = 1.0 - c;

    return mat4(oc * axis.x * axis.x + c,           oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s,  0.0,
                oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s,  0.0,
                oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c,           0.0,
                0.0,                                0.0,                                0.0,                                1.0);
}


void main()
{
  // color = vec4(vTexCoord.x,vTexCoord.y,0.2,1.0);
  // color = vec4(vNormal+0.5,1.0);
  // color = vColor;

  // vec4 heightsample = texture(height_tex, vTexCoord);
  color = vColor;

  texcoord = vTexCoord;



  vec3 yaw_vec = vec3(0,1,0); //yaw is about the axis represented by a vertical vector
  vec3 pitch_vec = vec3(1,0,0); //pitch is about the axis represented by a vector to the right
  vec3 roll_vec = vec3(0,0,1); //roll is about the axis represented by a vector in the direction of the ship's travel

  vec3 transformed_normal = vNormal;


//YAW
  mat4 apply_yaw =  rotationMatrix(yaw_vec, yawpitchroll.x);

  pitch_vec = (apply_yaw * vec4(pitch_vec,0.0)).xyz;  //these vectors have to be translated, to keep up with the translated model
  roll_vec = (apply_yaw * vec4(roll_vec,0.0)).xyz;

  transformed_normal = (apply_yaw * vec4(transformed_normal,0.0)).xyz;



//PITCH
  mat4 apply_pitch =  rotationMatrix(pitch_vec, yawpitchroll.y);

  roll_vec = (apply_pitch * vec4(roll_vec,0.0)).xyz;
  transformed_normal = (apply_pitch * vec4(transformed_normal,0.0)).xyz;



//ROLL

  mat4 apply_roll = rotationMatrix(roll_vec, yawpitchroll.z);

  transformed_normal = (apply_roll * vec4(transformed_normal,0.0)).xyz;

  normal = transformed_normal;

  vec4 vPosition_local = apply_roll*(apply_pitch*(apply_yaw*vec4(scale*vPosition, 1.0)));



  vpos = vPosition_local.xyz;





  gl_Position = proj*view*vPosition_local;

}
