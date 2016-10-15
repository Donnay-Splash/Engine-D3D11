#pragma once

#include "Font.h"
#include "FontShader.h"

namespace Engine
{
    class Text
    {
    private:
        struct SentenceType
        {
            ID3D11Buffer *vertexBuffer, *indexBuffer;
            int vertexCount, indexCount, maxLength;
            float red, green, blue;
        };

        struct VertexType
        {
            Utils::Maths::Vector3 position;
            Utils::Maths::Vector2 texture;
        };

    public:
        Text();
        ~Text();

        bool Initialize(ID3D11Device*, ID3D11DeviceContext*, HWND, int, int, Utils::Maths::Matrix);
        void Shutdown();
        bool Render(ID3D11DeviceContext*, FontShader*, Utils::Maths::Matrix, Utils::Maths::Matrix);

        bool SetVideoCardInfo(char*, int, ID3D11DeviceContext*);
        bool SetFps(int, ID3D11DeviceContext*);
        bool SetCpu(int, ID3D11DeviceContext*);
        bool SetCameraPosition(float, float, float, ID3D11DeviceContext*);
        bool SetCameraRotation(float, float, float, ID3D11DeviceContext*);

    private:
        bool InitializeSentence(SentenceType**, int, ID3D11Device*);
        bool UpdateSentence(SentenceType*, char*, int, int, float, float, float, ID3D11DeviceContext*);
        void ReleaseSentence(SentenceType**);
        bool RenderSentence(SentenceType*, ID3D11DeviceContext*, FontShader*, Utils::Maths::Matrix, Utils::Maths::Matrix);

    private:
        int m_screenWidth, m_screenHeight;
        Utils::Maths::Matrix m_baseViewMatrix;
        Font* m_Font;
        SentenceType *m_sentence1, *m_sentence2, *m_sentence3, *m_sentence4, *m_sentence5;
        SentenceType *m_sentence6, *m_sentence7, *m_sentence8, *m_sentence9, *m_sentence10;
    };
}

