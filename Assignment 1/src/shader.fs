#version 330 core
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;
in vec2 TexCoord;
uniform bool iszap;
uniform sampler2D ourTexture;
void main()
{
   if(iszap)
      FragColor=texture(ourTexture,TexCoord)*vec4(2.0,2.0,2.0,1.0);
   else
      FragColor = texture(ourTexture,TexCoord);
   if(iszap)
      BrightColor = vec4(FragColor.rgb, 1.0);
   else
      BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
}