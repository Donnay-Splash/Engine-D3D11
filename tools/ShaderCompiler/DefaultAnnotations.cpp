#include "DefaultAnnotations.h"

#include <cctype>
#include <cstdlib>
#include <sstream>

namespace ShaderCompiler {
namespace {

std::string Trim(const std::string &text)
{
    std::size_t begin = 0;
    std::size_t end = text.size();
    while (begin < end && std::isspace(static_cast<unsigned char>(text[begin]))) {
        ++begin;
    }
    while (end > begin && std::isspace(static_cast<unsigned char>(text[end - 1]))) {
        --end;
    }
    return text.substr(begin, end - begin);
}

bool IsIdentifierChar(char c)
{
    return std::isalnum(static_cast<unsigned char>(c)) || c == '_';
}

// The identifier a declaration introduces: the last one before `:`, `[`, `;` or `=`.
std::string DeclaredName(std::string declaration)
{
    for (char stop : {':', '[', '=', ';'}) {
        const std::size_t position = declaration.find(stop);
        if (position != std::string::npos) {
            declaration.resize(position);
        }
    }
    declaration = Trim(declaration);
    std::size_t begin = declaration.size();
    while (begin > 0 && IsIdentifierChar(declaration[begin - 1])) {
        --begin;
    }
    return declaration.substr(begin);
}

bool ParseNumber(const std::string &text, double &value)
{
    char *end = nullptr;
    value = std::strtod(text.c_str(), &end);
    return !text.empty() && end == text.c_str() + text.size();
}

std::string Describe(const DefaultAnnotation &annotation)
{
    return "@default on '" + annotation.name + "' (line " + std::to_string(annotation.line) + ")";
}

} // namespace

bool ParseDefaultAnnotations(const std::string &source, std::vector<DefaultAnnotation> &annotations,
                             std::string &error)
{
    annotations.clear();
    std::istringstream stream(source);
    std::string line;
    int lineNumber = 0;
    while (std::getline(stream, line)) {
        ++lineNumber;
        const std::size_t comment = line.find("//");
        if (comment == std::string::npos) {
            continue;
        }
        const std::size_t tag = line.find("@default", comment);
        if (tag == std::string::npos) {
            continue;
        }
        DefaultAnnotation annotation;
        annotation.line = lineNumber;
        annotation.name = DeclaredName(line.substr(0, comment));
        const std::string where = "line " + std::to_string(lineNumber);
        if (annotation.name.empty()) {
            error = "@default on " + where + " must follow a declaration on the same line.";
            return false;
        }
        const std::size_t open = line.find_first_not_of(" \t", tag + 8);
        const std::size_t close = line.find(')', tag);
        if (open == std::string::npos || line[open] != '(' || close == std::string::npos
            || close < open) {
            error = "Malformed @default on " + where + ": expected @default(value, ...).";
            return false;
        }
        std::stringstream arguments(line.substr(open + 1, close - open - 1));
        std::string argument;
        while (std::getline(arguments, argument, ',')) {
            annotation.arguments.push_back(Trim(argument));
        }
        if (annotation.arguments.empty() || annotation.arguments.front().empty()) {
            error = "Empty @default on " + where + ".";
            return false;
        }
        annotations.push_back(std::move(annotation));
    }
    return true;
}

bool BuildValueDefault(const DefaultAnnotation &annotation, const std::string &type,
                       nlohmann::json &value, std::string &error)
{
    std::size_t count = 1;
    if (type == "float2" || type == "float3" || type == "float4") {
        count = static_cast<std::size_t>(type.back() - '0');
    }
    if (annotation.arguments.size() != count) {
        error = Describe(annotation) + " needs " + std::to_string(count) + " value(s) for type " +
                type + ".";
        return false;
    }
    value = nlohmann::json::array();
    for (const std::string &argument : annotation.arguments) {
        double number = 0.0;
        if (type == "bool" && (argument == "true" || argument == "false")) {
            number = argument == "true" ? 1.0 : 0.0;
        } else if (!ParseNumber(argument, number)) {
            error = Describe(annotation) + " has an invalid value '" + argument + "'.";
            return false;
        }
        if (type == "bool" && number != 0.0 && number != 1.0) {
            error = Describe(annotation) + " must be true, false, 0 or 1.";
            return false;
        }
        if (type == "int" && number != static_cast<long long>(number)) {
            error = Describe(annotation) + " must be an integer.";
            return false;
        }
        value.push_back(number);
    }
    return true;
}

bool BuildTextureDefault(const DefaultAnnotation &annotation, nlohmann::json &value,
                         std::string &error)
{
    const std::string &name = annotation.arguments.front();
    if (annotation.arguments.size() != 1 || (name != "white" && name != "black" && name != "normal")) {
        error = Describe(annotation) + " must be white, black or normal for a texture.";
        return false;
    }
    value = name;
    return true;
}

} // namespace ShaderCompiler
