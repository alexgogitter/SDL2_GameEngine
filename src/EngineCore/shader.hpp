#pragma once

#include <string>

#include <glad/glad.h>
#include <glm.hpp>

/// RAII wrapper for one linked OpenGL shader program.
/// All methods require a current OpenGL context.
class Shader
{
  public:
    /// Creates an empty, invalid shader wrapper.
    Shader() = default;
    /// Deletes the linked OpenGL program when present.
    ~Shader();

    Shader(const Shader &) = delete;
    Shader &operator=(const Shader &) = delete;

    /// Reads, compiles, and links a vertex/fragment shader pair from disk.
    /// @param vertexPath Vertex shader path relative to the working directory.
    /// @param fragmentPath Fragment shader path relative to the working directory.
    /// @return True when both stages compile and the program links.
    bool loadFromFiles(const std::string &vertexPath, const std::string &fragmentPath);

    /// Compiles and links a vertex/fragment shader pair from source strings.
    /// @return True when both stages compile and the program links.
    bool loadFromSource(const std::string &vertexSource, const std::string &fragmentSource);

    /// Makes this program current with glUseProgram().
    void use() const;
    /// @return Whether this wrapper currently owns a linked OpenGL program.
    bool isValid() const { return programId != 0; }
    /// @return Owned OpenGL program object name, or zero when invalid.
    unsigned int getProgramId() const { return programId; }

    /// Sets an integer-backed GLSL boolean uniform on the current program.
    void setBool(const std::string &name, bool value) const;
    /// Sets a GLSL integer/sampler uniform on the current program.
    void setInt(const std::string &name, int value) const;
    /// Sets a GLSL float uniform on the current program.
    void setFloat(const std::string &name, float value) const;
    /// Sets a GLSL vec3 uniform on the current program.
    void setVec3(const std::string &name, const glm::vec3 &value) const;
    /// Sets a GLSL vec4 uniform on the current program.
    void setVec4(const std::string &name, const glm::vec4 &value) const;
    /// Sets a column-major GLSL mat4 uniform on the current program.
    void setMat4(const std::string &name, const glm::mat4 &value) const;

  private:
    static unsigned int compileStage(unsigned int type, const std::string &source);
    int getUniformLocation(const std::string &name) const;

    unsigned int programId = 0;
};
