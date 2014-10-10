in vec3 position;
in vec3 normal;

uniform vec3 lightAmbient;

uniform vec3 lightDirection;
uniform vec3 lightColor;
uniform vec3 specularColor;

uniform vec3 cameraPosition;

void main()
{
	vec3 N = normalize(normal);
	vec3 L = normalize(-lightDirection);	//direction the light is coming "from"
	
	vec3 R = reflect(-L, N);
	vec3 E = normalize(cameraPosition - position);
	
	// diffuse lighting
	float d = max(0, dot(N,L));
	vec3 diffuse = lightColor *d;
	
	vec3 ambient = lightAmbient;
	
	float s = pow(max(0, dot(E,R)), 128);
	vec3 specular = specularColor * s;
	
	gl_FragColor = vec4(ambient + diffuse + specular, 1);
}