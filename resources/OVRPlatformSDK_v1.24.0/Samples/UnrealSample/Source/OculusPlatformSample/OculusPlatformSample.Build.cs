// @generated

using UnrealBuildTool;

public class OculusPlatformSample : ModuleRules
{
	public OculusPlatformSample(ReadOnlyTargetRules Target) : base (Target)
	{
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "UMG", "Slate", "SlateCore", "OnlineSubsystem" });

		//PrivateDependencyModuleNames.AddRange(new string[] {  });
		
		// Uncomment if you are using online features
		PrivateDependencyModuleNames.Add("OnlineSubsystem");
		PrivateDependencyModuleNames.Add("LibOVRPlatform");
		if ((Target.Platform == UnrealTargetPlatform.Win32) || (Target.Platform == UnrealTargetPlatform.Win64))
		{
			DynamicallyLoadedModuleNames.Add("OnlineSubsystemOculus");
			
			PublicDelayLoadDLLs.Add("LibOVRPlatform64_1.dll");

		}
	}
}
