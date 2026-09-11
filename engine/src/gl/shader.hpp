#pragma once
#include "mse/pch.hpp"

namespace mse::gl
{
    class Shader final
    {
    public:
        Shader() = default;
        ~Shader();

        Shader(const Shader&)                = delete;
        Shader(Shader&&) noexcept            = delete;
        Shader& operator=(const Shader&)     = delete;
        Shader& operator=(Shader&&) noexcept = delete;

        bool create(string_view vertex, string_view fragment);

        void use() const;
        static void unuse();

        template <typename T>
        static void set_uniform(GLint location, const T& value)
        {
            if (location < 0)
            {
                printf("invalid uniform location: %d\n", location);
                return;
            }

            if constexpr (std::same_as<T, float>) glUniform1f(location, value);
            else if constexpr (std::same_as<T, glm::vec2>) glUniform2f(location, value.x, value.y);
            else if constexpr (std::same_as<T, glm::vec3>) glUniform3f(location, value.x, value.y, value.z);
            else if constexpr (std::same_as<T, glm::vec4>) glUniform4f(location, value.x, value.y, value.z, value.w);

            else if constexpr (std::same_as<T, int32_t>) glUniform1i(location, value);
            else if constexpr (std::same_as<T, glm::ivec2>) glUniform2i(location, value.x, value.y);
            else if constexpr (std::same_as<T, glm::ivec3>) glUniform3i(location, value.x, value.y, value.z);
            else if constexpr (std::same_as<T, glm::ivec4>) glUniform4i(location, value.x, value.y, value.z, value.w);

            else if constexpr (std::same_as<T, uint32_t>) glUniform1ui(location, value);
            else if constexpr (std::same_as<T, glm::uvec2>) glUniform2ui(location, value.x, value.y);
            else if constexpr (std::same_as<T, glm::uvec3>) glUniform3ui(location, value.x, value.y, value.z);
            else if constexpr (std::same_as<T, glm::uvec4>) glUniform4ui(location, value.x, value.y, value.z, value.w);

            else if constexpr (std::same_as<T, glm::mat3>) glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(value));
            else if constexpr (std::same_as<T, glm::mat4>) glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value));

            else static_assert(false, "Unsupported uniform type");
        }

        GLint uniform_location(const char* name) const;

    private:
        GLuint id_{ 0 };
    };
}
