/*
Title: Reflection and refraction
File Name: VertexShaderSkyBox.glsl
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
layout(location = 1) in vec3 in_normal;		//This is nor used in this shader. But since the vertex has an attribute called normal we just make it to avoid errors.
//During setting up the vertexattribs, and setting the buffer data, if we set-up the offset properly,this variable does not need to be here. 

out vec3 texCoord; // Our vec4 color variable containing r, g, b, a

void main(void)
{
	//use the posiiton as the texture coorinates for the skybox
	texCoord = normalize(in_position);

	//Call the funciton using the subroutine uniform which is set in the openGL application.
	gl_Position = vec4(in_position, 1.0); //w is 1.0, also notice cast to a vec4
}