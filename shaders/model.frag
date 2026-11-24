#version 410 core
in vec2 UV;
in vec3 FragPos;
in vec3 Normal;
out vec4 FragColor;

// El fragment shader calcula texturas, transparencia y luces.
uniform sampler2D uTexture;
uniform vec4 uBaseColor;
uniform vec3 uLightDir;
uniform vec3 uPointPos;
uniform bool uDescartarNegro;

void main() {
    vec4 color = texture(uTexture, UV) * uBaseColor;
    if (color.a < 0.5) discard;

    // El descarte de color casi negro se activa solo para Konoha.
    if (uDescartarNegro &&
        color.r < 0.03 && color.g < 0.03 && color.b < 0.03) discard;

    vec3 n = normalize(Normal);
    if (!gl_FrontFacing) n = -n;

    // Combino luz ambiental, direccional y puntual.
    vec3 ambiental = vec3(0.70);
    // El producto punto mide el ángulo entre normal y dirección de luz.
    float difusaSol = max(dot(n, normalize(uLightDir)), 0.0);
    vec3 sol = vec3(1.0, 0.97, 0.91) * 0.42 * difusaSol;

    vec3 haciaPunto = uPointPos - FragPos;
    float distancia = length(haciaPunto);
    vec3 dirPunto = haciaPunto / max(distancia, 0.0001);
    float difusaPunto = max(dot(n, dirPunto), 0.0);
    // La luz puntual pierde intensidad al aumentar la distancia.
    float atenuacion = 1.0 / (1.0 + 3.0 * distancia + 12.0 * distancia * distancia);
    vec3 punto = vec3(1.0, 0.78, 0.58) * 0.75 * difusaPunto * atenuacion;

    vec3 iluminacion = ambiental + sol + punto;
    // Aplico la iluminación al color RGB y conservo el canal alfa.
    FragColor = vec4(color.rgb * iluminacion, color.a);
}
