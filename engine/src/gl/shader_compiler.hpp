#pragma once
#include "mse/pch.hpp"

namespace mse::gl
{
    /// @brief The ShaderCompiler class is a utility class responsible for compiling and linking shaders
    class ShaderCompiler final
    {
    public:
        ShaderCompiler() = delete;

        static GLint compile_stage(GLenum stage, string_view source);
        static GLint link_program(span<const GLuint> shaders);

    private:
        static string shader_log(GLuint shader); // Returns the log of a shader compilation
        static string program_log(GLuint program); // Returns the log of a program linking
        static void trim_log(string& log);
    };
}