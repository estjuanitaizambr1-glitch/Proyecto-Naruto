#version 410 core
// Los atributos 0, 1 y 2 son posición, UV y normal.
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aUV;
layout(location = 2) in vec3 aNormal;

// El vertex shader usa MVP para proyectar y Model para iluminar en coordenadas del mundo.
uniform mat4 uMVP;
uniform mat4 uModel;

out vec2 UV;
out vec3 FragPos;
out vec3 Normal;

void main() {
    // Transformo cada vértice del modelo al espacio de recorte.
    gl_Position = uMVP * vec4(aPos, 1.0);
    UV = aUV;
    FragPos = vec3(uModel * vec4(aPos, 1.0));
    // La matriz inversa transpuesta conserva la orientación de la normal.
    Normal = mat3(transpose(inverse(uModel))) * aNormal;
}
