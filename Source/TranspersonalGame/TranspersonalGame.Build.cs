using UnrealBuildTool;

public class TranspersonalGame : ModuleRules
{
    public TranspersonalGame(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[] 
        { 
            "Core", 
            "CoreUObject", 
            "Engine", 
            "InputCore",
            "HeadMountedDisplay",
            "UMG",
            "Slate",
            "SlateCore",
            "AudioMixer",
            "AudioCapture",
            "SignalProcessing",
            "Niagara",
            "NiagaraCore",
            "GameplayTasks",
            "AIModule",
            "NavigationSystem",
            "PhysicsCore",
            "Chaos",
            "ChaosVehicles",
            "GeometryCollectionEngine",
            "FieldSystemEngine"
        });

        PrivateDependencyModuleNames.AddRange(new string[] 
        {
            "RenderCore",
            "RHI",
            "ApplicationCore",
            "Json",
            "JsonUtilities",
            "HTTP",
            "WebSockets",
            "OnlineSubsystem",
            "OnlineSubsystemUtils",
            "Networking",
            "Sockets",
            "PacketHandler",
            "ReplicationGraph",
            "NetCore",
            "AudioPlatformConfiguration",
            "AudioExtensions",
            "SoundFieldRendering",
            "Synthesis",
            "MetasoundFrontend",
            "MetasoundEngine",
            "WaveTable"
        });

        // Enable optimization for consciousness calculations
        OptimizeCode = CodeOptimization.InShippingBuildsOnly;
        
        // Enable RTTI for consciousness reflection system
        bUseRTTI = true;
        
        // Enable exceptions for quantum field calculations
        bEnableExceptions = true;

        // Consciousness system specific defines
        PublicDefinitions.Add("CONSCIOUSNESS_SYSTEM_ENABLED=1");
        PublicDefinitions.Add("SYNCHRONICITY_ENGINE_ENABLED=1");
        PublicDefinitions.Add("ARCHETYPAL_SYSTEM_ENABLED=1");
        PublicDefinitions.Add("COLLECTIVE_CONSCIOUSNESS_ENABLED=1");
        
        // Performance optimization defines
        PublicDefinitions.Add("CONSCIOUSNESS_HIGH_PRECISION=1");
        PublicDefinitions.Add("QUANTUM_FIELD_CALCULATIONS=1");
        
        // Debug defines for development
        if (Target.Configuration == UnrealTargetConfiguration.Development || 
            Target.Configuration == UnrealTargetConfiguration.Debug)
        {
            PublicDefinitions.Add("CONSCIOUSNESS_DEBUG=1");
            PublicDefinitions.Add("SYNCHRONICITY_DEBUG=1");
        }

        // Platform specific optimizations
        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            PublicDefinitions.Add("CONSCIOUSNESS_PLATFORM_WINDOWS=1");
        }
        else if (Target.Platform == UnrealTargetPlatform.Mac)
        {
            PublicDefinitions.Add("CONSCIOUSNESS_PLATFORM_MAC=1");
        }
        else if (Target.Platform == UnrealTargetPlatform.Linux)
        {
            PublicDefinitions.Add("CONSCIOUSNESS_PLATFORM_LINUX=1");
        }

        // Include paths for consciousness systems
        PublicIncludePaths.AddRange(new string[]
        {
            "TranspersonalGame/Core",
            "TranspersonalGame/Meditation",
            "TranspersonalGame/Synchronicity",
            "TranspersonalGame/Archetypes",
            "TranspersonalGame/Collective",
            "TranspersonalGame/Audio",
            "TranspersonalGame/Visual",
            "TranspersonalGame/Network"
        });
    }
}