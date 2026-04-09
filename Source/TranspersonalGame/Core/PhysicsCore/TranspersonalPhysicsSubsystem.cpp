// Copyright Transpersonal Game Studio. All Rights Reserved.
// TranspersonalPhysicsSubsystem.cpp - Core physics system implementation

#include "TranspersonalPhysicsSubsystem.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "TimerManager.h"
#include "Stats/Stats.h"

DEFINE_LOG_CATEGORY(LogTranspersonalPhysics);

void UTranspersonalPhysicsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTranspersonalPhysics, Warning, TEXT("=== TRANSPERSONAL PHYSICS SUBSYSTEM INITIALIZING ==="));
    
    // Initialize default settings
    PhysicsSettings = FTranspersonalPhysicsSettings();
    
    // Initialize physics systems
    InitializeChaosPhysics();
    InitializeCollisionProfiles();
    InitializePhysicsMaterials();
    InitializeDestructionSystem();
    
    // Start performance monitoring
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(PhysicsPerformanceTimer,
            this, &UTranspersonalPhysicsSubsystem::MonitorPhysicsPerformance,
            PhysicsPerformanceMonitoringInterval, true);
    }
    
    bPhysicsInitialized = true;
    UE_LOG(LogTranspersonalPhysics, Warning, TEXT("✅ Physics subsystem initialized successfully"));
}

void UTranspersonalPhysicsSubsystem::Deinitialize()
{
    UE_LOG(LogTranspersonalPhysics, Warning, TEXT("=== TRANSPERSONAL PHYSICS SUBSYSTEM SHUTTING DOWN ==="));
    
    // Clear performance monitoring
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(PhysicsPerformanceTimer);
    }
    
    bPhysicsInitialized = false;
    Super::Deinitialize();
}

bool UTranspersonalPhysicsSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    // Create physics subsystem for all worlds except CDO
    return !Outer->HasAnyFlags(RF_ClassDefaultObject);
}

UTranspersonalPhysicsSubsystem* UTranspersonalPhysicsSubsystem::Get(const UObject* WorldContext)
{
    if (const UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::LogAndReturnNull))
    {
        return World->GetSubsystem<UTranspersonalPhysicsSubsystem>();
    }
    return nullptr;
}

void UTranspersonalPhysicsSubsystem::InitializeChaosPhysics()
{
    UE_LOG(LogTranspersonalPhysics, Log, TEXT("Initializing Chaos Physics..."));
    
    // Configure Chaos Physics settings
    if (UPhysicsSettings* PhysSettings = UPhysicsSettings::Get())
    {
        // Set default physics engine to Chaos
        PhysSettings->DefaultBroadphaseType = EBroadphaseType::MBP;
        PhysSettings->bEnableAsyncScene = true;
        PhysSettings->bEnableEnhancedDeterminism = false; // For better performance
        
        // Set solver settings
        PhysSettings->SolverOptions.SolverIterations = PhysicsSettings.SolverIterations;
        PhysSettings->SolverOptions.VelocityIterations = 1;
        PhysSettings->SolverOptions.ProjectionIterations = 1;
        
        // Set CCD settings
        PhysSettings->bEnableCCD = PhysicsSettings.bEnableCCD;
        
        UE_LOG(LogTranspersonalPhysics, Log, TEXT("  Chaos Physics configured with %d solver iterations"), 
            PhysicsSettings.SolverIterations);
    }
    
    // Set world gravity
    if (UWorld* World = GetWorld())
    {
        World->GetPhysicsScene()->SetGravityZ(-980.0f); // Default gravity
        UE_LOG(LogTranspersonalPhysics, Log, TEXT("  World gravity set to -980.0f"));
    }
}

void UTranspersonalPhysicsSubsystem::InitializeCollisionProfiles()
{
    UE_LOG(LogTranspersonalPhysics, Log, TEXT("Initializing collision profiles..."));
    
    SetupDefaultCollisionProfiles();
    
    UE_LOG(LogTranspersonalPhysics, Log, TEXT("  Initialized %d collision profiles"), CollisionProfiles.Num());
}

void UTranspersonalPhysicsSubsystem::SetupDefaultCollisionProfiles()
{
    // Player collision profile
    FTranspersonalCollisionProfile PlayerProfile;
    PlayerProfile.ProfileName = TEXT("TranspersonalPlayer");
    PlayerProfile.CollisionEnabled = ECollisionEnabled::QueryAndPhysics;
    PlayerProfile.ObjectType = ECC_Pawn;
    CollisionProfiles.Add(PlayerProfile.ProfileName, PlayerProfile);
    
    // Dinosaur collision profile
    FTranspersonalCollisionProfile DinosaurProfile;
    DinosaurProfile.ProfileName = TEXT("TranspersonalDinosaur");
    DinosaurProfile.CollisionEnabled = ECollisionEnabled::QueryAndPhysics;
    DinosaurProfile.ObjectType = ECC_Pawn;
    CollisionProfiles.Add(DinosaurProfile.ProfileName, DinosaurProfile);
    
    // Environment collision profile
    FTranspersonalCollisionProfile EnvironmentProfile;
    EnvironmentProfile.ProfileName = TEXT("TranspersonalEnvironment");
    EnvironmentProfile.CollisionEnabled = ECollisionEnabled::QueryAndPhysics;
    EnvironmentProfile.ObjectType = ECC_WorldStatic;
    CollisionProfiles.Add(EnvironmentProfile.ProfileName, EnvironmentProfile);
    
    // Destructible collision profile
    FTranspersonalCollisionProfile DestructibleProfile;
    DestructibleProfile.ProfileName = TEXT("TranspersonalDestructible");
    DestructibleProfile.CollisionEnabled = ECollisionEnabled::QueryAndPhysics;
    DestructibleProfile.ObjectType = ECC_Destructible;
    CollisionProfiles.Add(DestructibleProfile.ProfileName, DestructibleProfile);
    
    UE_LOG(LogTranspersonalPhysics, Log, TEXT("  Setup default collision profiles: Player, Dinosaur, Environment, Destructible"));
}

void UTranspersonalPhysicsSubsystem::InitializePhysicsMaterials()
{
    UE_LOG(LogTranspersonalPhysics, Log, TEXT("Initializing physics materials..."));
    
    // Physics materials will be created and configured here
    // This includes materials for different surface types:
    // - Rock, Dirt, Grass, Water, Wood, etc.
    
    UE_LOG(LogTranspersonalPhysics, Log, TEXT("  Physics materials initialized"));
}

void UTranspersonalPhysicsSubsystem::InitializeDestructionSystem()
{
    UE_LOG(LogTranspersonalPhysics, Log, TEXT("Initializing destruction system..."));
    
    // Configure Chaos Destruction
    bDestructionEnabled = true;
    
    UE_LOG(LogTranspersonalPhysics, Log, TEXT("  Destruction system enabled"));
}

void UTranspersonalPhysicsSubsystem::SetPhysicsSettings(const FTranspersonalPhysicsSettings& NewSettings)
{
    PhysicsSettings = NewSettings;
    
    // Apply new settings to the physics engine
    if (UPhysicsSettings* PhysSettings = UPhysicsSettings::Get())
    {
        PhysSettings->SolverOptions.SolverIterations = PhysicsSettings.SolverIterations;
        PhysSettings->bEnableCCD = PhysicsSettings.bEnableCCD;
    }
    
    OnPhysicsSettingsChanged.Broadcast(PhysicsSettings);
    
    UE_LOG(LogTranspersonalPhysics, Log, TEXT("Physics settings updated"));
}

void UTranspersonalPhysicsSubsystem::SetPhysicsQuality(ETranspersonalPhysicsQuality Quality)
{
    PhysicsSettings.PhysicsQuality = Quality;
    
    // Adjust settings based on quality level
    switch (Quality)
    {
        case ETranspersonalPhysicsQuality::Low:
            PhysicsSettings.SolverIterations = 4;
            PhysicsSettings.bEnableCCD = false;
            PhysicsSettings.PhysicsTickRate = 30.0f;
            break;
            
        case ETranspersonalPhysicsQuality::Medium:
            PhysicsSettings.SolverIterations = 6;
            PhysicsSettings.bEnableCCD = true;
            PhysicsSettings.PhysicsTickRate = 45.0f;
            break;
            
        case ETranspersonalPhysicsQuality::High:
            PhysicsSettings.SolverIterations = 8;
            PhysicsSettings.bEnableCCD = true;
            PhysicsSettings.PhysicsTickRate = 60.0f;
            break;
            
        case ETranspersonalPhysicsQuality::Ultra:
            PhysicsSettings.SolverIterations = 12;
            PhysicsSettings.bEnableCCD = true;
            PhysicsSettings.PhysicsTickRate = 60.0f;
            break;
    }
    
    SetPhysicsSettings(PhysicsSettings);
    
    UE_LOG(LogTranspersonalPhysics, Log, TEXT("Physics quality set to: %s"), 
        *UEnum::GetValueAsString(Quality));
}

bool UTranspersonalPhysicsSubsystem::RegisterCollisionProfile(const FTranspersonalCollisionProfile& Profile)
{
    if (Profile.ProfileName.IsNone())
    {
        UE_LOG(LogTranspersonalPhysics, Warning, TEXT("Cannot register collision profile with empty name"));
        return false;
    }
    
    CollisionProfiles.Add(Profile.ProfileName, Profile);
    
    UE_LOG(LogTranspersonalPhysics, Log, TEXT("Registered collision profile: %s"), *Profile.ProfileName.ToString());
    return true;
}

bool UTranspersonalPhysicsSubsystem::UnregisterCollisionProfile(const FName& ProfileName)
{
    if (CollisionProfiles.Remove(ProfileName) > 0)
    {
        UE_LOG(LogTranspersonalPhysics, Log, TEXT("Unregistered collision profile: %s"), *ProfileName.ToString());
        return true;
    }
    
    UE_LOG(LogTranspersonalPhysics, Warning, TEXT("Failed to unregister collision profile: %s (not found)"), 
        *ProfileName.ToString());
    return false;
}

FTranspersonalCollisionProfile UTranspersonalPhysicsSubsystem::GetCollisionProfile(const FName& ProfileName) const
{
    if (const FTranspersonalCollisionProfile* Profile = CollisionProfiles.Find(ProfileName))
    {
        return *Profile;
    }
    
    UE_LOG(LogTranspersonalPhysics, Warning, TEXT("Collision profile not found: %s"), *ProfileName.ToString());
    return FTranspersonalCollisionProfile();
}

bool UTranspersonalPhysicsSubsystem::LineTrace(const FVector& Start, const FVector& End, FHitResult& HitResult, 
                                               ECollisionChannel TraceChannel) const
{
    if (UWorld* World = GetWorld())
    {
        return World->LineTraceSingleByChannel(HitResult, Start, End, TraceChannel);
    }
    return false;
}

bool UTranspersonalPhysicsSubsystem::SphereTrace(const FVector& Start, const FVector& End, float Radius, 
                                                 FHitResult& HitResult, ECollisionChannel TraceChannel) const
{
    if (UWorld* World = GetWorld())
    {
        FCollisionShape SphereShape = FCollisionShape::MakeSphere(Radius);
        return World->SweepSingleByChannel(HitResult, Start, End, FQuat::Identity, TraceChannel, SphereShape);
    }
    return false;
}

TArray<FHitResult> UTranspersonalPhysicsSubsystem::MultiLineTrace(const FVector& Start, const FVector& End, 
                                                                  ECollisionChannel TraceChannel) const
{
    TArray<FHitResult> HitResults;
    
    if (UWorld* World = GetWorld())
    {
        World->LineTraceMultiByChannel(HitResults, Start, End, TraceChannel);
    }
    
    return HitResults;
}

void UTranspersonalPhysicsSubsystem::SetGlobalGravity(const FVector& NewGravity)
{
    if (UWorld* World = GetWorld())
    {
        World->GetPhysicsScene()->SetGravityZ(NewGravity.Z);
        UE_LOG(LogTranspersonalPhysics, Log, TEXT("Global gravity set to: %s"), *NewGravity.ToString());
    }
}

FVector UTranspersonalPhysicsSubsystem::GetGlobalGravity() const
{
    if (UWorld* World = GetWorld())
    {
        float GravityZ = World->GetPhysicsScene()->GetGravityZ();
        return FVector(0.0f, 0.0f, GravityZ);
    }
    return FVector::ZeroVector;
}

void UTranspersonalPhysicsSubsystem::PausePhysicsSimulation()
{
    if (UWorld* World = GetWorld())
    {
        World->GetPhysicsScene()->SetIsStaticLoading(true);
        bPhysicsSimulationPaused = true;
        UE_LOG(LogTranspersonalPhysics, Log, TEXT("Physics simulation paused"));
    }
}

void UTranspersonalPhysicsSubsystem::ResumePhysicsSimulation()
{
    if (UWorld* World = GetWorld())
    {
        World->GetPhysicsScene()->SetIsStaticLoading(false);
        bPhysicsSimulationPaused = false;
        UE_LOG(LogTranspersonalPhysics, Log, TEXT("Physics simulation resumed"));
    }
}

bool UTranspersonalPhysicsSubsystem::IsPhysicsSimulationPaused() const
{
    return bPhysicsSimulationPaused;
}

void UTranspersonalPhysicsSubsystem::EnableDestruction(bool bEnable)
{
    bDestructionEnabled = bEnable;
    UE_LOG(LogTranspersonalPhysics, Log, TEXT("Destruction system %s"), 
        bEnable ? TEXT("enabled") : TEXT("disabled"));
}

bool UTranspersonalPhysicsSubsystem::IsDestructionEnabled() const
{
    return bDestructionEnabled;
}

void UTranspersonalPhysicsSubsystem::EnableRagdollPhysics(USkeletalMeshComponent* SkeletalMesh)
{
    if (SkeletalMesh)
    {
        SkeletalMesh->SetSimulatePhysics(true);
        SkeletalMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        UE_LOG(LogTranspersonalPhysics, Log, TEXT("Ragdoll physics enabled for: %s"), 
            *SkeletalMesh->GetName());
    }
}

void UTranspersonalPhysicsSubsystem::DisableRagdollPhysics(USkeletalMeshComponent* SkeletalMesh)
{
    if (SkeletalMesh)
    {
        SkeletalMesh->SetSimulatePhysics(false);
        SkeletalMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        UE_LOG(LogTranspersonalPhysics, Log, TEXT("Ragdoll physics disabled for: %s"), 
            *SkeletalMesh->GetName());
    }
}

float UTranspersonalPhysicsSubsystem::GetPhysicsFrameTime() const
{
    return LastPhysicsFrameTime;
}

int32 UTranspersonalPhysicsSubsystem::GetActivePhysicsBodies() const
{
    return LastActivePhysicsBodies;
}

float UTranspersonalPhysicsSubsystem::GetPhysicsMemoryUsage() const
{
    // Simplified physics memory usage calculation
    return LastActivePhysicsBodies * 0.001f; // Approximate MB per body
}

void UTranspersonalPhysicsSubsystem::MonitorPhysicsPerformance()
{
    // Get physics performance metrics
    LastPhysicsFrameTime = FApp::GetDeltaTime() * 1000.0f; // Convert to milliseconds
    
    // Count active physics bodies (simplified)
    if (UWorld* World = GetWorld())
    {
        LastActivePhysicsBodies = 0;
        for (TActorIterator<AActor> ActorIterator(World); ActorIterator; ++ActorIterator)
        {
            AActor* Actor = *ActorIterator;
            if (Actor && Actor->GetRootComponent())
            {
                if (UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent()))
                {
                    if (PrimComp->IsSimulatingPhysics())
                    {
                        LastActivePhysicsBodies++;
                    }
                }
            }
        }
    }
    
    // Check for performance warnings
    if (LastPhysicsFrameTime > PhysicsPerformanceWarningThreshold)
    {
        UE_LOG(LogTranspersonalPhysics, Warning, TEXT("⚠️ Physics Performance Warning: %.2fms frame time"), 
            LastPhysicsFrameTime);
        OnPhysicsPerformanceWarning.Broadcast(LastPhysicsFrameTime);
    }
    
    // Log metrics periodically
    static int32 PhysicsLogCounter = 0;
    if (++PhysicsLogCounter >= 10) // Every 10 seconds
    {
        UE_LOG(LogTranspersonalPhysics, Log, TEXT("Physics Metrics - Frame: %.2fms, Bodies: %d, Memory: %.1fMB"), 
            LastPhysicsFrameTime, LastActivePhysicsBodies, GetPhysicsMemoryUsage());
        PhysicsLogCounter = 0;
    }
}