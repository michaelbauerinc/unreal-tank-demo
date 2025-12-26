using UnrealBuildTool;

public class Sandbox : ModuleRules
{
	public Sandbox(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"PhysicsCore"
		});

		PublicIncludePaths.AddRange(new string[] {
			"Sandbox",
			"Sandbox/Pawns",
			"Sandbox/Components",
			"Sandbox/Input",
			"Sandbox/UI",
			"Sandbox/Projectiles"
		});
	}
}
