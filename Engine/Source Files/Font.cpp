#include "pch.h"
#include "Font.h"

using namespace std;
namespace Engine
{
    Font::Font()
    {
    }


    Font::~Font()
    {
    }


    bool Font::Initialize(char* fontFilename, WCHAR* textureFilename)
    {
        bool result;


        // Load in the text file containing the font data.
        result = LoadFontData(fontFilename);
        if (!result)
        {
            return false;
        }

        // Load the texture that has the font characters on it.
        result = LoadTexture(textureFilename);
        if (!result)
        {
            return false;
        }

        return true;
    }


    bool Font::LoadFontData(char* filename)
    {
        ifstream fin;
        int i;
        char temp;


        // Read in the font size and spacing between chars.
        fin.open(filename);
        if (fin.fail())
        {
            return false;
        }

        // Read in the 95 used ascii characters for text.
        for (i = 0; i < kCharacterCount; i++)
        {
            fin.get(temp);
            while (temp != ' ')
            {
                fin.get(temp);
            }
            fin.get(temp);
            while (temp != ' ')
            {
                fin.get(temp);
            }

            fin >> m_Font[i].left;
            fin >> m_Font[i].right;
            fin >> m_Font[i].size;
        }

        // Close the file.
        fin.close();

        return true;
    }


    void Font::ReleaseFontData()
    {
    }


    bool Font::LoadTexture(WCHAR* filename)
    {
        // Create the texture object.
        m_Texture = Texture::CreateTextureFromFile(filename);

        return true;
    }

    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> Font::GetTexture()
    {
		IMPLEMENT_FOR_DX12(return m_Texture->GetSRV();)
		return nullptr;
    }


    void Font::BuildVertexArray(void* vertices, char* sentence, float drawX, float drawY)
    {
        VertexType* vertexPtr;
        int numLetters, index, i, letter;


        // Coerce the input vertices into a VertexType structure.
        vertexPtr = (VertexType*)vertices;

        // Get the number of letters in the sentence.
        numLetters = (int)strlen(sentence);

        // Initialize the index to the vertex array.
        index = 0;

        // Draw each letter onto a quad.
        for (i = 0; i < numLetters; i++)
        {
            letter = ((int)sentence[i]) - 32;

            // If the letter is a space then just move over three pixels.
            if (letter == 0)
            {
                drawX = drawX + 3.0f;
            }
            else
            {
                // First triangle in quad.
                vertexPtr[index].position = Utils::Maths::Vector3(drawX, drawY, 0.0f);  // Top left.
                vertexPtr[index].texture = Utils::Maths::Vector2(m_Font[letter].left, 0.0f);
                index++;

                vertexPtr[index].position = Utils::Maths::Vector3((drawX + m_Font[letter].size), (drawY - 16), 0.0f);  // Bottom right.
                vertexPtr[index].texture = Utils::Maths::Vector2(m_Font[letter].right, 1.0f);
                index++;

                vertexPtr[index].position = Utils::Maths::Vector3(drawX, (drawY - 16), 0.0f);  // Bottom left.
                vertexPtr[index].texture = Utils::Maths::Vector2(m_Font[letter].left, 1.0f);
                index++;

                // Second triangle in quad.
                vertexPtr[index].position = Utils::Maths::Vector3(drawX, drawY, 0.0f);  // Top left.
                vertexPtr[index].texture = Utils::Maths::Vector2(m_Font[letter].left, 0.0f);
                index++;

                vertexPtr[index].position = Utils::Maths::Vector3(drawX + m_Font[letter].size, drawY, 0.0f);  // Top right.
                vertexPtr[index].texture = Utils::Maths::Vector2(m_Font[letter].right, 0.0f);
                index++;

                vertexPtr[index].position = Utils::Maths::Vector3((drawX + m_Font[letter].size), (drawY - 16), 0.0f);  // Bottom right.
                vertexPtr[index].texture = Utils::Maths::Vector2(m_Font[letter].right, 1.0f);
                index++;

                // Update the x location for drawing by the size of the letter and one pixel.
                drawX = drawX + m_Font[letter].size + 1.0f;
            }
        }

        return;
    }
}