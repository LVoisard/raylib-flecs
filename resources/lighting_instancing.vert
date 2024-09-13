#version 330

// Input vertex attributes
in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec3 vertexNormal;
//in vec4 vertexColor;      // Not required

in mat4 instanceTransform;

// Input uniform values
uniform mat4 mvp;
uniform mat4 matNormal;
uniform sampler2D matEmission;

uniform int textureWidth;
uniform int textureHeight;
uniform sampler2D cols;

// Output vertex attributes (to fragment shader)
out vec3 fragPosition;
out vec2 fragTexCoord;
out vec4 fragColor;
out vec3 fragNormal;

// NOTE: Add here your custom variables

void main()
{
    // Send vertex attributes to fragment shader
    fragPosition = vec3(instanceTransform*vec4(vertexPosition, 1.0));
    fragTexCoord = vertexTexCoord;
    //fragColor = vec4(vec3(textureWidth / 1000.0), 1);
    float xCoord = (gl_InstanceID % textureWidth) / float(textureWidth);
    float yCoord = floor(float(gl_InstanceID / float(textureWidth))) / float(textureHeight);
    fragColor = texture(matEmission, vec2(xCoord, yCoord));
    fragNormal = normalize(vec3(matNormal*vec4(vertexNormal, 1.0)));



    // Calculate final vertex position, note that we multiply mvp by instanceTransform

    
    gl_Position = mvp*instanceTransform*vec4(vertexPosition, 1.0);
}
