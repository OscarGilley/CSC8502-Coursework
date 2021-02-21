#version 330 core

uniform sampler2D mainTex;
uniform sampler2D secondTex;

uniform vec3 cameraPos;
uniform vec4 lightColour;
uniform vec3 lightPos;
uniform float lightRadius;
uniform float yCoord;
uniform float time;

in Vertex{
	vec4 colour;
	vec2 texCoord;
	vec3 normal;
	vec3 worldPos;
} IN;

out vec4 fragColour;

void main(void) {
	vec3 incident = normalize(lightPos - IN.worldPos);
	vec3 viewDir = normalize(cameraPos - IN.worldPos);
	vec3 halfDir = normalize(incident + viewDir);

	//vec4 diffuse = texture(diffuseTex, IN.texCoord);

	float lambert = max(dot(incident, IN.normal), 0.0f);
	float distance = length(lightPos - IN.worldPos);
	float attenuation = 1.0 - clamp(distance / lightRadius, 0.0, 1.0);

	float specFactor = clamp(dot(halfDir, IN.normal),0.0,1.0);
	specFactor = pow(specFactor, 60.0);

	//vec4 diffuseA = texture(mainTex, (IN.texCoord / 10) * 9);
	vec4 diffuseA = texture(mainTex, IN.texCoord);
	vec3 surface = (diffuseA.rgb * lightColour.rgb);
	vec3 diffuse = surface * lambert * attenuation;
	vec3 specular = (lightColour.rgb * specFactor) * attenuation * 0.33;
	vec3 ambient = surface * 0.1f; //ambient!
	vec4 fragColourA = vec4(ambient + diffuse + specular, diffuseA.a);
	//fragColour.a = diffuse.a;
	
	//vec4 diffuseB = texture(secondTex, IN.texCoord / 10);
	vec4 diffuseB = texture(secondTex, IN.texCoord);
	surface = (diffuseB.rgb * lightColour.rgb);
	diffuse = surface * lambert * attenuation;
	specular = (lightColour.rgb * specFactor) * attenuation * 0.33;
	ambient = surface * 0.1f; //ambient!
	vec4 fragColourB = vec4(ambient + diffuse + specular, diffuseB.a);
	
	vec4 firstTex = texture2D(mainTex, IN.texCoord);
	vec4 secondTex = texture2D(secondTex, IN.texCoord);
	
	if(IN.normal.y > 0){
		fragColour += fragColourB;
	}

	if(IN.normal.y <= 0){
		fragColour += fragColourA;
	}
	
}