#include "Core_PhysicsSystemManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Chaos/ChaosEngineInterface.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/WorldSettings.h"
#include "Kismet/GameplayStatics.h"
#include "Core_CollisionManagerComponent.h"
#include "Core_RagdollManagerComponent.h"
#include "Core_DestructionManagerComponent.h"

ACore_PhysicsSystemManager::ACore_PhysicsSystemManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // 10 FPS for physics management

    // Create components
    CollisionManager = CreateDefaultSubobject<UCore_CollisionManagerComponent>(TEXT("CollisionManager"));
    RagdollManager = CreateDefaultSubobject<UCore_RagdollManagerComponent>(TEXT("RagdollManager"));
    DestructionManager = CreateDefaultSubobject<UCore_DestructionManagerComponent>(TEXT("DestructionManager"));

    // Initialize default settings
    PhysicsMode = ECore_PhysicsMode::Realistic;
    bEnablePhysicsDebugDraw = false;
    bEnablePerformanceMonitoring = true;
    MetricsUpdateInterval = 1.0f;

    // Set up default biome physics settings
    FCore_PhysicsSettings DefaultSettings;
    
    // Pantano - softer physics, more damping
    FCore_PhysicsSettings PantanoSettings = DefaultSettings;
    PantanoSettings.LinearDamping = 0.05f;
    PantanoSettings.AngularDamping = 0.05f;
    PantanoSettings.GravityScale = 0.9f;
    
    // Floresta - standard physics
    FCore_PhysicsSettings FlorestaSettings = DefaultSettings;
    
    // Savana - standard physics
    FCore_PhysicsSettings SavanaSettings = DefaultSettings;
    
    // Deserto - slightly reduced gravity for sand effect
    FCore_PhysicsSettings DesertoSettings = DefaultSettings;
    DesertoSettings.GravityScale = 0.95f;
    DesertoSettings.LinearDamping = 0.02f;
    
    // Montanha - increased gravity, reduced damping
    FCore_PhysicsSettings MontanhaSettings = DefaultSettings;
    MontanhaSettings.GravityScale = 1.1f;
    MontanhaSettings.LinearDamping = 0.005f;

    BiomePhysicsSettings.Add(EBiomeType::Pantano, PantanoSettings);
    BiomePhysicsSettings.Add(EBiomeType::Floresta, FlorestaSettings);
    BiomePhysicsSettings.Add(EBiomeType::Savana, SavanaSettings);
    BiomePhysicsSettings.Add(EBiomeType::Deserto, DesertoSettings);
    BiomePhysicsSettings.Add(EBiomeType::Montanha, MontanhaSettings);

    bIsInitialized = false;
}

void ACore_PhysicsSystemManager::BeginPlay()
{
    Super::BeginPlay();
    
    CachedWorld = GetWorld();
    if (CachedWorld)
    {
        InitializePhysicsSystem();
        UE_LOG(LogTemp, Log, TEXT("PhysicsSystemManager: Initialized successfully"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("PhysicsSystemManager: Failed to get world reference"));
    }
}

void ACore_PhysicsSystemManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bIsInitialized || !CachedWorld)
    {
        return;
    }

    // Update performance metrics
    if (bEnablePerformanceMonitoring)
    {
        MetricsTimer += DeltaTime;
        if (MetricsTimer >= MetricsUpdateInterval)
        {
            UpdatePerformanceMetrics();
            MetricsTimer = 0.0f;
        }
    }

    // Update physics settings based on current mode
    UpdatePhysicsSettings();
}

void ACore_PhysicsSystemManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    ShutdownPhysicsSystem();
    Super::EndPlay(EndPlayReason);
}

void ACore_PhysicsSystemManager::InitializePhysicsSystem()
{
    if (bIsInitialized)
    {
        UE_LOG(LogTemp, Warning, TEXT("PhysicsSystemManager: Already initialized"));
        return;
    }

    if (!CachedWorld)
    {
        UE_LOG(LogTemp, Error, TEXT("PhysicsSystemManager: No valid world reference"));
        return;
    }

    // Initialize components
    InitializeComponents();

    // Configure world physics
    ConfigureWorldPhysics();

    // Apply initial physics settings
    ApplyPhysicsSettings(PhysicsSettings);

    bIsInitialized = true;
    UE_LOG(LogTemp, Log, TEXT("PhysicsSystemManager: Physics system initialized"));
}

void ACore_PhysicsSystemManager::ShutdownPhysicsSystem()
{
    if (!bIsInitialized)
    {
        return;
    }

    // Clear registered actors
    RegisteredPhysicsActors.Empty();

    // Reset components
    if (CollisionManager)
    {
        CollisionManager->Shutdown();
    }
    if (RagdollManager)
    {
        RagdollManager->Shutdown();
    }
    if (DestructionManager)
    {
        DestructionManager->Shutdown();
    }

    bIsInitialized = false;
    UE_LOG(LogTemp, Log, TEXT("PhysicsSystemManager: Physics system shutdown"));
}

void ACore_PhysicsSystemManager::SetPhysicsMode(ECore_PhysicsMode NewMode)
{
    if (PhysicsMode == NewMode)
    {
        return;
    }

    PhysicsMode = NewMode;

    // Adjust physics settings based on mode
    switch (PhysicsMode)
    {
        case ECore_PhysicsMode::Realistic:
            PhysicsSettings.GravityScale = 1.0f;
            PhysicsSettings.LinearDamping = 0.01f;
            PhysicsSettings.AngularDamping = 0.01f;
            PhysicsSettings.bEnableCCD = true;
            break;

        case ECore_PhysicsMode::Arcade:
            PhysicsSettings.GravityScale = 0.7f;
            PhysicsSettings.LinearDamping = 0.05f;
            PhysicsSettings.AngularDamping = 0.05f;
            PhysicsSettings.bEnableCCD = false;
            break;

        case ECore_PhysicsMode::Cinematic:
            PhysicsSettings.GravityScale = 0.5f;
            PhysicsSettings.LinearDamping = 0.1f;
            PhysicsSettings.AngularDamping = 0.1f;
            PhysicsSettings.bEnableCCD = false;
            break;

        case ECore_PhysicsMode::Disabled:
            // Physics will be disabled in UpdatePhysicsSettings
            break;
    }

    ApplyPhysicsSettings(PhysicsSettings);
    UE_LOG(LogTemp, Log, TEXT("PhysicsSystemManager: Physics mode changed to %d"), (int32)PhysicsMode);
}

void ACore_PhysicsSystemManager::ApplyPhysicsSettings(const FCore_PhysicsSettings& NewSettings)
{
    PhysicsSettings = NewSettings;

    if (!CachedWorld)
    {
        return;
    }

    // Apply to world settings
    AWorldSettings* WorldSettings = CachedWorld->GetWorldSettings();
    if (WorldSettings)
    {
        WorldSettings->GlobalGravityZ = -980.0f * PhysicsSettings.GravityScale;
        
        // Update physics scene settings if available
        if (CachedWorld->GetPhysicsScene())
        {
            // Note: Some settings require engine-level changes
            // This is a simplified implementation
        }
    }

    UE_LOG(LogTemp, Log, TEXT("PhysicsSystemManager: Applied physics settings - Gravity: %.2f, LinearDamping: %.3f"), 
           PhysicsSettings.GravityScale, PhysicsSettings.LinearDamping);
}

void ACore_PhysicsSystemManager::SetBiomePhysicsSettings(EBiomeType BiomeType, const FCore_PhysicsSettings& Settings)
{
    BiomePhysicsSettings.Add(BiomeType, Settings);
    UE_LOG(LogTemp, Log, TEXT("PhysicsSystemManager: Updated physics settings for biome %d"), (int32)BiomeType);
}

void ACore_PhysicsSystemManager::UpdatePerformanceMetrics()
{
    if (!CachedWorld)
    {
        return;
    }

    CollectPerformanceData();

    // Log performance if it's concerning
    if (!IsPerformanceAcceptable())
    {
        UE_LOG(LogTemp, Warning, TEXT("PhysicsSystemManager: Performance below acceptable levels - Step time: %.3fms, Active bodies: %d"), 
               CurrentMetrics.PhysicsStepTime, CurrentMetrics.ActiveRigidBodies);
    }
}

FCore_PhysicsPerformanceMetrics ACore_PhysicsSystemManager::GetCurrentMetrics() const
{
    return CurrentMetrics;
}

bool ACore_PhysicsSystemManager::IsPerformanceAcceptable() const
{
    // Performance thresholds
    const float MaxStepTime = 5.0f; // 5ms
    const int32 MaxActiveBodies = 1000;

    return (CurrentMetrics.PhysicsStepTime <= MaxStepTime) && 
           (CurrentMetrics.ActiveRigidBodies <= MaxActiveBodies);
}

void ACore_PhysicsSystemManager::TogglePhysicsDebugDraw()
{
    bEnablePhysicsDebugDraw = !bEnablePhysicsDebugDraw;
    
    if (CachedWorld)
    {
        FString Command = bEnablePhysicsDebugDraw ? TEXT("showdebug collision") : TEXT("showdebug none");
        GEngine->Exec(CachedWorld, *Command);
    }

    UE_LOG(LogTemp, Log, TEXT("PhysicsSystemManager: Physics debug draw %s"), 
           bEnablePhysicsDebugDraw ? TEXT("enabled") : TEXT("disabled"));
}

void ACore_PhysicsSystemManager::LogPhysicsStatus()
{
    UE_LOG(LogTemp, Log, TEXT("=== PHYSICS SYSTEM STATUS ==="));
    UE_LOG(LogTemp, Log, TEXT("Initialized: %s"), bIsInitialized ? TEXT("Yes") : TEXT("No"));
    UE_LOG(LogTemp, Log, TEXT("Physics Mode: %d"), (int32)PhysicsMode);
    UE_LOG(LogTemp, Log, TEXT("Gravity Scale: %.2f"), PhysicsSettings.GravityScale);
    UE_LOG(LogTemp, Log, TEXT("Registered Actors: %d"), RegisteredPhysicsActors.Num());
    UE_LOG(LogTemp, Log, TEXT("Active Rigid Bodies: %d"), CurrentMetrics.ActiveRigidBodies);
    UE_LOG(LogTemp, Log, TEXT("Physics Step Time: %.3fms"), CurrentMetrics.PhysicsStepTime);
    UE_LOG(LogTemp, Log, TEXT("Performance Acceptable: %s"), IsPerformanceAcceptable() ? TEXT("Yes") : TEXT("No"));
}

void ACore_PhysicsSystemManager::ValidatePhysicsSetup()
{
    UE_LOG(LogTemp, Log, TEXT("=== VALIDATING PHYSICS SETUP ==="));

    bool bValid = true;

    // Check world reference
    if (!CachedWorld)
    {
        UE_LOG(LogTemp, Error, TEXT("No valid world reference"));
        bValid = false;
    }

    // Check components
    if (!CollisionManager)
    {
        UE_LOG(LogTemp, Error, TEXT("CollisionManager component missing"));
        bValid = false;
    }
    if (!RagdollManager)
    {
        UE_LOG(LogTemp, Error, TEXT("RagdollManager component missing"));
        bValid = false;
    }
    if (!DestructionManager)
    {
        UE_LOG(LogTemp, Error, TEXT("DestructionManager component missing"));
        bValid = false;
    }

    // Check biome settings
    for (int32 i = 0; i < (int32)EBiomeType::MAX; ++i)
    {
        EBiomeType BiomeType = (EBiomeType)i;
        if (!BiomePhysicsSettings.Contains(BiomeType))
        {
            UE_LOG(LogTemp, Warning, TEXT("Missing physics settings for biome %d"), i);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Physics setup validation: %s"), bValid ? TEXT("PASSED") : TEXT("FAILED"));
}

FCore_PhysicsSettings ACore_PhysicsSystemManager::GetBiomePhysicsSettings(EBiomeType BiomeType) const
{
    if (BiomePhysicsSettings.Contains(BiomeType))
    {
        return BiomePhysicsSettings[BiomeType];
    }
    return PhysicsSettings; // Return default if biome-specific not found
}

void ACore_PhysicsSystemManager::RegisterPhysicsActor(AActor* Actor)
{
    if (Actor && !RegisteredPhysicsActors.Contains(Actor))
    {
        RegisteredPhysicsActors.Add(Actor);
        UE_LOG(LogTemp, Log, TEXT("PhysicsSystemManager: Registered actor %s"), *Actor->GetName());
    }
}

void ACore_PhysicsSystemManager::UnregisterPhysicsActor(AActor* Actor)
{
    if (Actor)
    {
        RegisteredPhysicsActors.Remove(Actor);
        UE_LOG(LogTemp, Log, TEXT("PhysicsSystemManager: Unregistered actor %s"), *Actor->GetName());
    }
}

ACore_PhysicsSystemManager* ACore_PhysicsSystemManager::GetPhysicsSystemManager(UWorld* World)
{
    if (!World)
    {
        return nullptr;
    }

    // Find existing physics system manager in the world
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(World, ACore_PhysicsSystemManager::StaticClass(), FoundActors);

    if (FoundActors.Num() > 0)
    {
        return Cast<ACore_PhysicsSystemManager>(FoundActors[0]);
    }

    return nullptr;
}

void ACore_PhysicsSystemManager::InitializeComponents()
{
    if (CollisionManager)
    {
        CollisionManager->Initialize();
    }
    if (RagdollManager)
    {
        RagdollManager->Initialize();
    }
    if (DestructionManager)
    {
        DestructionManager->Initialize();
    }
}

void ACore_PhysicsSystemManager::ConfigureWorldPhysics()
{
    if (!CachedWorld)
    {
        return;
    }

    // Configure world-level physics settings
    AWorldSettings* WorldSettings = CachedWorld->GetWorldSettings();
    if (WorldSettings)
    {
        WorldSettings->bEnableWorldBoundsChecks = true;
        WorldSettings->KillZ = -10000.0f; // Set kill plane below terrain
    }
}

void ACore_PhysicsSystemManager::UpdatePhysicsSettings()
{
    if (PhysicsMode == ECore_PhysicsMode::Disabled)
    {
        // Disable physics simulation for performance
        if (CachedWorld)
        {
            // Note: This is a simplified implementation
            // In a full implementation, you'd pause the physics scene
        }
        return;
    }

    // Apply any dynamic physics adjustments here
    // This could include LOD-based physics quality adjustments
}

void ACore_PhysicsSystemManager::CollectPerformanceData()
{
    // Simplified performance data collection
    // In a full implementation, this would query the physics scene directly
    
    CurrentMetrics.ActiveRigidBodies = RegisteredPhysicsActors.Num();
    CurrentMetrics.SleepingRigidBodies = 0; // Would be calculated from physics scene
    CurrentMetrics.ActiveConstraints = 0;  // Would be calculated from physics scene
    CurrentMetrics.PhysicsStepTime = 0.0f; // Would be measured from physics tick
    CurrentMetrics.MemoryUsageMB = 0.0f;   // Would be calculated from physics memory usage

    // Estimate based on registered actors
    for (AActor* Actor : RegisteredPhysicsActors)
    {
        if (Actor)
        {
            TArray<UPrimitiveComponent*> PrimComponents;
            Actor->GetComponents<UPrimitiveComponent>(PrimComponents);
            
            for (UPrimitiveComponent* PrimComp : PrimComponents)
            {
                if (PrimComp && PrimComp->IsSimulatingPhysics())
                {
                    CurrentMetrics.ActiveRigidBodies++;
                }
            }
        }
    }
}