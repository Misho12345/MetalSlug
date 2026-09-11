#include "mse/pch.hpp"

#include "shader.hpp"
#include "shader_compiler.hpp"

namespace mse::gl
{
    Shader::~Shader()
    {
        if (id_) glDeleteProgram(id_);
    }

    bool Shader::create(const string_view vertex, const string_view fragment)
    {
        if (id_)
        {
            printf("already compiled shader program\n");
            return false;
        }

        const string vert_src = FileIO::read(vertex);
        if (vert_src.empty())
        {
            printf("failed to read vertex shader source\n");
            return false;
        }

        const string frag_src = FileIO::read(fragment);
        if (frag_src.empty())
        {
            printf("failed to read fragment shader source\n");
            return false;
        }

        const GLint vert_stage = ShaderCompiler::compile_stage(GL_VERTEX_SHADER, vert_src);
        if (vert_stage < 0) return false;

        const GLint frag_stage = ShaderCompiler::compile_stage(GL_FRAGMENT_SHADER, frag_src);
        if (frag_stage < 0)
        {
            glDeleteShader(static_cast<GLuint>(vert_stage));
            return false;
        }

        const GLuint stages[] = {
            static_cast<GLuint>(vert_stage),
            static_cast<GLuint>(frag_stage)
        };

        id_ = ShaderCompiler::link_program(stages);

        glDeleteShader(stages[0]);
        glDeleteShader(stages[1]);

        return id_ > 0;
    }

    void Shader::use() const
    {
        if (id_) glUseProgram(id_);
    }

    void Shader::unuse() { glUseProgram(0); }

    GLint Shader::uniform_location(const char* name) const
    {
        if (!id_) return -1;
        return glGetUniformLocation(id_, name);
    }
}
