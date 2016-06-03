/*
Title: Reflection and refraction
File Name: VertexShader.glsl
Copyright © 2015
Original authors: Srinivasan T
Written under the supervision of David I. Schwartz, Ph.D., and
supported by a professional development seed grant from the B. Thomas
Golisano College of Computing & Information Sciences
(https://www.rit.edu/gccis) at the Rochester Institute of Technology.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or (at
your option) any later version.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

Description:
This program demonstrates the implementation of reflection and refraction
in games using skyboxes.
In games, the reflections are rendered using skyboxes. We calculate the
vector from the camera to the opint (vertex) to be rendered. This vector
is then reflected off the surface with respect to the surface normal at that
point. Then that vector is used as the texture coordinates to sample the
cube map of the skybox. Then that color is used to display that pixel.
This process does not have much effect on the performance as most of the
work is done in the shader including the reflecting and refracting.

To refract light rays, the same process is applied as for reflection. In this
program, the background image is the skybox. So we can use the refracted ray
as a texture coordinate for the cube map of the skybox. But if you wish to do
this more realistically, you should render the object behind the refractive object
to a texture using a frame buffer. then sample that texture using the refracted
ray as texture coordinates.

Use the mouse to move the sphere around in xy plane.
Comment out the reflective or refractive component in the fragment shader to see the
effects more vividly.
References:
OpenGL 4 shading language cookbook by David Wolff
*/

#version 430 core // Identifies the version of the shader, this line must be on a separate line from the rest of the shader code
 
layout(location = 0) in vec3 in_position;	// Get in a vec3 for position
layout(location = 1) in vec3 in_normal;

out vec4 color;								// This variable carries the light component on that pixel. 
out vec3 reflectDir;						// this variable hold the reflected vector
out vec3 refractDir;						// This variable hold the refracted vector

uniform mat4 PV;							// Our uniform PV matrix to implement projection and view for the camera
uniform mat4 translation;					// This is the transformation matrix. Since we are not rotating the sphere, this basically contains just the translation.
uniform vec3 camPos;						// camera position for specular lighting.

vec3 LightPos;								// Light properties.
vec3 DiffuseLight;
vec3 SpecularLight;

//This function returns the component of the light reflected as diffuse texture.
vec3 diffuseComponent(vec3 position, vec3 normal)
{
	vec3 s = normalize(LightPos - position);

	return DiffuseLight * max(dot(s, normal),0.0f);
}

//This function deals with the light reflected due to specular behaviour
vec3 specularComponent(vec3 position, vec3 normal)
{
	vec3 s = normalize(LightPos - position);
	vec3 r = (2 * dot(s,normal) * normal) - s;
	vec3 v = normalize(camPos - position);
	
	//the more the power of dot(v,r), the smaller the shinier the surface.
	return SpecularLight * max(pow(dot(v,r),3),0.0f);
}

vec4 diffuseAndSpecular (vec3 position, vec3 normal)
{
	// This function uses the above two functions to calculate the lighting values.
	return vec4((diffuseComponent(position,normal) + specularComponent(position, normal)),1.0f);
}

void main(void)
{
	LightPos = vec3(3.0f, 3.0f,0.0f);
	DiffuseLight = vec3 (0.5f,0.5f,0.5f);
	SpecularLight = vec3(0.74f,0.74f,0.74f);

	//Since the object is moving in the world space, we need to apply those transformation to the position and normals of the vertex.
	vec3 pos = (vec4(in_position,1.0f) * translation).xyz;
	vec3 normal = (vec4(in_normal, 1.0f) * translation).xyz;
	vec3 viewDirection = normalize(camPos - pos);
	
	//Reflect the vector view direction with respect to normal.
	reflectDir = reflect(-viewDirection, normal);
	//Refract the vector view Direction, with respect to normal with the ration of the indices of refraction.
	// refract(incidentVector, normalVector, ratio)
	refractDir = refract(-viewDirection, normal, 0.5f);
	
	//Calculate the lighting calculations
	color = diffuseAndSpecular(pos, normalize(normal));
	//apply the transformation and multiply with the view and prespective matrix to get the final positio nof the vertex.
	gl_Position = PV * translation * vec4(in_position, 1.0); //w is 1.0, also notice cast to a vec4
}