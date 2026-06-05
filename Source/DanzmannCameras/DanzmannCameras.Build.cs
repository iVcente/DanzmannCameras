// Copyright (C) 2026 Vicente Danzmann. All Rights Reserved.

using UnrealBuildTool;

public class DanzmannCameras : ModuleRules
{
	public DanzmannCameras(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"DanzmannInput",
				"Engine",
				"EnhancedInput",
				"GameplayTags",
				"ModularGameplay"
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"InputCore"
			}
		);
	}
}
