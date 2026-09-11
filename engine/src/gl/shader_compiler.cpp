#include "shader_compiler.hpp"

namespace mse::gl
{
    GLint ShaderCompiler::compile_stage(const GLenum stage, const string_view source)
    {
        if (source.empty())
        {
            printf("Shader source is empty\n");
            return -1;
        }

        const GLuint shader = glCreateShader(stage);
        const char*  data   = source.data();
        glShaderSource(shader, 1, &data, nullptr);
        glCompileShader(shader);

        GLint compiled = GL_FALSE;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);

        if (compiled == GL_FALSE)
        {
            const string log = shader_log(shader);
            glDeleteShader(shader);
            printf("Failed to compile shader: %s\n",
                log.empty() ? "No additional info" : log.data());
            return -1;
        }

        return shader;
    }

    GLint ShaderCompiler::link_program(const span<const GLuint> shaders)
    {
        if (shaders.empty())
        {
            printf("No shader stages provided for linking\n");
            return -1;
        }

        const GLuint program = glCreateProgram();

        for (size_t i = 0; i < shaders.size(); ++i) glAttachShader(program, shaders[i]);
        glLinkProgram(program);
        for (size_t i = 0; i < shaders.size(); ++i) glDetachShader(program, shaders[i]);

        GLint linked = GL_FALSE;
        glGetProgramiv(program, GL_LINK_STATUS, &linked);

        if (linked == GL_FALSE)
        {
            const string log = program_log(program);
            glDeleteProgram(program);
            printf("Failed to link shader program: %s\n",
                   log.empty() ? "No additional info" : log.data());
            return -1;
        }

        return program;
    }

    string ShaderCompiler::shader_log(const GLuint shader)
    {
        GLint length = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);

        if (length <= 1) return {};

        string log(static_cast<std::size_t>(length), '\0');
        glGetShaderInfoLog(shader, length, nullptr, log.data());
        trim_log(log);
        return log;
    }

    string ShaderCompiler::program_log(const GLuint program)
    {
        GLint length = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);

        if (length <= 1) return {};

        string log(static_cast<size_t>(length), '\0');
        glGetProgramInfoLog(program, length, nullptr, log.data());
        trim_log(log);
        return log;
    }

    void ShaderCompiler::trim_log(string& log)
    {
        while (!log.empty() && isspace(log.back())) log.pop_back();
    }
}
