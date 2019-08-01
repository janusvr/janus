// @generated

using UnrealBuildTool;
using System.Collections.Generic;

public class OculusPlatformSampleEditorTarget : TargetRules
{
	public OculusPlatformSampleEditorTarget(TargetInfo Target) : base (Target)
	{
		Type = TargetType.Editor;
		ExtraModuleNames.Add("OculusPlatformSample");
	}

	//
	// TargetRules interface.
	//

	/*
	public override void SetupBinaries(
		TargetInfo Target,
		ref List<UEBuildBinaryConfiguration> OutBuildBinaryConfigurations,
		ref List<string> OutExtraModuleNames
		)
	{
		OutExtraModuleNames.AddRange( new string[] { "OculusPlatformSample" } );
	}
	*/
}
