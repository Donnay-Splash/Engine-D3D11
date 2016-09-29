#pragma once
#include "Data.h"
#include <iostream>
#include <fstream>

namespace Utils
{
    namespace Loader
    {
        class Loader
        {
        public:
            Loader();

            SceneData LoadFile(std::string filePath);
            SceneData GetSceneData() const { return m_sceneData; }
        private:
            void LoadHeaderFromFile();
            void LoadSceneNodesFromFile();
            void LoadMaterialsFromFile();
            void LoadTexturesFromFile();

            bool AtEndOfFile();


            template<typename T>
            void ReadFromFile(T* dataOut, size_t count = 1)
            {
                m_file.read((char*)dataOut, sizeof(T) * count);
            }

        private:
            SceneData m_sceneData;
            std::ifstream m_file;
            uint32_t m_currentDataID;
            uint32_t m_versionNumer;
        };

    } // End namespace Loader
} // End namespace Utils
