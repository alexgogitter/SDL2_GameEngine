#include "shader.hpp"

#include <fstream>
#include <iostream>
#include <sstream>

#include <glm/gtc/type_ptr.hpp>

Shader::~Shader()
{
    if (programId != 0)
    {
        glDeleteProgram(programId);
    }
}

bool Shader::loadFromFiles(
    const std::string& vertexPath,
    const std::string& fragmentPath)
{
    std::ifstream vertexFile(vertexPath);
    std::ifstream fragmentFile(fragmentPath);
    if (!vertexFile || !fragmentFile)
    {
        std::cerr << "ERROR: Unable to open shader files: "
                  << vertexPath << " and " << fragmentPath << '\n';
        return false;
    }

    std::ostringstream vertexSource;
    std::ostringstream fragmentSource;
    vertexSource << vertexFile.rdbuf();
    fragmentSource << fragmentFile.rdbuf();
    return loadFromSource(vertexSource.str(), fragmentSource.str());
}

bool Shader::loadFromSource(
    const std::string& vertexSource,
    const std::string& fragmentSource)
{
    const unsigned int vertexShader = compileStage(GL_VERTEX_SHADER, vertexSource);
    const unsigned int fragmentShader = compileStage(GL_FRAGMENT_SHADER, fragmentSource);
    if (vertexShader == 0 || fragmentShader == 0)
    {
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        return false;
    }

    const unsigned int newProgram = glCreateProgram();
    glAttachShader(newProgram, vertexShader);
    glAttachShader(newProgram, fragmentShader);
    glLinkProgram(newProgram);

    int success = GL_FALSE;
    glGetProgramiv(newProgram, GL_LINK_STATUS, &success);
    if (success != GL_TRUE)
    {
        char log[2048] = {};
        glGetProgramInfoLog(newProgram, sizeof(log), nullptr, log);
        std::cerr << "ERROR: Shader program link failed:\n" << log << '\n';
        glDeleteProgram(newProgram);
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        return false;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    if (programId != 0)
    {
        glDeleteProgram(programId);
    }

    programId = newProgram;
    return true;
}

void Shader::use() const
{
    glUseProgram(programId);
}

void Shader::setBool(const std::string& name, bool value) const
{
    glUniform1i(getUniformLocation(name), value ? 1 : 0);
}

void Shader::setInt(const std::string& name, int value) const
{
    glUniform1i(getUniformLocation(name), value);
}

void Shader::setFloat(const std::string& name, float value) const
{
    glUniform1f(getUniformLocation(name), value);
}

void Shader::setVec3(const std::string& name, const glm::vec3& value) const
{
    glUniform3fv(getUniformLocation(name), 1, glm::value_ptr(value));
}

void Shader::setVec4(const std::string& name, const glm::vec4& value) const
{
    glUniform4fv(getUniformLocation(name), 1, glm::value_ptr(value));
}

void Shader::setMat4(const std::string& name, const glm::mat4& value) const
{
    glUniformMatrix4fv(
        getUniformLocation(name),
        1,
        GL_FALSE,
        glm::value_ptr(value)
    );
}

unsigned int Shader::compileStage(unsigned int type, const std::string& source)
{
    const unsigned int shader = glCreateShader(type);
    const char* sourcePointer = source.c_str();
    glShaderSource(shader, 1, &sourcePointer, nullptr);
    glCompileShader(shader);

    int success = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (success != GL_TRUE)
    {
        char log[2048] = {};
        glGetShaderInfoLog(shader, sizeof(log), nullptr, log);
        std::cerr << "ERROR: "
                  << (type == GL_VERTEX_SHADER ? "Vertex" : "Fragment")
                  << " shader compilation failed:\n" << log << '\n';
        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

int Shader::getUniformLocation(const std::string& name) const
{
    return glGetUniformLocation(programId, name.c_str());
}
