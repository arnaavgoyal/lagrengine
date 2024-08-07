#version 460 core

in vec3 position;
in vec3 normal;
in vec2 uv;

out vec4 frag_color;

struct Material {
    sampler2D ambient;
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
};

struct DirLight {
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight {
    vec3 position;

    float constant;
    float linear;
    float quadratic;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform Material mat;

vec3 dirLightColor(DirLight light, Material mat, vec3 unit_surface_norm,
    vec3 view_dir) {
    vec3 flipped_light_dir = -normalize(light.direction);

    // diffuse calculation
    float diff_factor = max(dot(unit_surface_norm, flipped_light_dir), 0.0);

    // specular calculation
    vec3 reflect_dir = reflect(-flipped_light_dir, unit_surface_norm);
    float spec_factor = pow(max(dot(view_dir, reflect_dir), 0.0),
        mat.shininess);

    // add ambient, diffuse, and specular lighting
    vec3 total = light.ambient * vec3(texture(mat.ambient, uv));
    total += light.diffuse * diff_factor * vec3(texture(mat.diffuse, uv));
    total += light.specular * spec_factor * vec3(texture(mat.specular, uv));

    return total;
}

vec3 pointLightColor(PointLight light, Material mat, vec3 unit_surface_norm,
    vec3 view_dir, vec3 frag_pos) {
    vec3 flipped_light_dir = normalize(light.position - frag_pos);

    // diffuse calculation
    float diff_factor = max(dot(unit_surface_norm, flipped_light_dir), 0.0);

    // specular calculation
    vec3 reflect_dir = reflect(-flipped_light_dir, unit_surface_norm);
    float spec_factor = pow(max(dot(view_dir, reflect_dir), 0.0),
        mat.shininess);

    float d = length(light.position - frag_pos);
    float attenuation = 1.0 /
        (light.constant + light.linear * d + light.quadratic * (d * d));

    vec3 total = light.ambient * vec3(texture(mat.ambient, uv));
    total += light.diffuse * diff_factor * vec3(texture(mat.diffuse, uv));
    total += light.specular * spec_factor * vec3(texture(mat.specular, uv));

    return total;
}

void main() {
    //DirLight light;
    //light.direction = normalize(vec3(-1.0, 0.0, 0.0));
    //light.ambient = vec3(0.2, 0.2, 0.2);
    //light.diffuse = vec3(0.5, 0.5, 0.5);
    //light.specular = vec3(0.0, 0.0, 0.0);

    //vec3 color = dirLightColor(light, mat, normalize(normal), vec3(0.0, 0.0, -1.0));

    PointLight light;
    light.position = vec3(2.5, 0.0, 0.0);
    light.constant = 1.0;
    light.linear = 0.7;
    light.quadratic = 1.8;
    light.ambient = vec3(0.3, 0.3, 0.3);
    light.diffuse = vec3(0.6, 0.6, 0.6);
    light.specular = vec3(0.0, 0.0, 0.0);

    vec3 color = pointLightColor(light, mat, normalize(normal),
        vec3(0.0, 0.0, -1.0), position);

    frag_color = vec4(color, 1.0);
}

