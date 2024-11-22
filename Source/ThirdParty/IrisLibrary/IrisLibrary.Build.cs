// Copyright Epic Games, Inc. All Rights Reserved.

using System;
using System.IO;
using System.IO.Compression;
using UnrealBuildTool;

public class IrisLibrary : ModuleRules
{
    public IrisLibrary(ReadOnlyTargetRules Target) : base(Target)
    {
        Type = ModuleType.External;

        PublicSystemIncludePaths.Add(Path.Combine(ModuleDirectory, "include"));
        PublicSystemIncludePaths.Add(Path.Combine(ModuleDirectory, "include", "nlohmann"));
        PublicSystemIncludePaths.Add(Path.Combine(ModuleDirectory, "include", "opencv2"));
        PublicSystemIncludePaths.Add(Path.Combine(ModuleDirectory, "include", "spdlog"));
        PublicSystemIncludePaths.Add(Path.Combine(ModuleDirectory, "include", "fmt"));


        PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "include"));
        PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "include", "nlohmann"));
        PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "include", "opencv2"));
        PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "include", "spdlog"));
        PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "include", "fmt"));


        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            //Add the Iris dependencies
            string IrisDependenciesPath = Path.Combine(ModuleDirectory, "Win64");

            //LIBs
            string[] LIBFiles = Directory.GetFiles(IrisDependenciesPath, "*.lib", SearchOption.TopDirectoryOnly);
            foreach (string LibFile in LIBFiles)
            {
                string fileName = Path.GetFileName(LibFile);
                Console.WriteLine("Current Lib load: {0}", fileName);
                PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "Win64", LibFile));
            }

            //DLLs
            string[] DLLFiles = Directory.GetFiles(IrisDependenciesPath, "*.dll", SearchOption.TopDirectoryOnly);
            foreach (string DLLFile in DLLFiles)
            {
                string fileName = Path.GetFileName(DLLFile);
                Console.WriteLine("Current DLL load: {0}", fileName);

                string binariesPath = Path.Combine("$(BinaryOutputDir)/", fileName);

                // Remove old .dll files
                if (File.Exists(binariesPath))
                {
                    File.Delete(binariesPath);
                }

                PublicDelayLoadDLLs.Add(DLLFile);
                if (Target.Configuration == UnrealTargetConfiguration.Development ||
                    Target.Configuration == UnrealTargetConfiguration.DebugGame ||
                    Target.Configuration == UnrealTargetConfiguration.Shipping)
                { 
                    RuntimeDependencies.Add(binariesPath, DLLFile);
                }
            }
            string settingsPath = Path.Combine(IrisDependenciesPath, "appsettings.json");
            //Copy appsettings into the .exe directory
            RuntimeDependencies.Add(settingsPath, StagedFileType.NonUFS);

        }
        else
        {
            throw new Exception(Target.Platform.ToString() + " is not supported by the IrisEA plugin");
        }

        PublicDefinitions.AddRange(
           new string[]
           {
                "IRIS_SHARED",
                "UTILS_SHARED",
                "SPDLOG_COMPILED_LIB",
                "SPDLOG_SHARED_LIB"
           });
    }
}
