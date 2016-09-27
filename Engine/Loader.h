#pragma once

#include <assimp\Importer.hpp>
#include <assimp\scene.h>
#include <assimp\postprocess.h>
#include "pch.h"
#include "d3dclass.h"
#include "Scene.h"

class Loader
{
public:
    using Ptr = std::shared_ptr<Loader>;
    Loader();
    ~Loader();

    void LoadFile(const std::string& filePath);
private:
    void LoadScene();

private:
    std::unique_ptr<Assimp::Importer> m_importer;
    D3DClass::Ptr m_d3dClass;
    Scene::Ptr m_scene;
};