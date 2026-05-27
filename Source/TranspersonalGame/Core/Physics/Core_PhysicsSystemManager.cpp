#include "Core_PhysicsSystemManager.h"
#include "Core_CollisionSystem.h"
#include "Core_RagdollSystem.h"
#include "Core_DestructionSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "DrawDebugHelpers.h"

UCore_PhysicsSystemManager::UCore_PhysicsSystemManager()
{
    // Initialize default physics settings
    GlobalGravityScale = 1.0f;
    GlobalPhysicsTimeStep = 0.016667f; // 60 FPS
    MaxPhysicsSubSteps = 6;
    bEnablePhysicsSimulation = true;
    
    // Performance settings
    MaxActiveRagdolls = 10;
    MaxDestructionObjects = 50;
    PhysicsUpdateDistance = 5000.0f;
    
    // Internal state
    LastPhysicsUpdateTime = 0.0f;
    CurrentActiveRagdolls = 0;
    CurrentDestructionObjects = 0;
    bSystemsInitialized = false;
    
    // System references
    CollisionSystem = nullptr;
    RagdollSystem = nullptr;
    DestructionSystem = nullptr;
}

void UCore_PhysicsSystemManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("PhysicsSystemManager: Initializing physics subsystem"));
    
    // Create and initialize physics systems
    CreatePhysicsSystems();
    ConfigurePhysicsSettings();
    RegisterPhysicsEvents();
    
    bSystemsInitialized = true;
    
    UE_LOG(LogTemp, Warning, TEXT("PhysicsSystemManager: Physics subsystem initialized successfully"));
}

void UCore_PhysicsSystemManager::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("PhysicsSystemManager: Shutting down physics subsystem"));
    
    ShutdownPhysicsSystems();
    
    // Clear system references
    CollisionSystem = nullptr;
    RagdollSystem = nullptr;
    DestructionSystem = nullptr;
    
    bSystemsInitialized = false;
    
    Super::Deinitialize();
}

void UCore_PhysicsSystemManager::InitializePhysicsSystems()
{
    if (bSystemsInitialized)
    {
        UE_LOG(LogTemp, Warning, TEXT("PhysicsSystemManager: Systems already initialized"));
        return;
    }
    
    CreatePhysicsSystems();
    ConfigurePhysicsSettings();
    RegisterPhysicsEvents();
    
    bSystemsInitialized = true;
    
    UE_LOG(LogTemp, Warning, TEXT("PhysicsSystemManager: Physics systems initialized"));
}

void UCore_PhysicsSystemManager::ShutdownPhysicsSystems()
{
    if (!bSystemsInitialized)
    {
        return;
    }
    
    // Shutdown individual systems
    if (CollisionSystem)
    {
        CollisionSystem->ConditionalBeginDestroy();
    }
    
    if (RagdollSystem)
    {
        RagdollSystem->ConditionalBeginDestroy();
    }
    
    if (DestructionSystem)
    {
        DestructionSystem->ConditionalBeginDestroy();
    }
    
    bSystemsInitialized = false;
    
    UE_LOG(LogTemp, Warning, TEXT("PhysicsSystemManager: Physics systems shut down"));
}

void UCore_PhysicsSystemManager::UpdatePhysicsSystems(float DeltaTime)
{
    if (!bSystemsInitialized || !bEnablePhysicsSimulation)
    {
        return;
    }
    
    LastPhysicsUpdateTime = DeltaTime;
    
    // Update individual physics systems
    if (CollisionSystem)
    {
        CollisionSystem->UpdateCollisionSystem(DeltaTime);
    }
    
    if (RagdollSystem)
    {
        RagdollSystem->UpdateRagdollSystem(DeltaTime);
        CurrentActiveRagdolls = RagdollSystem->GetActiveRagdollCount();
    }
    
    if (DestructionSystem)
    {
        DestructionSystem->UpdateDestructionSystem(DeltaTime);
        CurrentDestructionObjects = DestructionSystem->GetActiveDestructionCount();
    }
    
    // Performance monitoring
    if (CurrentActiveRagdolls > MaxActiveRagdolls)
    {
        UE_LOG(LogTemp, Warning, TEXT("PhysicsSystemManager: Ragdoll limit exceeded (%d/%d)"), 
               CurrentActiveRagdolls, MaxActiveRagdolls);
    }
    
    if (CurrentDestructionObjects > MaxDestructionObjects)
    {
        UE_LOG(LogTemp, Warning, TEXT("PhysicsSystemManager: Destruction object limit exceeded (%d/%d)"), 
               CurrentDestructionObjects, MaxDestructionObjects);
    }
}

void UCore_PhysicsSystemManager::CreatePhysicsSystems()
{
    // Create collision system
    CollisionSystem = NewObject<UCore_CollisionSystem>(this);
    if (CollisionSystem)
    {
        UE_LOG(LogTemp, Warning, TEXT("PhysicsSystemManager: Collision system created"));
    }
    
    // Create ragdoll system
    RagdollSystem = NewObject<UCore_RagdollSystem>(this);
    if (RagdollSystem)
    {
        UE_LOG(LogTemp, Warning, TEXT("PhysicsSystemManager: Ragdoll system created"));
    }
    
    // Create destruction system
    DestructionSystem = NewObject<UCore_DestructionSystem>(this);
    if (DestructionSystem)
    {
        UE_LOG(LogTemp, Warning, TEXT("PhysicsSystemManager: Destruction system created"));
    }
}

void UCore_PhysicsSystemManager::ConfigurePhysicsSettings()
{
    UWorld* World = GetWorld();
    if (World)
    {
        ApplyWorldPhysicsSettings(World);
    }
    
    // Configure global physics settings
    if (UPhysicsSettings* PhysicsSettings = UPhysicsSettings::Get())
    {
        PhysicsSettings->DefaultGravityZ = -980.0f * GlobalGravityScale;
        PhysicsSettings->MaxSubstepDeltaTime = GlobalPhysicsTimeStep;
        PhysicsSettings->MaxSubsteps = MaxPhysicsSubSteps;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("PhysicsSystemManager: Physics settings configured"));
}

void UCore_PhysicsSystemManager::RegisterPhysicsEvents()
{
    // Bind event handlers
    OnLargeImpact.AddDynamic(this, &UCore_PhysicsSystemManager::HandleLargeImpact);
    OnObjectDestroyed.AddDynamic(this, &UCore_PhysicsSystemManager::HandleObjectDestruction);
    OnRagdollActivated.AddDynamic(this, &UCore_PhysicsSystemManager::HandleRagdollActivation);
    
    UE_LOG(LogTemp, Warning, TEXT("PhysicsSystemManager: Physics events registered"));
}

void UCore_PhysicsSystemManager::ApplyWorldPhysicsSettings(UWorld* World)
{
    if (!World)
    {
        return;
    }
    
    // Apply world-specific physics settings
    World->GetWorldSettings()->GlobalGravityZ = -980.0f * GlobalGravityScale;
    
    UE_LOG(LogTemp, Warning, TEXT("PhysicsSystemManager: World physics settings applied"));
}

void UCore_PhysicsSystemManager::RestoreWorldPhysicsSettings(UWorld* World)
{
    if (!World)
    {
        return;
    }
    
    // Restore default physics settings
    World->GetWorldSettings()->GlobalGravityZ = -980.0f;
    
    UE_LOG(LogTemp, Warning, TEXT("PhysicsSystemManager: World physics settings restored"));
}

int32 UCore_PhysicsSystemManager::GetActiveRagdollCount() const
{
    return CurrentActiveRagdolls;
}

int32 UCore_PhysicsSystemManager::GetActiveDestructionCount() const
{
    return CurrentDestructionObjects;
}

float UCore_PhysicsSystemManager::GetCurrentPhysicsLoad() const
{
    float RagdollLoad = static_cast<float>(CurrentActiveRagdolls) / static_cast<float>(MaxActiveRagdolls);
    float DestructionLoad = static_cast<float>(CurrentDestructionObjects) / static_cast<float>(MaxDestructionObjects);
    
    return FMath::Max(RagdollLoad, DestructionLoad);
}

void UCore_PhysicsSystemManager::DebugDrawPhysicsInfo()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    FVector PlayerLocation = FVector::ZeroVector;
    if (APawn* PlayerPawn = World->GetFirstPlayerController()->GetPawn())
    {
        PlayerLocation = PlayerPawn->GetActorLocation();
    }
    
    // Draw physics system status
    FString DebugText = FString::Printf(TEXT("Physics Systems Status:\nRagdolls: %d/%d\nDestruction: %d/%d\nLoad: %.2f%%"),
        CurrentActiveRagdolls, MaxActiveRagdolls,
        CurrentDestructionObjects, MaxDestructionObjects,
        GetCurrentPhysicsLoad() * 100.0f);
    
    DrawDebugString(World, PlayerLocation + FVector(0, 0, 200), DebugText, nullptr, FColor::Yellow, 0.0f);
    
    UE_LOG(LogTemp, Warning, TEXT("PhysicsSystemManager: Debug info drawn"));
}

void UCore_PhysicsSystemManager::LogPhysicsStats()
{
    UE_LOG(LogTemp, Warning, TEXT("=== Physics System Statistics ==="));
    UE_LOG(LogTemp, Warning, TEXT("Active Ragdolls: %d/%d"), CurrentActiveRagdolls, MaxActiveRagdolls);
    UE_LOG(LogTemp, Warning, TEXT("Active Destruction Objects: %d/%d"), CurrentDestructionObjects, MaxDestructionObjects);
    UE_LOG(LogTemp, Warning, TEXT("Physics Load: %.2f%%"), GetCurrentPhysicsLoad() * 100.0f);
    UE_LOG(LogTemp, Warning, TEXT("Last Update Time: %.4f ms"), LastPhysicsUpdateTime * 1000.0f);
    UE_LOG(LogTemp, Warning, TEXT("Systems Initialized: %s"), bSystemsInitialized ? TEXT("Yes") : TEXT("No"));
    UE_LOG(LogTemp, Warning, TEXT("Physics Simulation Enabled: %s"), bEnablePhysicsSimulation ? TEXT("Yes") : TEXT("No"));
    UE_LOG(LogTemp, Warning, TEXT("================================"));
}

void UCore_PhysicsSystemManager::HandleLargeImpact(AActor* Actor, const FVector& ImpactLocation)
{
    if (!Actor)
    {
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("PhysicsSystemManager: Large impact detected on %s at %s"), 
           *Actor->GetName(), *ImpactLocation.ToString());
    
    // Handle large impact effects
    // This could trigger screen shake, sound effects, particle effects, etc.
}

void UCore_PhysicsSystemManager::HandleObjectDestruction(AActor* Actor, const FVector& DestructionLocation)
{
    if (!Actor)
    {
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("PhysicsSystemManager: Object destroyed: %s at %s"), 
           *Actor->GetName(), *DestructionLocation.ToString());
    
    // Handle destruction effects
    // This could spawn debris, play destruction sounds, create particle effects, etc.
}

void UCore_PhysicsSystemManager::HandleRagdollActivation(AActor* Actor, const FVector& ActivationLocation)
{
    if (!Actor)
    {
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("PhysicsSystemManager: Ragdoll activated on %s at %s"), 
           *Actor->GetName(), *ActivationLocation.ToString());
    
    CurrentActiveRagdolls++;
    
    // Handle ragdoll activation effects
    // This could play death sounds, trigger AI reactions, etc.
}