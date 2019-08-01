// @generated

using UnrealBuildTool;
using System.Collections.Generic;

public class OculusPlatformSampleTarget : TargetRules
{
	public OculusPlatformSampleTarget(TargetInfo Target) : base (Target)
	{
		Type = TargetType.Game;
		ExtraModuleNames.Add("OculusPlatformSample");
	}

	//
	// TargetRules interface.
	//

	/*public override void SetupBinaries(
		TargetInfo Target,
		ref List<UEBuildBinaryConfiguration> OutBuildBinaryConfigurations,
		ref List<string> OutExtraModuleNames
		)
	{
		OutExtraModuleNames.AddRange( new string[] { "OculusPlatformSample" } );
	}
	*/
}
