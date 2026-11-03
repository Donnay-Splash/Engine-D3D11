#pragma once

#include <map>
#include <optional>
#include <string>
#include <string_view>

namespace Engine {
struct Error;

// Names one entry point of a compiled shader source, written "<source>:<entry>" in config.
struct ShaderRef
{
    std::string source;
    std::string entry;
};

// Either a graphics program (a vertex entry and an optional fragment entry, which may come
// from different sources; depth-only programs have no fragment) or a compute program (a
// single compute entry).
struct ShaderProgramDesc
{
    std::string name;
    std::optional<ShaderRef> vertex;
    std::optional<ShaderRef> fragment;
    std::optional<ShaderRef> compute;
};

// Programs parsed from engine/shaders/ShaderPrograms.cfg:
//   [program]
//   name = Mesh
//   vertex = Scene:mesh_vertex
//   fragment = Scene:mesh_fragment
// A compute program uses "compute = <source>:<entry>" instead. <source> is the HLSL file name
// without extension. Only entries named here are compiled.
// "#" starts a comment.
class ShaderProgramCatalog
{
public:
    static bool Parse(std::string_view text, ShaderProgramCatalog &catalog, Error &error);

    const ShaderProgramDesc *Find(const std::string &name) const noexcept;
    const std::map<std::string, ShaderProgramDesc> &Programs() const noexcept
    {
        return m_Programs;
    }

private:
    std::map<std::string, ShaderProgramDesc> m_Programs;
};

} // namespace Engine
