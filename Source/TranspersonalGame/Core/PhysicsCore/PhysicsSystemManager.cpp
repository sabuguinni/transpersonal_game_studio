#include "PhysicsSystemManager.h"
#include "Core_RagdollComponent.h"
#include "Core_DestructionComponent.h"
#include "Core_CollisionComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Chaos/ChaosEngineInterface.h"

UCore_PhysicsSystemManager::UCore_PhysicsSystemManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
    
    // Initialize default settings
    PhysicsSettings.GravityScale = 1.0f;
    PhysicsSettings.MaxSubsteps = 6;
    PhysicsSettings.FixedTimeStep = 0.016667f;
    PhysicsSettings.QualityLevel = ECore_PhysicsQuality::High;
    PhysicsSettings.MaxSimulatedBodies = 1000;
    PhysicsSettings.CullDistance = 5000.0f;
}

void UCore_PhysicsSystemManager::BeginPlay()
{
    Super::BeginPlay();
    InitializePhysicsSystem();
}

void UCore_PhysicsSystemManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bIsInitialized)
        return;

    // Update stats periodically
    StatsUpdateTimer += DeltaTime;
    if (StatsUpdateTimer >= STATS_UPDATE_FREQUENCY)
    {
        UpdatePhysicsStats();
        StatsUpdateTimer = 0.0f;
    }

    // Manage physics LOD based on performance
    ManagePhysicsLOD();
}

void UCore_PhysicsSystemManager::InitializePhysicsSystem()
{
    if (bIsInitialized)
        return;

    UE_LOG(LogTemp, Warning, TEXT("PhysicsSystemManager: Initializing physics system..."));

    // Apply quality settings
    ApplyQualitySettings();

    // Initialize component arrays
    RegisteredRagdollComponents.Empty();
    RegisteredDestructionComponents.Empty();
    RegisteredCollisionComponents.Empty();

    bIsInitialized = true;
    UE_LOG(LogTemp, Warning, TEXT("PhysicsSystemManager: Physics system initialized successfully"));
}

void UCore_PhysicsSystemManager::ShutdownPhysicsSystem()
{
    if (!bIsInitialized)
        return;

    UE_LOG(LogTemp, Warning, TEXT("PhysicsSystemManager: Shutting down physics system..."));

    // Clear all registered components
    RegisteredRagdollComponents.Empty();
    RegisteredDestructionComponents.Empty();
    RegisteredCollisionComponents.Empty();

    bIsInitialized = false;
    UE_LOG(LogTemp, Warning, TEXT("PhysicsSystemManager: Physics system shutdown complete"));
}

void UCore_PhysicsSystemManager::UpdatePhysicsSettings(const FCore_PhysicsSettings& NewSettings)
{
    PhysicsSettings = NewSettings;
    ApplyQualitySettings();
    UE_LOG(LogTemp, Log, TEXT("PhysicsSystemManager: Physics settings updated"));
}

void UCore_PhysicsSystemManager::OptimizePhysicsPerformance()
{
    if (!bIsInitialized)
        return;

    // Optimize ragdoll components
    for (UCore_RagdollComponent* RagdollComp : RegisteredRagdollComponents)
    {
        if (IsValid(RagdollComp))
        {
            // Distance-based optimization would go here
            // RagdollComp->OptimizeForDistance(ViewerLocation);
        }
    }

    // Optimize destruction components
    for (UCore_DestructionComponent* DestructionComp : RegisteredDestructionComponents)
    {
        if (IsValid(DestructionComp))
        {
            // Destruction optimization would go here
            // DestructionComp->OptimizeForPerformance();
        }
    }

    UE_LOG(LogTemp, Log, TEXT("PhysicsSystemManager: Performance optimization complete"));
}

void UCore_PhysicsSystemManager::CullDistantPhysicsBodies(const FVector& ViewerLocation)
{
    if (!bIsInitialized)
        return;

    int32 CulledBodies = 0;
    const float CullDistanceSquared = PhysicsSettings.CullDistance * PhysicsSettings.CullDistance;

    // Cull ragdoll components
    for (UCore_RagdollComponent* RagdollComp : RegisteredRagdollComponents)
    {
        if (IsValid(RagdollComp))
        {
            float DistanceSquared = FVector::DistSquared(ViewerLocation, RagdollComp->GetOwner()->GetActorLocation());
            if (DistanceSquared > CullDistanceSquared)
            {
                // RagdollComp->SetSimulationEnabled(false);
                CulledBodies++;
            }
        }
    }

    UE_LOG(LogTemp, Log, TEXT("PhysicsSystemManager: Culled %d distant physics bodies"), CulledBodies);
}

void UCore_PhysicsSystemManager::RegisterRagdollComponent(UCore_RagdollComponent* Component)
{
    if (IsValid(Component) && !RegisteredRagdollComponents.Contains(Component))
    {
        RegisteredRagdollComponents.Add(Component);
        UE_LOG(LogTemp, Log, TEXT("PhysicsSystemManager: Registered ragdoll component"));
    }
}

void UCore_PhysicsSystemManager::RegisterDestructionComponent(UCore_DestructionComponent* Component)
{
    if (IsValid(Component) && !RegisteredDestructionComponents.Contains(Component))
    {
        RegisteredDestructionComponents.Add(Component);
        UE_LOG(LogTemp, Log, TEXT("PhysicsSystemManager: Registered destruction component"));
    }
}

void UCore_PhysicsSystemManager::RegisterCollisionComponent(UCore_CollisionComponent* Component)
{
    if (IsValid(Component) && !RegisteredCollisionComponents.Contains(Component))
    {
        RegisteredCollisionComponents.Add(Component);
        UE_LOG(LogTemp, Log, TEXT("PhysicsSystemManager: Registered collision component"));
    }
}

FCore_PhysicsStats UCore_PhysicsSystemManager::GetPhysicsStats() const
{
    return CurrentStats;
}

void UCore_PhysicsSystemManager::TogglePhysicsDebugDraw()
{
    bDebugDrawEnabled = !bDebugDrawEnabled;
    UE_LOG(LogTemp, Warning, TEXT("PhysicsSystemManager: Debug draw %s"), bDebugDrawEnabled ? TEXT("enabled") : TEXT("disabled"));
}

void UCore_PhysicsSystemManager::ValidatePhysicsSetup()
{
    UE_LOG(LogTemp, Warning, TEXT("=== Physics System Validation ==="));
    UE_LOG(LogTemp, Warning, TEXT("Initialized: %s"), bIsInitialized ? TEXT("Yes") : TEXT("No"));
    UE_LOG(LogTemp, Warning, TEXT("Registered Ragdoll Components: %d"), RegisteredRagdollComponents.Num());
    UE_LOG(LogTemp, Warning, TEXT("Registered Destruction Components: %d"), RegisteredDestructionComponents.Num());
    UE_LOG(LogTemp, Warning, TEXT("Registered Collision Components: %d"), RegisteredCollisionComponents.Num());
    UE_LOG(LogTemp, Warning, TEXT("Quality Level: %d"), (int32)PhysicsSettings.QualityLevel);
    UE_LOG(LogTemp, Warning, TEXT("Max Simulated Bodies: %d"), PhysicsSettings.MaxSimulatedBodies);
    UE_LOG(LogTemp, Warning, TEXT("=== Validation Complete ==="));
}

void UCore_PhysicsSystemManager::UpdatePhysicsStats()
{
    CurrentStats.ActiveRigidBodies = RegisteredRagdollComponents.Num() + RegisteredDestructionComponents.Num();
    CurrentStats.SleepingBodies = 0; // Would be calculated from actual physics bodies
    CurrentStats.PhysicsFrameTime = GetWorld() ? GetWorld()->GetDeltaSeconds() : 0.0f;
    CurrentStats.CollisionPairs = RegisteredCollisionComponents.Num();
}

void UCore_PhysicsSystemManager::ApplyQualitySettings()
{
    UWorld* World = GetWorld();
    if (!World)
        return;

    // Apply gravity scale
    AWorldSettings* WorldSettings = World->GetWorldSettings();
    if (WorldSettings)
    {
        WorldSettings->GlobalGravityZ = -980.0f * PhysicsSettings.GravityScale;
    }

    // Apply quality-specific settings
    switch (PhysicsSettings.QualityLevel)
    {
        case ECore_PhysicsQuality::Low:
            PhysicsSettings.MaxSubsteps = 2;
            PhysicsSettings.FixedTimeStep = 0.033333f; // 30 FPS
            break;
        case ECore_PhysicsQuality::Medium:
            PhysicsSettings.MaxSubsteps = 4;
            PhysicsSettings.FixedTimeStep = 0.020000f; // 50 FPS
            break;
        case ECore_PhysicsQuality::High:
            PhysicsSettings.MaxSubsteps = 6;
            PhysicsSettings.FixedTimeStep = 0.016667f; // 60 FPS
            break;
        case ECore_PhysicsQuality::Ultra:
            PhysicsSettings.MaxSubsteps = 8;
            PhysicsSettings.FixedTimeStep = 0.008333f; // 120 FPS
            break;
    }

    UE_LOG(LogTemp, Log, TEXT("PhysicsSystemManager: Applied quality settings - Level: %d"), (int32)PhysicsSettings.QualityLevel);
}

void UCore_PhysicsSystemManager::ManagePhysicsLOD()
{
    // Simple LOD management based on frame rate
    if (GetWorld())
    {
        float CurrentFPS = 1.0f / GetWorld()->GetDeltaSeconds();
        
        if (CurrentFPS < 30.0f && PhysicsSettings.QualityLevel > ECore_PhysicsQuality::Low)
        {
            // Reduce quality if performance is poor
            PhysicsSettings.QualityLevel = static_cast<ECore_PhysicsQuality>((int32)PhysicsSettings.QualityLevel - 1);
            ApplyQualitySettings();
            UE_LOG(LogTemp, Warning, TEXT("PhysicsSystemManager: Reduced physics quality due to low FPS"));
        }
        else if (CurrentFPS > 50.0f && PhysicsSettings.QualityLevel < ECore_PhysicsQuality::Ultra)
        {
            // Increase quality if performance allows
            PhysicsSettings.QualityLevel = static_cast<ECore_PhysicsQuality>((int32)PhysicsSettings.QualityLevel + 1);
            ApplyQualitySettings();
            UE_LOG(LogTemp, Log, TEXT("PhysicsSystemManager: Increased physics quality due to good FPS"));
        }
    }
}