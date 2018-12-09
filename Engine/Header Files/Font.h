#pragma once

#include <fstream>
#include <Resources\Texture.h>

namespace Engine
{
    class Font
    {
    private:
        struct FontType
        {
            float left, right;
            int size;
        };

        struct VertexType
        {
            Utils::Maths::Vector3 position;
            Utils::Maths::Vector2 texture;
        };

    public:
        Font();
        Font(const Font&) = delete;
        ~Font();

        bool Initialize(char*, WCHAR*);

        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> GetTexture();

        void BuildVertexArray(void*, char*, float, float);

    private:
        bool LoadFontData(char*);
        void ReleaseFontData();
        bool LoadTexture(WCHAR*);

    private:
        static const int kCharacterCount = 95;
        std::array<FontType, kCharacterCount> m_Font;
        std::shared_ptr<Texture> m_Texture;
    };
}

