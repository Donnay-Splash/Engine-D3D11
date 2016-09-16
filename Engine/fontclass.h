#pragma once

//////////////
// INCLUDES //
//////////////
#include <fstream>

///////////////////////
// MY CLASS INCLUDES //
///////////////////////
#include "Texture.h"

class FontClass
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
    FontClass();
    FontClass(const FontClass&);
    ~FontClass();

    bool Initialize(ID3D11Device*, char*, WCHAR*);

    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> GetTexture();

    void BuildVertexArray(void*, char*, float, float);

private:
    bool LoadFontData(char*);
    void ReleaseFontData();
    bool LoadTexture(ID3D11Device*, WCHAR*);

private:
    static const int kCharacterCount = 95;
    std::array<FontType, kCharacterCount> m_Font;
    std::shared_ptr<Texture> m_Texture;
};

