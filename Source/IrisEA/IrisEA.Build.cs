//Copyright(c) 2024 Electronic Arts Inc.All rights reserved.

using System.IO;
using UnrealBuildTool;
using UnrealBuildTool.Rules;

public class IrisEA : ModuleRules
{
	public IrisEA(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
				Path.Combine(PluginDirectory, "Source", "ThirdParty", "IrisLibrary", "include"),
                Path.Combine(PluginDirectory, "Source", "ThirdParty", "IrisLibrary", "include", "iris"),
                Path.Combine(PluginDirectory, "Source", "ThirdParty", "IrisLibrary", "include", "src"),
                Path.Combine(PluginDirectory, "Source", "ThirdParty", "IrisLibrary", "include", "opencv2"),

				Path.Combine(EngineDirectory, "Plugins", "Media", "PixelCapture", "Source","PixelCapture", "Private"),
                Path.Combine(EngineDirectory, "Plugins", "Media", "PixelCapture", "Source","PixelCapture", "Public"),
            }
		);
		
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"IrisLibrary",
                "Projects",
                "RHI", 
				"RenderCore", 
				"ImageWrapper",
				"InputCore",
				"PixelCapture",
                "PixelCaptureShaders",
                "Renderer",
				"WebRTC"
				// ... add private dependencies that you statically link with here ...	
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);

        PublicDefinitions.AddRange(
           new string[]
           {
                //"ENABLE_SCREEN_CAPTURE"
           });
    }
}
