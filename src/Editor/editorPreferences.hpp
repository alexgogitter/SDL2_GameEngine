#include <unordered_map>
#include <string>
#include <cstring>
#include <any>       // C++17 required for std::any
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp> // Include the nlohmann JSON library

using json = nlohmann::json;


class EditorPreferences
{
private:
    // Use std::any instead of void* for type safety and automatic memory management
    std::unordered_map<std::string, std::any> preferences;

    EditorPreferences() = default;
    ~EditorPreferences() = default;

    EditorPreferences(const EditorPreferences&) = delete;
    EditorPreferences& operator=(const EditorPreferences&) = delete;

public:
    static EditorPreferences& getInstance()
    {
        static EditorPreferences instance;
        return instance;
    }

    template<typename T>
    void setPreference(const std::string& key, const T& value)
    {
        preferences[key] = value; // std::any handles the copy and memory management
    }

    template<typename T>
    T getPreference(const std::string& key) const
    {
        auto it = preferences.find(key);
        if (it != preferences.end() && it->second.type() == typeid(T))
        {
            return std::any_cast<T>(it->second);
        }
        return T();
    }

    bool hasPreference(const std::string& key) const
    {
        return preferences.find(key) != preferences.end();
    }

    // --- Serialization ---
    void saveToFile(const std::string& filename) const
    {
        json j;

        for (const auto& [key, value] : preferences)
        {
            // Check the type of the std::any and serialize accordingly
            if (value.type() == typeid(bool)) {
                j[key] = std::any_cast<bool>(value);
            }
            else if (value.type() == typeid(int)) {
                j[key] = std::any_cast<int>(value);
            }
            else if (value.type() == typeid(float)) {
                j[key] = std::any_cast<float>(value);
            }
            else if (value.type() == typeid(double)) {
                j[key] = std::any_cast<double>(value);
            }
            else if (value.type() == typeid(std::string)) {
                j[key] = std::any_cast<std::string>(value);
            }
            // Add more types here as needed (e.g., ImVec4, std::vector)
        }

        std::ofstream file(filename);
        if (file.is_open()) {
            file << j.dump(4); // 4 spaces for pretty formatting
            file.close();
        }
    }

    // --- Deserialization ---
    void loadFromFile(const std::string& filename)
    {
        std::ifstream file(filename);
        if (!file.is_open()) return;

        json j;
        file >> j;
        file.close();

        for (auto& [key, value] : j.items())
        {
            if (value.is_boolean()) {
                setPreference(key, value.get<bool>());
            }
            else if (value.is_number_integer()) {
                setPreference(key, value.get<int>());
            }
            else if (value.is_number_float()) {
                setPreference(key, value.get<float>());
            }
            else if (value.is_string()) {
                setPreference(key, value.get<std::string>());
            }
            // Add more type checks here
        }
    }
};