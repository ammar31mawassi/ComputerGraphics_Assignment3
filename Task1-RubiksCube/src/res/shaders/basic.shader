#shader vertex
#version 330

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec2 texCoord;

out vec4 v_Color;
out vec2 v_TexCoord;
out vec3 v_Position;

uniform mat4 u_MVP;
uniform int u_UseFaceColors;
uniform vec4 u_ColorFace;

void main()
{
	gl_Position = u_MVP * vec4(position.x, position.y, position.z, 1.0);
	v_Position = position;
	v_TexCoord = texCoord;
	if (u_UseFaceColors == 1)
		v_Color = u_ColorFace;
	else
		v_Color = vec4(color.x, color.y, color.z, 1.0);
}

#shader fragment
#version 330

layout(location = 0) out vec4 FragColor;

in vec4 v_Color;
in vec2 v_TexCoord;
in vec3 v_Position;

uniform vec4 u_Color;
uniform sampler2D u_Texture;
uniform int u_PickingMode;
uniform int u_UseFaceColors;
uniform int u_Highlight;
uniform float u_EdgeWidth;
uniform int u_FaceAxis;

void main()
{
	if (u_PickingMode == 1) {
		FragColor = u_Color;
	} else if (u_UseFaceColors == 1) {
		vec3 p = v_Position;
		bool atEdge = false;
		if (u_FaceAxis == 0)      // right/left face: y,z vary
			atEdge = (p.y < -0.5 + u_EdgeWidth || p.y > 0.5 - u_EdgeWidth || p.z < -0.5 + u_EdgeWidth || p.z > 0.5 - u_EdgeWidth);
		else if (u_FaceAxis == 1) // top/bottom face: x,z vary
			atEdge = (p.x < -0.5 + u_EdgeWidth || p.x > 0.5 - u_EdgeWidth || p.z < -0.5 + u_EdgeWidth || p.z > 0.5 - u_EdgeWidth);
		else                      // front/back face: x,y vary
			atEdge = (p.x < -0.5 + u_EdgeWidth || p.x > 0.5 - u_EdgeWidth || p.y < -0.5 + u_EdgeWidth || p.y > 0.5 - u_EdgeWidth);
		if (atEdge) {
			FragColor = vec4(0.0, 0.0, 0.0, 1.0);
		} else {
			vec4 faceColor = v_Color;
			if (u_Highlight == 1)
				faceColor = mix(faceColor, vec4(1.0, 1.0, 0.4, 1.0), 0.35);
			FragColor = faceColor;
		}
	} else {
		vec4 texColor = texture(u_Texture, v_TexCoord) * u_Color;
		FragColor = texColor * v_Color;
	}
}