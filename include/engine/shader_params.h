/**
 * Funciones para establecer parámetros de shader en modelos
 */

// Función auxiliar para encontrar o crear un parámetro
static ShaderParam *modelo_get_or_create_param(Modelo *m, const char *name)
{
    // Buscar si ya existe
    for (int i = 0; i < m->num_params; i++)
    {
        if (strcmp(m->params[i].name, name) == 0)
        {
            return &m->params[i];
        }
    }

    // Si no existe, crear uno nuevo
    if (m->num_params >= m->max_params)
    {
        // Necesitamos expandir el array
        int new_max = (m->max_params == 0) ? 4 : m->max_params * 2;
        ShaderParam *new_params = (ShaderParam *)SDL_realloc(m->params, new_max * sizeof(ShaderParam));
        
        if (!new_params)
        {
            debug_log("ERROR: No se pudo asignar memoria para parámetros de shader");
            return NULL;
        }
        
        m->params = new_params;
        m->max_params = new_max;
    }

    // Crear nuevo parámetro
    ShaderParam *param = &m->params[m->num_params];
    strncpy(param->name, name, 31);
    param->name[31] = '\0';
    m->num_params++;

    return param;
}

// Establecer parámetro float
static inline void modelo_set_float(Modelo *m, const char *name, float value)
{
    ShaderParam *param = modelo_get_or_create_param(m, name);
    if (param)
    {
        param->type = SHADER_PARAM_FLOAT;
        param->value.f = value;
    }
}

// Establecer parámetro vec2
static inline void modelo_set_vec2(Modelo *m, const char *name, float x, float y)
{
    ShaderParam *param = modelo_get_or_create_param(m, name);
    if (param)
    {
        param->type = SHADER_PARAM_VEC2;
        param->value.vec2[0] = x;
        param->value.vec2[1] = y;
    }
}

// Establecer parámetro vec3
static inline void modelo_set_vec3(Modelo *m, const char *name, float x, float y, float z)
{
    ShaderParam *param = modelo_get_or_create_param(m, name);
    if (param)
    {
        param->type = SHADER_PARAM_VEC3;
        param->value.vec3[0] = x;
        param->value.vec3[1] = y;
        param->value.vec3[2] = z;
    }
}

// Establecer parámetro vec4
static inline void modelo_set_vec4(Modelo *m, const char *name, float x, float y, float z, float w)
{
    ShaderParam *param = modelo_get_or_create_param(m, name);
    if (param)
    {
        param->type = SHADER_PARAM_VEC4;
        param->value.vec4[0] = x;
        param->value.vec4[1] = y;
        param->value.vec4[2] = z;
        param->value.vec4[3] = w;
    }
}

// Establecer parámetro int
static inline void modelo_set_int(Modelo *m, const char *name, int value)
{
    ShaderParam *param = modelo_get_or_create_param(m, name);
    if (param)
    {
        param->type = SHADER_PARAM_INT;
        param->value.i = value;
    }
}

// Aplicar todos los parámetros al shader actual
static inline void modelo_apply_shader_params(Modelo *m)
{
    if (!m->params || m->num_params == 0)
        return;

    GLuint current_program = (m->shader != 0) ? m->shader : 0;
    if (current_program == 0)
        return;

    for (int i = 0; i < m->num_params; i++)
    {
        ShaderParam *param = &m->params[i];
        
        switch (param->type)
        {
        case SHADER_PARAM_FLOAT:
            shader_set_float(current_program, param->name, param->value.f);
            break;
        case SHADER_PARAM_VEC2:
            shader_set_vec2(current_program, param->name, 
                          param->value.vec2[0], param->value.vec2[1]);
            break;
        case SHADER_PARAM_VEC3:
            shader_set_vec3(current_program, param->name,
                          param->value.vec3[0], param->value.vec3[1], param->value.vec3[2]);
            break;
        case SHADER_PARAM_VEC4:
            shader_set_vec4(current_program, param->name,
                          param->value.vec4[0], param->value.vec4[1], 
                          param->value.vec4[2], param->value.vec4[3]);
            break;
        case SHADER_PARAM_INT:
            shader_set_int(current_program, param->name, param->value.i);
            break;
        }
    }
}
