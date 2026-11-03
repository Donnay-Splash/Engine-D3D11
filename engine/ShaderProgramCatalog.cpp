#include "ShaderProgramCatalog.h"

#include "Core/Types.h"

#include <utility>
#include <vector>

namespace Engine {
namespace {

std::string trim(std::string_view text)
{
    const auto begin = text.find_first_not_of(" \t\r");
    if (begin == std::string_view::npos) {
        return {};
    }
    return std::string(text.substr(begin, text.find_last_not_of(" \t\r") - begin + 1));
}

bool parseRef(const std::string &value, ShaderRef &ref)
{
    const auto colon = value.find(':');
    if (colon == std::string::npos || colon == 0 || colon + 1 == value.size()
        || value.find(':', colon + 1) != std::string::npos) {
        return false;
    }
    ref.source = value.substr(0, colon);
    ref.entry = value.substr(colon + 1);
    return true;
}

} // namespace

bool ShaderProgramCatalog::Parse(
    std::string_view text, ShaderProgramCatalog &catalog, Error &error)
{
    std::vector<ShaderProgramDesc> programs;
    int lineNumber = 0;
    const auto fail = [&](const std::string &message) {
        error.message = "ShaderPrograms.cfg:" + std::to_string(lineNumber) + ": " + message;
        return false;
    };

    while (!text.empty()) {
        const auto newline = text.find('\n');
        std::string_view rawLine = text.substr(0, newline);
        text = newline == std::string_view::npos ? std::string_view() : text.substr(newline + 1);
        ++lineNumber;

        rawLine = rawLine.substr(0, rawLine.find('#'));
        const std::string line = trim(rawLine);
        if (line.empty()) {
            continue;
        }
        if (line == "[program]") {
            programs.emplace_back();
            continue;
        }
        const auto equals = line.find('=');
        if (equals == std::string::npos || programs.empty()) {
            return fail("expected '[program]' or 'key = value'");
        }
        const std::string key = trim(line.substr(0, equals));
        const std::string value = trim(line.substr(equals + 1));
        ShaderProgramDesc &program = programs.back();

        if (key == "name") {
            program.name = value;
        } else if (key == "vertex" || key == "fragment" || key == "compute") {
            ShaderRef ref;
            if (!parseRef(value, ref)) {
                return fail(key + " must be '<source>:<entry>'");
            }
            if (key == "vertex") {
                program.vertex = std::move(ref);
            } else if (key == "compute") {
                program.compute = std::move(ref);
            } else {
                program.fragment = std::move(ref);
            }
        } else {
            return fail("unknown program key '" + key + "'");
        }
    }

    ShaderProgramCatalog parsed;
    for (ShaderProgramDesc &program : programs) {
        if (program.name.empty()) {
            error.message = "Every [program] needs a name.";
            return false;
        }
        const bool graphics = program.vertex.has_value();
        if (graphics == program.compute.has_value() || (program.compute && program.fragment)) {
            error.message = "Program '" + program.name
                + "' needs either a vertex (and optional fragment) entry or a compute entry.";
            return false;
        }
        const std::string name = program.name;
        if (!parsed.m_Programs.emplace(name, std::move(program)).second) {
            error.message = "Duplicate shader program '" + name + "'.";
            return false;
        }
    }
    catalog = std::move(parsed);
    return true;
}

const ShaderProgramDesc *ShaderProgramCatalog::Find(const std::string &name) const noexcept
{
    const auto found = m_Programs.find(name);
    return found == m_Programs.end() ? nullptr : &found->second;
}

} // namespace Engine
