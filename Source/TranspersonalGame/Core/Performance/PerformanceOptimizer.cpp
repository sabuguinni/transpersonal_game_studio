// Copyright Transpersonal Game Studio. All Rights Reserved.

#include "PerformanceOptimizer.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "HAL/IConsoleManager.h"
#include "Stats/StatsHierarchical.h"
#include "RenderingThread.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "GameFramework/GameUserSettings.h"
#include "Engine/GameViewportClient.h"
#include "RHI.h"
#include "HAL/PlatformMemory.h"
#include "Engine/StaticMesh.h"
#include "Components/StaticMeshComponent.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "AI/NavigationSystemBase.h"
#include "Chaos/ChaosEngineInterface.h"

DEFINE_LOG_CATEGORY(LogPerformanceOptimizer);

// Console variables for performance optimization
static TAutoConsoleVariable<int32> CVarPerformanceQuality(
    TEXT("tp.Performance.Quality"),
    2,
    TEXT("Performance quality level (0=Low, 1=Medium, 2=High, 3=Epic)"),
    ECVF_Scalability | ECVF_RenderThreadSafe
);

static TAutoConsoleVariable<bool> CVarAutoPerformanceScaling(
    TEXT("tp.Performance.AutoScaling"),
    true,
    TEXT("Enable automatic performance scaling based on frame time"),
    ECVF_Scalability
);

static TAutoConsoleVariable<float> CVarTargetFrameTime(
    TEXT("tp.Performance.TargetFrameTime"),
    16.67f,
    TEXT("Target frame time in milliseconds (16.67ms = 60fps)"),
    ECVF_Scalability
);

static TAutoConsoleVariable<int32> CVarMaxPhysicsBodies(
    TEXT("tp.Performance.MaxPhysicsBodies"),
    500,
    TEXT("Maximum active physics bodies"),
    ECVF_Scalability
);

static TAutoConsoleVariable<int32> CVarMaxAIAgents(
    TEXT("tp.Performance.MaxAIAgents"),
    100,
    TEXT("Maximum AI agents to simulate"),
    ECVF_Scalability
);

static TAutoConsoleVariable<float> CVarLODDistanceMultiplier(
    TEXT("tp.Performance.LODDistanceMultiplier"),
    1.0f,
    TEXT("LOD distance multiplier for performance scaling"),
    ECVF_Scalability
);

UPerformanceOptimizer::UPerformanceOptimizer()
{
    CurrentQualityLevel = EPerformanceQualityLevel::High;
    bAutoPerformanceScaling = true;
    bInitialized = false;
    FrameTimeHistory.SetNum(60); // Track last 60 frames
    FrameTimeHistoryIndex = 0;
    LastOptimizationTime = 0.0;
    OptimizationCooldown = 2.0; // 2 seconds between optimizations
}

void UPerformanceOptimizer::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogPerformanceOptimizer, Log, TEXT("Performance Optimizer initialized"));
    
    // Initialize platform-specific performance profiles
    InitializePlatformProfiles();
    
    // Set up performance monitoring timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            PerformanceMonitorTimer,
            this,
            &UPerformanceOptimizer::UpdatePerformanceMetrics,
            0.1f, // Update every 100ms
            true
        );
    }
}

void UPerformanceOptimizer::Deinitialize()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(PerformanceMonitorTimer);
    }
    
    UE_LOG(LogPerformanceOptimizer, Log, TEXT("Performance Optimizer deinitialized"));
    
    Super::Deinitialize();
}

void UPerformanceOptimizer::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);
    
    InitializePerformanceOptimization();
}

UPerformanceOptimizer* UPerformanceOptimizer::Get(const UObject* WorldContext)
{
    if (const UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::LogAndReturnNull))
    {
        return World->GetSubsystem<UPerformanceOptimizer>();
    }
    return nullptr;
}

void UPerformanceOptimizer::InitializePerformanceOptimization()
{
    if (bInitialized)
    {
        return;
    }
    
    UE_LOG(LogPerformanceOptimizer, Log, TEXT("Initializing performance optimization..."));
    
    // Detect current platform and set appropriate profile
    DetectPlatformAndSetProfile();
    
    // Apply initial performance settings
    ApplyPerformanceSettings();
    
    // Initialize performance metrics
    CurrentMetrics = FPerformanceMetrics();
    
    bInitialized = true;
    
    UE_LOG(LogPerformanceOptimizer, Log, TEXT("Performance optimization initialized successfully"));
}

void UPerformanceOptimizer::DetectPlatformAndSetProfile()
{
    FPlatformPerformanceProfile Profile;
    
    // Detect platform and set appropriate defaults
    FString PlatformName = FPlatformProperties::PlatformName();
    
    if (PlatformName == TEXT("Windows") || PlatformName == TEXT("Mac") || PlatformName == TEXT("Linux"))
    {
        // PC Platform - Target 60fps
        Profile.TargetFPS = 60;
        Profile.PhysicsTickBudget = 4.17f;
        Profile.MaxActivePhysicsBodies = 1000;
        Profile.MaxCollisionChecksPerFrame = 20000;
        Profile.MaxAIAgents = 200;
        Profile.MaxVisibleDinosaurs = 100;
        Profile.LODDistanceMultiplier = 1.0f;
        Profile.MemoryBudgetMB = 16384; // 16GB
        Profile.bNaniteEnabled = true;
        Profile.bLumenEnabled = true;
    }
    else
    {
        // Console Platform - Target 30fps
        Profile.TargetFPS = 30;
        Profile.PhysicsTickBudget = 8.33f;
        Profile.MaxActivePhysicsBodies = 500;
        Profile.MaxCollisionChecksPerFrame = 10000;
        Profile.MaxAIAgents = 100;
        Profile.MaxVisibleDinosaurs = 50;
        Profile.LODDistanceMultiplier = 0.8f;
        Profile.MemoryBudgetMB = 8192; // 8GB
        Profile.bNaniteEnabled = true;
        Profile.bLumenEnabled = true;
    }
    
    SetPlatformPerformanceProfile(Profile);
    
    UE_LOG(LogPerformanceOptimizer, Log, TEXT("Platform detected: %s, Target FPS: %d"), *PlatformName, Profile.TargetFPS);
}

void UPerformanceOptimizer::InitializePlatformProfiles()
{
    // Initialize default platform profiles
    PlatformProfiles.Empty();
    
    // PC High-end profile
    FPlatformPerformanceProfile PCProfile;
    PCProfile.TargetFPS = 60;
    PCProfile.PhysicsTickBudget = 4.17f;
    PCProfile.MaxActivePhysicsBodies = 1000;
    PCProfile.MaxCollisionChecksPerFrame = 20000;
    PCProfile.MaxAIAgents = 200;
    PCProfile.MaxVisibleDinosaurs = 100;
    PCProfile.LODDistanceMultiplier = 1.0f;
    PCProfile.MemoryBudgetMB = 16384;
    PCProfile.bNaniteEnabled = true;
    PCProfile.bLumenEnabled = true;
    
    PlatformProfiles.Add(TEXT("PC_High"), PCProfile);
    
    // Console profile
    FPlatformPerformanceProfile ConsoleProfile;
    ConsoleProfile.TargetFPS = 30;
    ConsoleProfile.PhysicsTickBudget = 8.33f;
    ConsoleProfile.MaxActivePhysicsBodies = 500;
    ConsoleProfile.MaxCollisionChecksPerFrame = 10000;
    ConsoleProfile.MaxAIAgents = 100;
    ConsoleProfile.MaxVisibleDinosaurs = 50;
    ConsoleProfile.LODDistanceMultiplier = 0.8f;
    ConsoleProfile.MemoryBudgetMB = 8192;
    ConsoleProfile.bNaniteEnabled = true;
    ConsoleProfile.bLumenEnabled = true;
    
    PlatformProfiles.Add(TEXT("Console"), ConsoleProfile);
}

void UPerformanceOptimizer::SetPerformanceQualityLevel(EPerformanceQualityLevel QualityLevel)
{
    if (CurrentQualityLevel == QualityLevel)
    {
        return;
    }
    
    CurrentQualityLevel = QualityLevel;
    
    UE_LOG(LogPerformanceOptimizer, Log, TEXT("Setting performance quality level to: %d"), (int32)QualityLevel);
    
    ApplyPerformanceSettings();
    
    // Update console variable
    CVarPerformanceQuality->Set((int32)QualityLevel);
}

void UPerformanceOptimizer::ApplyPerformanceSettings()
{
    // Apply scalability settings based on quality level
    int32 ScalabilityLevel = (int32)CurrentQualityLevel;
    
    // Apply rendering scalability
    static IConsoleVariable* CVarSGResolutionQuality = IConsoleManager::Get().FindConsoleVariable(TEXT("sg.ResolutionQuality"));
    static IConsoleVariable* CVarSGViewDistanceQuality = IConsoleManager::Get().FindConsoleVariable(TEXT("sg.ViewDistanceQuality"));
    static IConsoleVariable* CVarSGAntiAliasingQuality = IConsoleManager::Get().FindConsoleVariable(TEXT("sg.AntiAliasingQuality"));
    static IConsoleVariable* CVarSGShadowQuality = IConsoleManager::Get().FindConsoleVariable(TEXT("sg.ShadowQuality"));
    static IConsoleVariable* CVarSGPostProcessQuality = IConsoleManager::Get().FindConsoleVariable(TEXT("sg.PostProcessQuality"));
    static IConsoleVariable* CVarSGTextureQuality = IConsoleManager::Get().FindConsoleVariable(TEXT("sg.TextureQuality"));
    static IConsoleVariable* CVarSGEffectsQuality = IConsoleManager::Get().FindConsoleVariable(TEXT("sg.EffectsQuality"));
    
    if (CVarSGResolutionQuality) CVarSGResolutionQuality->Set(ScalabilityLevel);
    if (CVarSGViewDistanceQuality) CVarSGViewDistanceQuality->Set(ScalabilityLevel);
    if (CVarSGAntiAliasingQuality) CVarSGAntiAliasingQuality->Set(ScalabilityLevel);
    if (CVarSGShadowQuality) CVarSGShadowQuality->Set(ScalabilityLevel);
    if (CVarSGPostProcessQuality) CVarSGPostProcessQuality->Set(ScalabilityLevel);
    if (CVarSGTextureQuality) CVarSGTextureQuality->Set(ScalabilityLevel);
    if (CVarSGEffectsQuality) CVarSGEffectsQuality->Set(ScalabilityLevel);
    
    // Apply physics settings
    ApplyPhysicsSettings();
    
    // Apply LOD settings
    ApplyLODSettings();
    
    // Apply Nanite/Lumen settings
    ApplyNaniteLumenSettings();
}

void UPerformanceOptimizer::ApplyPhysicsSettings()
{
    // Set physics quality based on platform profile
    static IConsoleVariable* CVarPhysicsQuality = IConsoleManager::Get().FindConsoleVariable(TEXT("p.Chaos.Solver.SubSteps"));
    static IConsoleVariable* CVarCollisionComplexity = IConsoleManager::Get().FindConsoleVariable(TEXT("p.DefaultShapeComplexity"));
    
    int32 PhysicsQuality = FMath::Clamp((int32)CurrentQualityLevel, 0, 3);
    
    if (CVarPhysicsQuality)
    {
        CVarPhysicsQuality->Set(PhysicsQuality + 1); // 1-4 substeps
    }
    
    if (CVarCollisionComplexity)
    {
        CVarCollisionComplexity->Set(PhysicsQuality); // 0-3 complexity
    }
    
    // Set physics tick budget
    CVarTargetFrameTime->Set(1000.0f / PlatformProfile.TargetFPS);
}

void UPerformanceOptimizer::ApplyLODSettings()
{
    // Apply LOD bias based on quality level and platform
    float LODBias = 0.0f;
    
    switch (CurrentQualityLevel)
    {
        case EPerformanceQualityLevel::Low:
            LODBias = 2.0f;
            break;
        case EPerformanceQualityLevel::Medium:
            LODBias = 1.0f;
            break;
        case EPerformanceQualityLevel::High:
            LODBias = 0.0f;
            break;
        case EPerformanceQualityLevel::Epic:
            LODBias = -1.0f;
            break;
        default:
            LODBias = 0.0f;
            break;
    }
    
    // Apply LOD distance multiplier from platform profile
    LODBias *= PlatformProfile.LODDistanceMultiplier;
    
    SetLODBias(LODBias);
}

void UPerformanceOptimizer::ApplyNaniteLumenSettings()
{
    // Enable/disable Nanite based on platform profile and quality level
    static IConsoleVariable* CVarNaniteEnabled = IConsoleManager::Get().FindConsoleVariable(TEXT("r.Nanite"));
    static IConsoleVariable* CVarLumenEnabled = IConsoleManager::Get().FindConsoleVariable(TEXT("r.DynamicGlobalIllumination"));
    
    bool bEnableNanite = PlatformProfile.bNaniteEnabled && (CurrentQualityLevel >= EPerformanceQualityLevel::Medium);
    bool bEnableLumen = PlatformProfile.bLumenEnabled && (CurrentQualityLevel >= EPerformanceQualityLevel::High);
    
    if (CVarNaniteEnabled)
    {
        CVarNaniteEnabled->Set(bEnableNanite ? 1 : 0);
    }
    
    if (CVarLumenEnabled)
    {
        CVarLumenEnabled->Set(bEnableLumen ? 1 : 0);
    }
    
    UE_LOG(LogPerformanceOptimizer, Log, TEXT("Nanite: %s, Lumen: %s"), 
           bEnableNanite ? TEXT("Enabled") : TEXT("Disabled"),
           bEnableLumen ? TEXT("Enabled") : TEXT("Disabled"));
}

void UPerformanceOptimizer::SetPlatformPerformanceProfile(const FPlatformPerformanceProfile& Profile)
{
    PlatformProfile = Profile;
    
    // Update console variables
    CVarTargetFrameTime->Set(1000.0f / Profile.TargetFPS);
    CVarMaxPhysicsBodies->Set(Profile.MaxActivePhysicsBodies);
    CVarMaxAIAgents->Set(Profile.MaxAIAgents);
    CVarLODDistanceMultiplier->Set(Profile.LODDistanceMultiplier);
    
    UE_LOG(LogPerformanceOptimizer, Log, TEXT("Platform performance profile updated - Target FPS: %d"), Profile.TargetFPS);
}

void UPerformanceOptimizer::UpdatePerformanceMetrics()
{
    if (!bInitialized)
    {
        return;
    }
    
    SCOPE_CYCLE_COUNTER(STAT_RenderingTime);
    
    // Update frame time history
    float CurrentFrameTime = FApp::GetDeltaTime() * 1000.0f; // Convert to milliseconds
    FrameTimeHistory[FrameTimeHistoryIndex] = CurrentFrameTime;
    FrameTimeHistoryIndex = (FrameTimeHistoryIndex + 1) % FrameTimeHistory.Num();
    
    // Calculate average frame time
    float AverageFrameTime = 0.0f;
    for (float FrameTime : FrameTimeHistory)
    {
        AverageFrameTime += FrameTime;
    }
    AverageFrameTime /= FrameTimeHistory.Num();
    
    // Update metrics
    CurrentMetrics.CurrentFPS = 1000.0f / AverageFrameTime;
    CurrentMetrics.FrameTimeMS = AverageFrameTime;
    CurrentMetrics.GameThreadTimeMS = FPlatformTime::ToMilliseconds(GGameThreadTime);
    CurrentMetrics.RenderThreadTimeMS = FPlatformTime::ToMilliseconds(GRenderThreadTime);
    CurrentMetrics.GPUTimeMS = FPlatformTime::ToMilliseconds(GGPUFrameTime);
    
    // Update memory usage
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    CurrentMetrics.MemoryUsageMB = MemStats.UsedPhysical / (1024.0f * 1024.0f);
    
    // Update physics metrics
    UpdatePhysicsMetrics();
    
    // Update AI metrics
    UpdateAIMetrics();
    
    // Detect performance bottleneck
    DetectPerformanceBottleneck();
    
    // Auto-scale performance if enabled
    if (bAutoPerformanceScaling)
    {
        AutoScalePerformance();
    }
}

void UPerformanceOptimizer::UpdatePhysicsMetrics()
{
    // Count active physics bodies
    int32 PhysicsBodies = 0;
    if (UWorld* World = GetWorld())
    {
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (Actor && Actor->GetRootComponent())
            {
                if (UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent()))
                {
                    if (PrimComp->IsSimulatingPhysics())
                    {
                        PhysicsBodies++;
                    }
                }
            }
        }
    }
    
    CurrentMetrics.ActivePhysicsBodies = PhysicsBodies;
    CurrentMetrics.PhysicsObjects = PhysicsBodies;
    
    // Update physics time (this would need integration with Chaos physics system)
    CurrentMetrics.PhysicsTimeMS = 0.0f; // Placeholder
}

void UPerformanceOptimizer::UpdateAIMetrics()
{
    // Count active AI agents (this would need integration with AI system)
    CurrentMetrics.ActiveAIAgents = 0; // Placeholder
    CurrentMetrics.AITimeMS = 0.0f; // Placeholder
}

void UPerformanceOptimizer::DetectPerformanceBottleneck()
{
    float TargetFrameTime = 1000.0f / PlatformProfile.TargetFPS;
    
    // Reset bottleneck
    CurrentMetrics.Bottleneck = EPerformanceBottleneck::None;
    
    // Check if we're missing target frame time
    if (CurrentMetrics.FrameTimeMS > TargetFrameTime * 1.1f) // 10% tolerance
    {
        // Determine bottleneck
        if (CurrentMetrics.GameThreadTimeMS > TargetFrameTime * 0.8f)
        {
            CurrentMetrics.Bottleneck = EPerformanceBottleneck::CPU_GameThread;
        }
        else if (CurrentMetrics.RenderThreadTimeMS > TargetFrameTime * 0.8f)
        {
            CurrentMetrics.Bottleneck = EPerformanceBottleneck::CPU_RenderThread;
        }
        else if (CurrentMetrics.GPUTimeMS > TargetFrameTime * 0.8f)
        {
            CurrentMetrics.Bottleneck = EPerformanceBottleneck::GPU;
        }
        else if (CurrentMetrics.PhysicsTimeMS > PlatformProfile.PhysicsTickBudget)
        {
            CurrentMetrics.Bottleneck = EPerformanceBottleneck::Physics;
        }
        else if (CurrentMetrics.MemoryUsageMB > PlatformProfile.MemoryBudgetMB * 0.9f)
        {
            CurrentMetrics.Bottleneck = EPerformanceBottleneck::Memory;
        }
    }
}

void UPerformanceOptimizer::AutoScalePerformance()
{
    double CurrentTime = FPlatformTime::Seconds();
    
    // Only auto-scale every few seconds to avoid oscillation
    if (CurrentTime - LastOptimizationTime < OptimizationCooldown)
    {
        return;
    }
    
    float TargetFrameTime = 1000.0f / PlatformProfile.TargetFPS;
    
    // If we're significantly over target frame time, reduce quality
    if (CurrentMetrics.FrameTimeMS > TargetFrameTime * 1.2f)
    {
        if (CurrentQualityLevel > EPerformanceQualityLevel::Low)
        {
            EPerformanceQualityLevel NewLevel = (EPerformanceQualityLevel)((int32)CurrentQualityLevel - 1);
            SetPerformanceQualityLevel(NewLevel);
            LastOptimizationTime = CurrentTime;
            
            UE_LOG(LogPerformanceOptimizer, Warning, TEXT("Auto-scaling performance down to level: %d"), (int32)NewLevel);
        }
    }
    // If we're well under target frame time, increase quality
    else if (CurrentMetrics.FrameTimeMS < TargetFrameTime * 0.8f)
    {
        if (CurrentQualityLevel < EPerformanceQualityLevel::Epic)
        {
            EPerformanceQualityLevel NewLevel = (EPerformanceQualityLevel)((int32)CurrentQualityLevel + 1);
            SetPerformanceQualityLevel(NewLevel);
            LastOptimizationTime = CurrentTime;
            
            UE_LOG(LogPerformanceOptimizer, Log, TEXT("Auto-scaling performance up to level: %d"), (int32)NewLevel);
        }
    }
}

void UPerformanceOptimizer::SetAutoPerformanceScaling(bool bEnabled)
{
    bAutoPerformanceScaling = bEnabled;
    CVarAutoPerformanceScaling->Set(bEnabled);
    
    UE_LOG(LogPerformanceOptimizer, Log, TEXT("Auto performance scaling: %s"), bEnabled ? TEXT("Enabled") : TEXT("Disabled"));
}

bool UPerformanceOptimizer::IsPerformanceTargetMet() const
{
    float TargetFrameTime = 1000.0f / PlatformProfile.TargetFPS;
    return CurrentMetrics.FrameTimeMS <= TargetFrameTime * 1.05f; // 5% tolerance
}

FString UPerformanceOptimizer::GetPerformanceBottleneck() const
{
    switch (CurrentMetrics.Bottleneck)
    {
        case EPerformanceBottleneck::CPU_GameThread:
            return TEXT("CPU Game Thread");
        case EPerformanceBottleneck::CPU_RenderThread:
            return TEXT("CPU Render Thread");
        case EPerformanceBottleneck::GPU:
            return TEXT("GPU");
        case EPerformanceBottleneck::Memory:
            return TEXT("Memory");
        case EPerformanceBottleneck::Physics:
            return TEXT("Physics");
        case EPerformanceBottleneck::Collision:
            return TEXT("Collision");
        case EPerformanceBottleneck::AI:
            return TEXT("AI");
        case EPerformanceBottleneck::Streaming:
            return TEXT("Streaming");
        default:
            return TEXT("None");
    }
}

void UPerformanceOptimizer::ForcePerformanceOptimization()
{
    UE_LOG(LogPerformanceOptimizer, Log, TEXT("Forcing performance optimization..."));
    
    ApplyPerformanceSettings();
    UpdatePerformanceMetrics();
    
    UE_LOG(LogPerformanceOptimizer, Log, TEXT("Performance optimization complete"));
}

void UPerformanceOptimizer::SetLODBias(float Bias)
{
    static IConsoleVariable* CVarStaticMeshLODBias = IConsoleManager::Get().FindConsoleVariable(TEXT("r.StaticMeshLODBias"));
    static IConsoleVariable* CVarSkeletalMeshLODBias = IConsoleManager::Get().FindConsoleVariable(TEXT("r.SkeletalMeshLODBias"));
    
    if (CVarStaticMeshLODBias)
    {
        CVarStaticMeshLODBias->Set(Bias);
    }
    
    if (CVarSkeletalMeshLODBias)
    {
        CVarSkeletalMeshLODBias->Set(Bias);
    }
    
    UE_LOG(LogPerformanceOptimizer, Log, TEXT("LOD bias set to: %f"), Bias);
}

void UPerformanceOptimizer::SetPhysicsQuality(int32 Quality)
{
    Quality = FMath::Clamp(Quality, 0, 3);
    
    static IConsoleVariable* CVarPhysicsSubSteps = IConsoleManager::Get().FindConsoleVariable(TEXT("p.Chaos.Solver.SubSteps"));
    static IConsoleVariable* CVarPhysicsIterations = IConsoleManager::Get().FindConsoleVariable(TEXT("p.Chaos.Solver.Iterations"));
    
    if (CVarPhysicsSubSteps)
    {
        CVarPhysicsSubSteps->Set(Quality + 1);
    }
    
    if (CVarPhysicsIterations)
    {
        CVarPhysicsIterations->Set((Quality + 1) * 2);
    }
    
    UE_LOG(LogPerformanceOptimizer, Log, TEXT("Physics quality set to: %d"), Quality);
}

void UPerformanceOptimizer::SetShadowQuality(int32 Quality)
{
    Quality = FMath::Clamp(Quality, 0, 3);
    
    static IConsoleVariable* CVarShadowQuality = IConsoleManager::Get().FindConsoleVariable(TEXT("sg.ShadowQuality"));
    
    if (CVarShadowQuality)
    {
        CVarShadowQuality->Set(Quality);
    }
    
    UE_LOG(LogPerformanceOptimizer, Log, TEXT("Shadow quality set to: %d"), Quality);
}

void UPerformanceOptimizer::SetTextureQuality(int32 Quality)
{
    Quality = FMath::Clamp(Quality, 0, 3);
    
    static IConsoleVariable* CVarTextureQuality = IConsoleManager::Get().FindConsoleVariable(TEXT("sg.TextureQuality"));
    
    if (CVarTextureQuality)
    {
        CVarTextureQuality->Set(Quality);
    }
    
    UE_LOG(LogPerformanceOptimizer, Log, TEXT("Texture quality set to: %d"), Quality);
}

void UPerformanceOptimizer::SetEffectsQuality(int32 Quality)
{
    Quality = FMath::Clamp(Quality, 0, 3);
    
    static IConsoleVariable* CVarEffectsQuality = IConsoleManager::Get().FindConsoleVariable(TEXT("sg.EffectsQuality"));
    
    if (CVarEffectsQuality)
    {
        CVarEffectsQuality->Set(Quality);
    }
    
    UE_LOG(LogPerformanceOptimizer, Log, TEXT("Effects quality set to: %d"), Quality);
}

void UPerformanceOptimizer::EnableNanite(bool bEnabled)
{
    static IConsoleVariable* CVarNanite = IConsoleManager::Get().FindConsoleVariable(TEXT("r.Nanite"));
    
    if (CVarNanite)
    {
        CVarNanite->Set(bEnabled ? 1 : 0);
    }
    
    UE_LOG(LogPerformanceOptimizer, Log, TEXT("Nanite: %s"), bEnabled ? TEXT("Enabled") : TEXT("Disabled"));
}

void UPerformanceOptimizer::EnableLumen(bool bEnabled)
{
    static IConsoleVariable* CVarLumen = IConsoleManager::Get().FindConsoleVariable(TEXT("r.DynamicGlobalIllumination"));
    
    if (CVarLumen)
    {
        CVarLumen->Set(bEnabled ? 1 : 0);
    }
    
    UE_LOG(LogPerformanceOptimizer, Log, TEXT("Lumen: %s"), bEnabled ? TEXT("Enabled") : TEXT("Disabled"));
}

void UPerformanceOptimizer::SetResolutionScale(float Scale)
{
    Scale = FMath::Clamp(Scale, 0.5f, 1.0f);
    
    static IConsoleVariable* CVarScreenPercentage = IConsoleManager::Get().FindConsoleVariable(TEXT("r.ScreenPercentage"));
    
    if (CVarScreenPercentage)
    {
        CVarScreenPercentage->Set(Scale * 100.0f);
    }
    
    UE_LOG(LogPerformanceOptimizer, Log, TEXT("Resolution scale set to: %f"), Scale);
}

void UPerformanceOptimizer::OptimizeForDinosaurSimulation()
{
    UE_LOG(LogPerformanceOptimizer, Log, TEXT("Optimizing for large-scale dinosaur simulation..."));
    
    // Optimize physics for many dynamic objects
    SetPhysicsQuality(2); // Medium physics quality for performance
    
    // Optimize LOD for distant dinosaurs
    SetLODBias(0.5f); // Slightly more aggressive LOD
    
    // Optimize shadows for many characters
    SetShadowQuality(2); // Medium shadow quality
    
    // Enable Nanite for detailed dinosaur meshes
    EnableNanite(true);
    
    // Set specific limits for dinosaur simulation
    CVarMaxPhysicsBodies->Set(PlatformProfile.MaxActivePhysicsBodies);
    CVarMaxAIAgents->Set(PlatformProfile.MaxAIAgents);
    
    UE_LOG(LogPerformanceOptimizer, Log, TEXT("Dinosaur simulation optimization complete"));
}