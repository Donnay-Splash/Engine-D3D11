#include "stdafx.h"
#include <iostream>
#include "Importer.h"
#include <Utils\Loader\Exporter.h>
#include <Utils\Loader\MikeLoader.h>

using namespace Utils::Loader;

std::string filePath;
std::string fileDirectory;
std::string fileName;
std::string fileExtension;

std::string outputDirectory;
std::string outputFilePath;
std::string AssetConverterHelp =
"AssetConverter.exe expects the following switches:\n"
"-file <path to file to convert> - This is the only expected argument. If you do not specify an output file path "
"The generated .mike file will be saved alongside the given file\n"
"-output <path to output directory> - If supplied this path will be where the generated .mike file is saved to.\n"
"-option <Option to enable> - Will enable the given option if it exists. Possible values are: \n "
"MergeMeshes - If found all meshes that share the same material will be merged together into a single mesh";

static const std::string kMergeMeshes = "MergeMeshes";

bool CheckFilePath();

int main(int argc, char* argv[])
{
    bool correctArguments = true;
    bool mergeMeshes = false;
    
    // We expect at least two arguments. 1. The path of the executable 2. The -file identifier 3. The path of the file to convert.
    // The number of arguments should also be odd as we expect a switch followed by the value
    if (argc >= 3 && (argc & 1) != 0)
    {
        for (int i = 1; i < argc; i++)
        {
            auto argument = std::string(argv[i]);
            // We expect a '-' to start a switch
            if (argument == "-file")
            {
                filePath = std::string(argv[++i]);
            }
            else if (argument == "-output")
            {
                outputDirectory = std::string(argv[++i]);
            }
            else if (argument == "-option")
            {
                auto value = std::string(argv[++i]);
                if (value == kMergeMeshes)
                {
                    mergeMeshes = true;
                }
            }
            else
            {
                // Invalid argument print out usage.
                std::cout << "Unrecognised value: " << argument << '\n';
                correctArguments = false;
            }
        }
    }

    correctArguments &= CheckFilePath();

    if (correctArguments && !filePath.empty())
    {
        Importer::ImportOptions options;
        options.MergeMeshesByMaterial = mergeMeshes;
        auto importer = std::make_unique<Importer>(options);
        auto error = importer->ReadFile(fileDirectory, fileName + '.' + fileExtension);
        if (error.empty())
        {
            auto sceneData = importer->GetSceneData();
            Exporter exporter(outputFilePath, sceneData);

            // Test by loading data back in
            /*MikeLoader loader;
            loader.LoadFile(outputFilePath);
            auto loadedSceneData = loader.GetSceneData();*/
        }
        else
        {
            std::cout << "Failed to convert model with given error: " << error << '\n';
        }
    }
    else
    {
        std::cout << AssetConverterHelp << '\n';
    }

    return 0;
}

bool CheckFilePath()
{
    // We need to check that the file path we were given is a valid one.
    std::ifstream test(filePath);
    if (!test)
    {
        std::cout << "Cannot open given file. Either the file does not exist or the given path is invalid" << '\n';
        return false;
    }
    test.close();

    // Get the output directory from the input file
    auto lastSlash = filePath.find_last_of("/\\");
    // We could be given a file in the local directory
    // if so then output directory should stay empty.
    if (lastSlash != filePath.npos)
    {
        fileDirectory = filePath.substr(0, lastSlash);
    }

    fileName = filePath.substr(lastSlash + 1);
    auto extensionPosition = fileName.find_first_of('.');
    fileExtension = fileName.substr(extensionPosition + 1);
    fileName = fileName.substr(0, extensionPosition);
    

    // Check that the given output directory exists.
    if (!outputDirectory.empty())
    {
        DWORD fileType = GetFileAttributesA(outputDirectory.c_str());
        if (fileType == INVALID_FILE_ATTRIBUTES)
        {
            std::cout << outputDirectory << " is not a valid directory.";
            return false;
        }
        else if ((fileType & FILE_ATTRIBUTE_DIRECTORY) == 0)
        {
            std::cout << outputDirectory << " is not a valid directory.";
            return false;
        }
    }
    else
    {
        outputDirectory = fileDirectory;
    }

    outputFilePath = outputDirectory.empty() ? fileName + ".mike" : outputDirectory + "\\" + fileName + ".mike";

    return true;
}