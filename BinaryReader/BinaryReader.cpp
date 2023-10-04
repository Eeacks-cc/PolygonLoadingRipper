#include <iostream>
#include <sstream>
#include <Windows.h>

#include "json.hpp"
#include "base64.h"

struct Vector4
{
    float x;
    float y;
    float z;
    float w;
};


struct VectorInt4
{
    int x;
    int y;
    int z;
    int w;
};


char cBuffer[1024] = {0};

char cWatermark[] = { "# keter.tech - polygon loading ripper " };
char cPadding[] = { "\n\n" };

nlohmann::json pJson;

std::string GetPermFilePath()
{
    OPENFILENAMEA m_OpenFileName = { 0 };
    ZeroMemory(&m_OpenFileName, sizeof(OPENFILENAMEA));

    m_OpenFileName.lStructSize = sizeof(OPENFILENAMEA);

    static char m_FilePath[MAX_PATH] = { '\0' };
    m_OpenFileName.lpstrFile = m_FilePath;
    m_OpenFileName.nMaxFile = sizeof(m_FilePath);
    m_OpenFileName.lpstrFilter = "(Binary File)\0*.bin\0";
    m_OpenFileName.Flags = (OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST);

    if (GetOpenFileNameA(&m_OpenFileName) == 0)
        m_OpenFileName.lpstrFile[0] = '\0';

    return m_OpenFileName.lpstrFile;
}

// not implemented yet... no clue
// read 3b859834d2847b665c68_InfoString.json will help you
void RenameJson()
{
    pJson["sr"][0]["position"] = pJson["sr"][0]["po"];
    pJson.erase(pJson["sr"][0]["po"]);

    pJson["sr"][0]["rotation"] = pJson["sr"][0]["ro"];
    pJson.erase(pJson["sr"][0]["ro"]);

    pJson["sr"][0]["scale"] = pJson["sr"][0]["sc"];
    pJson.erase(pJson["sr"][0]["sc"]);
}

int main()
{
    std::cout << "waiting for file..." << std::endl;

    std::string sFilePath = GetPermFilePath();
    if (sFilePath.empty())
    {
        std::cout << "not opening any file" << std::endl;
        return 0;
    }

    if (sFilePath.find(".bin") == std::string::npos)
    {
        std::cout << "i need .bin file!!!" << std::endl;
        return 0;
    }

    std::cout << "Reading file..." << std::endl;

    FILE* hFile = fopen(sFilePath.c_str(), "r");
    if (!hFile)
    {
        std::cout << "Couldnt open the file!" << std::endl;
        return 0;
    }

    int iFileName = sFilePath.find_last_of("\\/");
    std::string sDirectory = sFilePath.substr(0, iFileName + 1);

    std::string sOutputFilePath = sDirectory + sFilePath.substr(iFileName + 1) + ".obj";
    std::string sShaderOutputFilePath = sDirectory + sFilePath.substr(iFileName + 1) + ".shaderInfo.txt";

    fseek(hFile, 0, SEEK_END);
    int iSize = ftell(hFile);
    fseek(hFile, 0, SEEK_SET);

    void* pContent = VirtualAlloc(0x0, iSize, MEM_COMMIT, PAGE_READWRITE);
    if (!pContent)
    {
        std::cout << "allocate memory for " << iSize << " bytes failed!" << std::endl;
        fclose(hFile);
        return 0;
    }

    memset(pContent, 0x0, iSize);

    fread(pContent, 1, iSize, hFile);
    
    fclose(hFile);

    std::cout << "File has been read to memory, processing..." << std::endl;
    
    std::string sRawFile = std::string((char*)pContent);
    VirtualFree(pContent, 0x0, MEM_DECOMMIT);

    std::cout << "Preparing ModelInfo" << std::endl;
    int iJsonBegin = sRawFile.find_first_of('{');
    if (iJsonBegin == std::string::npos)
    {
        std::cout << "ModelInfo not found in data" << std::endl;
        return 0;
    }

    int iJsonEnd = sRawFile.find_last_of('}');
    if (iJsonEnd == std::string::npos)
    {
        std::cout << "Invalid json" << std::endl;
        return 0;
    }

    iJsonEnd++;

    std::string sModelInfo = sRawFile.substr(iJsonBegin, iJsonEnd - iJsonBegin);

    std::cout << "Preparing JSON" << std::endl;
    pJson = nlohmann::json::parse(sModelInfo);
    
    std::cout << "Preparing Content" << std::endl;
    int iContentBegin = sRawFile.find("AA");
    if (iContentBegin == std::string::npos)
    {
        std::cout << "Content not found." << std::endl;
        return 0;
    }

    std::string ;

    std::string sContent = sRawFile.substr(iContentBegin);
   
    std::cout << "Preparing MeshInfo" << std::endl;
    int iMeshEnd = sContent.find("A=");
    if (iMeshEnd == std::string::npos)
    {
        std::cout << "Mesh not found." << std::endl;
        return 0;
    }

    // "A==" = 3
    iMeshEnd += 2;
    if (sContent[iMeshEnd] == '=')
    {
        iMeshEnd++;
    }

    std::string sMesh = sContent.substr(0, iMeshEnd);
    
    std::cout << "Preparing ShaderData" << std::endl;
    std::string sShaderData = sRawFile.substr(iJsonEnd, iContentBegin - iJsonEnd);
    std::vector<std::string> vShaderData;
    if (!sShaderData.empty())
    {
        std::stringstream ss(sShaderData);
        std::string sSplit;
        while (std::getline(ss, sSplit, ','))
        {
            vShaderData.push_back(sSplit);
        }
        FILE* fShaderInfo = fopen(sShaderOutputFilePath.c_str(), "w");
        if (fShaderInfo)
        {
            for (auto& sData : vShaderData)
            {
                fwrite(cBuffer, 1, sprintf(cBuffer, "%s\n", sData.c_str()), fShaderInfo);
            }
            fclose(fShaderInfo);
            std::cout << "Shader Data Dumped!" << std::endl;
        }
        else std::cout << "Dump shader data failed: couldnt create file!" << std::endl;
    } 
    else {
        std::cout << "Shader Data not found, but will continue unpacking..." << std::endl;
    }
    bool bHasShaderInfo = !vShaderData.empty();
    if(bHasShaderInfo)
        std::cout << "Shader Data Found, will flag to assigned material!" << std::endl;

    std::cout << "Decoding Base64..." << std::endl;
    macaron::Base64 pBase64;
    std::vector<unsigned char> vMeshData;
    
    pBase64.Decode(sMesh, vMeshData);

    if (vMeshData.empty())
    {
        std::cout << "Base64 decode issued." << std::endl;
        return 0;
    }
    
    void* pMeshContent = vMeshData.data();

    FILE* hFileWrite = fopen(sOutputFilePath.c_str(), "w");
    if (!hFileWrite)
    {
        std::cout << "fopen for writing file failed!" << std::endl;
        return 0;
    }

    std::cout << "Preparing model info" << std::endl;

    int dwTrianglesOffset = 0;
    
    int dwUVOffset = pJson["sr"][0]["us"].get<int>() * 0x10;
    int dwUVLength = pJson["sr"][0]["ul"].get<int>() * 0x10;
    
    int dwVertexOffset = pJson["sr"][0]["vs"].get<int>() * 0x10;
    int dwVertexLength = pJson["sr"][0]["vl"].get<int>() * 0x10;
    
    int dwNormalOffset = pJson["sr"][0]["ns"].get<int>() * 0x10;
    int dwNormalLength = pJson["sr"][0]["nl"].get<int>() * 0x10;
    
    int dwTangentOffset = pJson["sr"][0]["ta"].get<int>() * 0x10;
    int dwTangentLength = pJson["sr"][0]["tb"].get<int>() * 0x10;

    fwrite(cWatermark, sizeof(cWatermark) - 1, 1, hFileWrite);
    fwrite(cPadding, sizeof(cPadding) - 1, 1, hFileWrite);
    for (int i = dwVertexOffset; i < dwVertexOffset + dwVertexLength; i += sizeof(Vector4))
    {
        Vector4 v4Current = *(Vector4*)((uintptr_t)pMeshContent + i);

        fwrite(cBuffer, sizeof(char), sprintf(cBuffer, "v %f %f %f\n", v4Current.x, v4Current.y, v4Current.z), hFileWrite);
    }
    fwrite(cPadding, sizeof(cPadding) - 1, 1, hFileWrite);
    printf("VERTEX DONE\n");

    for (int i = dwUVOffset; i < dwUVOffset + dwUVLength; i += sizeof(Vector4))
    {
        Vector4 v4Current = *(Vector4*)((uintptr_t)pMeshContent + i);

        fwrite(cBuffer, sizeof(char), sprintf(cBuffer, "vt %f %f\n", v4Current.x, v4Current.y), hFileWrite);
    }
    fwrite(cPadding, sizeof(cPadding) - 1, 1, hFileWrite);
    printf("UV DONE\n");

    for (int i = dwNormalOffset; i < dwNormalOffset + dwNormalLength; i += sizeof(Vector4))
    {
        Vector4 v4Current = *(Vector4*)((uintptr_t)pMeshContent + i);

        fwrite(cBuffer, sizeof(char), sprintf(cBuffer, "vn %f %f %f\n", v4Current.x, v4Current.y, v4Current.z), hFileWrite);
    }
    fwrite(cPadding, sizeof(cPadding) - 1, 1, hFileWrite);
    printf("NORMALS DONE\n");

    int iSubMeshes = pJson["sr"][0]["ss"].size();
    for (int i = 0; i < iSubMeshes; i++)
    {
        int iIndexStartOffset = pJson["sr"][0]["ss"][i]["st"] * 0x10;
        int iIndexLength = pJson["sr"][0]["ss"][i]["il"] * 0x10;

        int iIndexEndOffset = iIndexStartOffset + iIndexLength;

        fwrite(cBuffer, sizeof(char), sprintf(cBuffer, "g sm_%d\n", i), hFileWrite);
        if (bHasShaderInfo)
            fwrite(cBuffer, sizeof(char), sprintf(cBuffer, "usemtl material_%d_%s\n", i, vShaderData[pJson["sm"][i]["sn"]].c_str()), hFileWrite);
        else 
            fwrite(cBuffer, sizeof(char), sprintf(cBuffer, "usemtl material_%d\n", i), hFileWrite);

        for (int n = iIndexStartOffset; n < iIndexEndOffset; n += sizeof(Vector4))
        {
            Vector4 v4Current = *(Vector4*)((uintptr_t)pMeshContent + n);

            VectorInt4 v4iConverted;
            v4iConverted.x = static_cast<int>(floor(v4Current.x));
            v4iConverted.y = static_cast<int>(floor(v4Current.y));
            v4iConverted.z = static_cast<int>(floor(v4Current.z));
            v4iConverted.w = static_cast<int>(floor(v4Current.w));


            fwrite(cBuffer, sizeof(char),
                sprintf(cBuffer, "f %d/%d/%d %d/%d/%d %d/%d/%d\n",
                    v4iConverted.x + 1, v4iConverted.x + 1, v4iConverted.x + 1,
                    v4iConverted.y + 1, v4iConverted.y + 1, v4iConverted.y + 1,
                    v4iConverted.z + 1, v4iConverted.z + 1, v4iConverted.z + 1),
                hFileWrite);

        }
        fwrite(cPadding, sizeof(cPadding) - 1, 1, hFileWrite);
    }

    printf("TRIANGLES DONE\n");

    fclose(hFileWrite);
    printf("ALL DONE\n");
    /*
    std::cout << "Renaming JSON" << std::endl;
    RenameJson();
    */
    system("pause");
}
