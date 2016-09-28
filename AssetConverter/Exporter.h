#pragma once

#include "Data.h"
#include <stdio.h>
#include <iostream>
#include <fstream>

class Exporter
{
public:
    Exporter(std::string outputFilePath, const SceneData& sceneData);

private:
    void WriteHeaderToFile();
    void WriteSceneNodesToFile();
    void WriteMaterialsToFile();
    void WriteTexturesToFile();

    template <typename T>
    void WriteToFile(T* data, size_t count = 1)
    {
        m_outputFile.write((char*)data, sizeof(T) * count);
    }
private:
    SceneData m_sceneData;
    std::ofstream m_outputFile;
};