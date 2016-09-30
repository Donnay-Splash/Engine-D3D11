#pragma once
#include "Data.h"
#include <Utils\DirectxHelpers\EngineHelpers.h>
#include <iostream>
#include <fstream>

namespace Utils
{
    namespace Loader
    {
        class MikeLoader
        {
        public:
            MikeLoader();

            SceneData LoadFile(std::string filePath);
            SceneData LoadFile(const uint8_t* data, uint64_t byteCount);
            SceneData GetSceneData() const { return m_sceneData; }
        private:
            // For reading from file;
            void LoadDataFromBuffer();
            void LoadHeaderFromBuffer();
            void LoadSceneNodesFromBuffer();
            void LoadMaterialsFromBuffer();
            void LoadTexturesFromBuffer();
            bool AtEndOfBuffer();

            void LoadFileToBuffer(std::ifstream& file);

            // This is crude and ugly for now.
            // TODO: Tidy this function
            template<typename T>
            void ReadFromBuffer(T* dataOut, size_t count = 1)
            {
                auto bytesToRead = sizeof(T) * count;
                EngineAssert((m_readBytes + bytesToRead) <= m_buffer.size());
                memcpy((uint8_t*)dataOut, m_buffer.data() + m_readBytes, bytesToRead);
                m_readBytes += bytesToRead;
            }

        private:
            SceneData m_sceneData;

            std::vector<uint8_t> m_buffer;
            uint64_t m_readBytes;

            uint32_t m_currentDataID;
            uint32_t m_versionNumer;
        };

    } // End namespace Loader
} // End namespace Utils
