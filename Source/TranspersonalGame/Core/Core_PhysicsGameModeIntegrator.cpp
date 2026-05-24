#include "Core_PhysicsGameModeIntegrator.h"
#include "Core_PhysicsManager.h"
#include "Core_CollisionSystem.h"
#include "Core_RagdollSystem.h"
#include "Core_DestructionSystem.h"
#include "Core_PhysicsWorldManager.h"
#include "Eng_GameModeArchitect.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Engine/Engine.h"
#include "Chaos/ChaosEngineInterface.h"
#include "Components/PrimitiveComponent.h"
#include "Materials/MaterialInterface.h"
#include "PhysicalMaterials/PhysicalMaterial.h"

UCore_PhysicsGameModeIntegrator::UCore_PhysicsGameModeIntegrator()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second
    
    // Initialize integration state
    bIsIntegrationInitialized = false;
    bPhysicsSystemsRegistered = false;
    bValidationPassed = false;
    
    // Initialize performance tracking
    CurrentPhysicsFrameTime = 0.0f;
    ActivePhysicsActors = 0;
    ActiveRagdollCount = 0;
    PhysicsMemoryUsageMB = 0.0f;
    
    // Initialize timestamps
    LastValidationTime = 0.0;
    IntegrationStartTime = 0.0;
    
    // Initialize system references to null
    PhysicsManager = nullptr;
    CollisionSystem = nullptr;
    RagdollSystem = nullptr;
    DestructionSystem = nullptr;
    PhysicsWorldManager = nullptr;
    GameModeArchitectRef = nullptr;
}

void UCore_PhysicsGameModeIntegrator::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsGameModeIntegrator: BeginPlay - Starting physics integration"));
    
    IntegrationStartTime = FPlatformTime::Seconds();
    
    // Initialize physics system references
    InitializePhysicsSystemReferences();
    
    // Set up performance monitoring timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            PerformanceUpdateTimer,
            this,
            &UCore_PhysicsGameModeIntegrator::UpdatePerformanceMetrics,
            1.0f, // Update every second
            true  // Loop
        );
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsGameModeIntegrator: BeginPlay completed"));
}

void UCore_PhysicsGameModeIntegrator::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // Clear timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(PerformanceUpdateTimer);
    }
    
    // Clear system references
    PhysicsManager = nullptr;
    CollisionSystem = nullptr;
    RagdollSystem = nullptr;
    DestructionSystem = nullptr;
    PhysicsWorldManager = nullptr;
    GameModeArchitectRef = nullptr;
    
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsGameModeIntegrator: EndPlay - Physics integration shutdown"));
    
    Super::EndPlay(EndPlayReason);
}

void UCore_PhysicsGameModeIntegrator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update physics integration status
    if (bIsIntegrationInitialized && bPhysicsSystemsRegistered)
    {
        // Periodic validation check (every 10 seconds)
        double CurrentTime = FPlatformTime::Seconds();
        if (CurrentTime - LastValidationTime > 10.0)
        {
            ValidatePhysicsIntegration();
            LastValidationTime = CurrentTime;
        }
    }
}

void UCore_PhysicsGameModeIntegrator::InitializePhysicsIntegration(AEng_GameModeArchitect* GameModeArchitect)
{
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsGameModeIntegrator: InitializePhysicsIntegration started"));
    
    if (!GameModeArchitect)
    {
        UE_LOG(LogTemp, Error, TEXT("Core_PhysicsGameModeIntegrator: GameModeArchitect is null!"));
        return;
    }
    
    GameModeArchitectRef = GameModeArchitect;
    
    // Initialize physics system references
    InitializePhysicsSystemReferences();
    
    // Register physics subsystems with GameMode
    if (RegisterPhysicsSubsystems())
    {
        UE_LOG(LogTemp, Log, TEXT("Core_PhysicsGameModeIntegrator: Physics subsystems registered successfully"));
        
        // Configure Cretaceous physics
        ConfigureCretaceousPhysics();
        
        // Initialize dinosaur physics
        InitializeDinosaurPhysics();
        
        // Configure environmental destruction
        ConfigureEnvironmentalDestruction();
        
        // Validate integration
        if (ValidatePhysicsIntegration())
        {
            bIsIntegrationInitialized = true;
            UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsGameModeIntegrator: Physics integration completed successfully"));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Core_PhysicsGameModeIntegrator: Physics integration validation failed"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Core_PhysicsGameModeIntegrator: Failed to register physics subsystems"));
    }
}

bool UCore_PhysicsGameModeIntegrator::RegisterPhysicsSubsystems()
{
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsGameModeIntegrator: RegisterPhysicsSubsystems started"));
    
    if (!GameModeArchitectRef)
    {
        UE_LOG(LogTemp, Error, TEXT("Core_PhysicsGameModeIntegrator: GameModeArchitectRef is null"));
        return false;
    }
    
    int32 RegisteredCount = 0;
    
    // Register Physics Manager
    if (PhysicsManager)
    {
        UE_LOG(LogTemp, Log, TEXT("Core_PhysicsGameModeIntegrator: Registering Physics Manager"));
        RegisteredCount++;
    }
    
    // Register Collision System
    if (CollisionSystem)
    {
        UE_LOG(LogTemp, Log, TEXT("Core_PhysicsGameModeIntegrator: Registering Collision System"));
        RegisteredCount++;
    }
    
    // Register Ragdoll System
    if (RagdollSystem)
    {
        UE_LOG(LogTemp, Log, TEXT("Core_PhysicsGameModeIntegrator: Registering Ragdoll System"));
        RegisteredCount++;
    }
    
    // Register Destruction System
    if (DestructionSystem)
    {
        UE_LOG(LogTemp, Log, TEXT("Core_PhysicsGameModeIntegrator: Registering Destruction System"));
        RegisteredCount++;
    }
    
    // Register Physics World Manager
    if (PhysicsWorldManager)
    {
        UE_LOG(LogTemp, Log, TEXT("Core_PhysicsGameModeIntegrator: Registering Physics World Manager"));
        RegisteredCount++;
    }
    
    bPhysicsSystemsRegistered = (RegisteredCount > 0);
    
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsGameModeIntegrator: Registered %d physics subsystems"), RegisteredCount);
    
    return bPhysicsSystemsRegistered;
}

bool UCore_PhysicsGameModeIntegrator::ValidatePhysicsIntegration()
{
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsGameModeIntegrator: ValidatePhysicsIntegration started"));
    
    bool bValidationSuccess = true;
    
    // Validate GameMode reference
    if (!GameModeArchitectRef)
    {
        UE_LOG(LogTemp, Error, TEXT("Core_PhysicsGameModeIntegrator: GameMode reference validation failed"));
        bValidationSuccess = false;
    }
    
    // Validate physics system dependencies
    if (!ValidatePhysicsSystemDependencies())
    {
        UE_LOG(LogTemp, Error, TEXT("Core_PhysicsGameModeIntegrator: Physics system dependencies validation failed"));
        bValidationSuccess = false;
    }
    
    // Validate world physics settings
    if (UWorld* World = GetWorld())
    {
        if (World->GetPhysicsScene())
        {
            UE_LOG(LogTemp, Log, TEXT("Core_PhysicsGameModeIntegrator: Physics scene validation passed"));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Core_PhysicsGameModeIntegrator: Physics scene validation failed"));
            bValidationSuccess = false;
        }
    }
    
    bValidationPassed = bValidationSuccess;
    
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsGameModeIntegrator: Validation result: %s"), 
           bValidationSuccess ? TEXT("PASSED") : TEXT("FAILED"));
    
    return bValidationSuccess;
}

void UCore_PhysicsGameModeIntegrator::ConfigureCretaceousPhysics()
{
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsGameModeIntegrator: ConfigureCretaceousPhysics started"));
    
    // Apply realistic physics settings for Cretaceous period
    ApplyRealisticPhysicsSettings();
    
    // Configure collision channels
    ConfigureCollisionChannels();
    
    // Initialize physics materials
    InitializePhysicsMaterials();
    
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsGameModeIntegrator: Cretaceous physics configuration completed"));
}

void UCore_PhysicsGameModeIntegrator::InitializeDinosaurPhysics()
{
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsGameModeIntegrator: InitializeDinosaurPhysics started"));
    
    if (!bEnableRealisticDinosaurPhysics)
    {
        UE_LOG(LogTemp, Log, TEXT("Core_PhysicsGameModeIntegrator: Realistic dinosaur physics disabled"));
        return;
    }
    
    // Configure dinosaur-specific physics settings
    if (RagdollSystem)
    {
        UE_LOG(LogTemp, Log, TEXT("Core_PhysicsGameModeIntegrator: Configuring dinosaur ragdoll physics"));
        // Dinosaur ragdoll configuration would go here
    }
    
    if (CollisionSystem)
    {
        UE_LOG(LogTemp, Log, TEXT("Core_PhysicsGameModeIntegrator: Configuring dinosaur collision system"));
        // Dinosaur collision configuration would go here
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsGameModeIntegrator: Dinosaur physics initialization completed"));
}

void UCore_PhysicsGameModeIntegrator::ConfigureEnvironmentalDestruction()
{
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsGameModeIntegrator: ConfigureEnvironmentalDestruction started"));
    
    if (!bEnableEnvironmentalDestruction)
    {
        UE_LOG(LogTemp, Log, TEXT("Core_PhysicsGameModeIntegrator: Environmental destruction disabled"));
        return;
    }
    
    if (DestructionSystem)
    {
        UE_LOG(LogTemp, Log, TEXT("Core_PhysicsGameModeIntegrator: Configuring environmental destruction system"));
        // Environmental destruction configuration would go here
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsGameModeIntegrator: Environmental destruction configuration completed"));
}

void UCore_PhysicsGameModeIntegrator::CollectPhysicsMetrics()
{
    UpdatePerformanceMetrics();
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("Core_PhysicsGameModeIntegrator: Physics metrics collected - Frame Time: %.2fms, Active Actors: %d, Ragdolls: %d, Memory: %.1fMB"),
           CurrentPhysicsFrameTime, ActivePhysicsActors, ActiveRagdollCount, PhysicsMemoryUsageMB);
}

FString UCore_PhysicsGameModeIntegrator::GetPhysicsPerformanceStatus() const
{
    return FString::Printf(TEXT("Physics Performance: %.2fms frame time, %d active actors, %d ragdolls, %.1fMB memory"),
                          CurrentPhysicsFrameTime, ActivePhysicsActors, ActiveRagdollCount, PhysicsMemoryUsageMB);
}

void UCore_PhysicsGameModeIntegrator::HandleWorldStreamingEvent(bool bIsLoading, const FString& StreamingLevel)
{
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsGameModeIntegrator: HandleWorldStreamingEvent - %s level: %s"),
           bIsLoading ? TEXT("Loading") : TEXT("Unloading"), *StreamingLevel);
    
    if (PhysicsWorldManager)
    {
        // Handle physics world streaming
        UE_LOG(LogTemp, VeryVerbose, TEXT("Core_PhysicsGameModeIntegrator: Delegating to PhysicsWorldManager"));
    }
}

void UCore_PhysicsGameModeIntegrator::UpdatePhysicsForWorld()
{
    if (bIsIntegrationInitialized && PhysicsWorldManager)
    {
        // Update physics systems for current world state
        CollectPhysicsMetrics();
    }
}

void UCore_PhysicsGameModeIntegrator::InitializePhysicsSystemReferences()
{
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsGameModeIntegrator: InitializePhysicsSystemReferences started"));
    
    // Find physics systems in the world
    if (UWorld* World = GetWorld())
    {
        // These would typically be found as subsystems or components
        // For now, we'll create placeholder references
        UE_LOG(LogTemp, Log, TEXT("Core_PhysicsGameModeIntegrator: Physics system references initialized"));
    }
}

bool UCore_PhysicsGameModeIntegrator::ValidatePhysicsSystemDependencies()
{
    // Check if critical physics systems are available
    bool bDependenciesValid = true;
    
    if (UWorld* World = GetWorld())
    {
        if (!World->GetPhysicsScene())
        {
            UE_LOG(LogTemp, Error, TEXT("Core_PhysicsGameModeIntegrator: Physics scene not available"));
            bDependenciesValid = false;
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Core_PhysicsGameModeIntegrator: World not available"));
        bDependenciesValid = false;
    }
    
    return bDependenciesValid;
}

void UCore_PhysicsGameModeIntegrator::ApplyRealisticPhysicsSettings()
{
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsGameModeIntegrator: ApplyRealisticPhysicsSettings - Gravity: %.1f, Air Density: %.2f"),
           CretaceousGravity, CretaceousAirDensity);
    
    // Apply Cretaceous period physics settings
    if (UWorld* World = GetWorld())
    {
        // Set gravity for Cretaceous period (slightly different from modern Earth)
        World->GetPhysicsScene()->SetGravityZ(CretaceousGravity);
        UE_LOG(LogTemp, Log, TEXT("Core_PhysicsGameModeIntegrator: Applied Cretaceous gravity: %.1f"), CretaceousGravity);
    }
}

void UCore_PhysicsGameModeIntegrator::ConfigureCollisionChannels()
{
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsGameModeIntegrator: ConfigureCollisionChannels - Setting up dinosaur-specific collision"));
    
    // Configure collision channels for dinosaur game
    // This would set up channels for:
    // - Player character
    // - Dinosaurs (different sizes)
    // - Environment (trees, rocks, terrain)
    // - Destructible objects
    // - Projectiles (spears, rocks)
}

void UCore_PhysicsGameModeIntegrator::InitializePhysicsMaterials()
{
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsGameModeIntegrator: InitializePhysicsMaterials - Setting up Cretaceous materials"));
    
    // Initialize physics materials for different surfaces:
    // - Dinosaur skin/scales (various friction/bounce values)
    // - Prehistoric vegetation (flexible, breakable)
    // - Ancient rock formations (hard, brittle)
    // - Mud/swamp terrain (high friction, low bounce)
    // - Water surfaces (low friction)
}

void UCore_PhysicsGameModeIntegrator::UpdatePerformanceMetrics()
{
    if (UWorld* World = GetWorld())
    {
        // Update physics performance metrics
        CurrentPhysicsFrameTime = FPlatformTime::ToMilliseconds(World->GetDeltaSeconds() * 1000.0);
        
        // Count active physics actors
        ActivePhysicsActors = 0;
        ActiveRagdollCount = 0;
        
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            if (AActor* Actor = *ActorItr)
            {
                if (UPrimitiveComponent* PrimComp = Actor->FindComponentByClass<UPrimitiveComponent>())
                {
                    if (PrimComp->IsSimulatingPhysics())
                    {
                        ActivePhysicsActors++;
                    }
                }
            }
        }
        
        // Estimate physics memory usage (simplified)
        PhysicsMemoryUsageMB = (ActivePhysicsActors * 0.1f) + (ActiveRagdollCount * 0.5f);
    }
}

void UCore_PhysicsGameModeIntegrator::HandlePhysicsSystemError(const FString& SystemName, const FString& ErrorMessage)
{
    UE_LOG(LogTemp, Error, TEXT("Core_PhysicsGameModeIntegrator: Physics System Error - %s: %s"), *SystemName, *ErrorMessage);
    
    // Report error to GameMode if available
    if (GameModeArchitectRef)
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsGameModeIntegrator: Reporting error to GameMode Architect"));
    }
}