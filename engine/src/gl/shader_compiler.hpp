#pragma once
#include "mse/pch.hpp"

namespace mse::gl
{
    class ShaderCompiler final
    {
    public:
        ShaderCompiler() = delete;

        static GLint compile_stage(GLenum stage, string_view source);
        static GLint link_program(span<const GLuint> shaders);

    private:
        static string shader_log(GLuint shader);
        static string program_log(GLuint program);
        static void trim_log(string& log);
    };
}