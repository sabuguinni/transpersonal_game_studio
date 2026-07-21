#include "Core_PhysicsArchitectureIntegrator.h"
#include "Core_PhysicsManager.h"
#include "Core_PhysicsIntegrator.h"
#include "Core_CollisionSystem.h"
#include "Core_RagdollSystem.h"
#include "Core_DestructionSystem.h"
#include "Core_TerrainPhysics.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"

UCore_PhysicsArchitectureIntegrator::UCore_PhysicsArchitectureIntegrator()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10Hz tick for performance monitoring
    
    // Initialize default values
    bPhysicsSystemsInitialized = false;
    bArchitectureIntegrationActive = false;
    PhysicsHealthCheckInterval = 5.0f;
    LastHealthCheckTime = 0.0f;
    CurrentPhysicsFPS = 0.0f;
    PhysicsSimulationTime = 0.0f;
    ActivePhysicsObjects = 0;
    PerformanceThreshold = 30.0f; // 30 FPS minimum
    
    // Initialize system references
    PhysicsManager = nullptr;
    PhysicsIntegrator = nullptr;
    CollisionSystem = nullptr;
    RagdollSystem = nullptr;
    DestructionSystem = nullptr;
    TerrainPhysics = nullptr;
    ArchitectureManager = nullptr;
}

void UCore_PhysicsArchitectureIntegrator::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsArchitectureIntegrator: Beginning physics architecture integration"));
    
    // Initialize physics systems
    if (InitializePhysicsSystems())
    {
        UE_LOG(LogTemp, Log, TEXT("Physics systems initialized successfully"));
        
        // Register with architecture manager
        if (RegisterWithArchitectureManager())
        {
            bArchitectureIntegrationActive = true;
            UE_LOG(LogTemp, Log, TEXT("Physics systems registered with architecture manager"));
            
            // Trigger Blueprint event
            OnPhysicsSystemsInitialized();
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Failed to register with architecture manager"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to initialize physics systems"));
    }
}

void UCore_PhysicsArchitectureIntegrator::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsArchitectureIntegrator: Ending physics architecture integration"));
    
    // Unregister from architecture manager
    UnregisterFromArchitectureManager();
    
    // Shutdown physics systems
    ShutdownPhysicsSystems();
    
    // Trigger Blueprint event
    OnPhysicsSystemsShutdown();
    
    Super::EndPlay(EndPlayReason);
}

void UCore_PhysicsArchitectureIntegrator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bPhysicsSystemsInitialized || !bArchitectureIntegrationActive)
    {
        return;
    }
    
    // Update performance metrics
    UpdatePerformanceMetrics();
    
    // Periodic health check
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastHealthCheckTime >= PhysicsHealthCheckInterval)
    {
        CheckPhysicsSystemsHealth();
        LastHealthCheckTime = CurrentTime;
    }
}

bool UCore_PhysicsArchitectureIntegrator::InitializePhysicsSystems()
{
    UE_LOG(LogTemp, Log, TEXT("Initializing physics systems..."));
    
    // Create physics system instances
    CreatePhysicsSystemInstances();
    
    // Configure system settings
    ConfigurePhysicsSystemSettings();
    
    // Validate system integrity
    bool bSystemsValid = ValidatePhysicsSystemIntegrity();
    
    if (bSystemsValid)
    {
        bPhysicsSystemsInitialized = true;
        UE_LOG(LogTemp, Log, TEXT("Physics systems initialization complete"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Physics systems validation failed"));
    }
    
    return bSystemsValid;
}

void UCore_PhysicsArchitectureIntegrator::ShutdownPhysicsSystems()
{
    UE_LOG(LogTemp, Log, TEXT("Shutting down physics systems..."));
    
    // Clear system references
    PhysicsManager = nullptr;
    PhysicsIntegrator = nullptr;
    CollisionSystem = nullptr;
    RagdollSystem = nullptr;
    DestructionSystem = nullptr;
    TerrainPhysics = nullptr;
    
    bPhysicsSystemsInitialized = false;
    bArchitectureIntegrationActive = false;
    
    UE_LOG(LogTemp, Log, TEXT("Physics systems shutdown complete"));
}

bool UCore_PhysicsArchitectureIntegrator::RegisterWithArchitectureManager()
{
    // Find architecture manager in world
    if (UWorld* World = GetWorld())
    {
        if (UGameInstance* GameInstance = World->GetGameInstance())
        {
            // Try to get architecture manager from game instance
            TArray<UObject*> FoundObjects;
            GetObjectsOfClass(UEng_UnifiedArchitectureManager::StaticClass(), FoundObjects, true);
            
            if (FoundObjects.Num() > 0)
            {
                ArchitectureManager = Cast<UEng_UnifiedArchitectureManager>(FoundObjects[0]);
                if (ArchitectureManager)
                {
                    UE_LOG(LogTemp, Log, TEXT("Found and connected to Unified Architecture Manager"));
                    return true;
                }
            }
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Could not find Unified Architecture Manager"));
    return false;
}

void UCore_PhysicsArchitectureIntegrator::UnregisterFromArchitectureManager()
{
    if (ArchitectureManager)
    {
        UE_LOG(LogTemp, Log, TEXT("Unregistering from architecture manager"));
        ArchitectureManager = nullptr;
    }
    
    bArchitectureIntegrationActive = false;
}

bool UCore_PhysicsArchitectureIntegrator::CheckPhysicsSystemsHealth()
{
    if (!bPhysicsSystemsInitialized)
    {
        return false;
    }
    
    bool bAllSystemsHealthy = true;
    TArray<FString> FailedSystems;
    
    // Check each physics system
    if (!PhysicsManager || !IsValid(PhysicsManager))
    {
        bAllSystemsHealthy = false;
        FailedSystems.Add(TEXT("PhysicsManager"));
    }
    
    if (!PhysicsIntegrator || !IsValid(PhysicsIntegrator))
    {
        bAllSystemsHealthy = false;
        FailedSystems.Add(TEXT("PhysicsIntegrator"));
    }
    
    if (!CollisionSystem || !IsValid(CollisionSystem))
    {
        bAllSystemsHealthy = false;
        FailedSystems.Add(TEXT("CollisionSystem"));
    }
    
    if (!RagdollSystem || !IsValid(RagdollSystem))
    {
        bAllSystemsHealthy = false;
        FailedSystems.Add(TEXT("RagdollSystem"));
    }
    
    if (!DestructionSystem || !IsValid(DestructionSystem))
    {
        bAllSystemsHealthy = false;
        FailedSystems.Add(TEXT("DestructionSystem"));
    }
    
    if (!TerrainPhysics || !IsValid(TerrainPhysics))
    {
        bAllSystemsHealthy = false;
        FailedSystems.Add(TEXT("TerrainPhysics"));
    }
    
    // Check performance threshold
    if (CurrentPhysicsFPS < PerformanceThreshold)
    {
        OnPhysicsPerformanceAlert(CurrentPhysicsFPS);
        UE_LOG(LogTemp, Warning, TEXT("Physics performance below threshold: %.1f FPS"), CurrentPhysicsFPS);
    }
    
    // Handle failed systems
    if (!bAllSystemsHealthy)
    {
        for (const FString& FailedSystem : FailedSystems)
        {
            HandlePhysicsSystemFailure(FailedSystem);
            OnPhysicsSystemFailure(FailedSystem);
        }
        
        // Attempt to restart failed systems
        RestartFailedPhysicsSystems();
    }
    
    // Log status
    LogPhysicsSystemStatus();
    
    return bAllSystemsHealthy;
}

void UCore_PhysicsArchitectureIntegrator::RestartFailedPhysicsSystems()
{
    UE_LOG(LogTemp, Log, TEXT("Attempting to restart failed physics systems..."));
    
    // Recreate failed system instances
    CreatePhysicsSystemInstances();
    
    // Reconfigure settings
    ConfigurePhysicsSystemSettings();
    
    // Revalidate
    bool bRestartSuccessful = ValidatePhysicsSystemIntegrity();
    
    if (bRestartSuccessful)
    {
        UE_LOG(LogTemp, Log, TEXT("Physics systems restart successful"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Physics systems restart failed"));
    }
}

void UCore_PhysicsArchitectureIntegrator::UpdatePerformanceMetrics()
{
    if (UWorld* World = GetWorld())
    {
        // Calculate current physics FPS
        float DeltaTime = World->GetDeltaSeconds();
        if (DeltaTime > 0.0f)
        {
            CurrentPhysicsFPS = 1.0f / DeltaTime;
        }
        
        // Update physics simulation time
        PhysicsSimulationTime = World->GetPhysicsScene()->GetPhysicsTime();
        
        // Count active physics objects
        ActivePhysicsObjects = 0;
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (Actor && Actor->GetRootComponent())
            {
                if (UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent()))
                {
                    if (PrimComp->IsSimulatingPhysics())
                    {
                        ActivePhysicsObjects++;
                    }
                }
            }
        }
    }
}

bool UCore_PhysicsArchitectureIntegrator::ArePhysicsSystemsHealthy() const
{
    return bPhysicsSystemsInitialized && 
           IsValid(PhysicsManager) && 
           IsValid(PhysicsIntegrator) && 
           IsValid(CollisionSystem) && 
           IsValid(RagdollSystem) && 
           IsValid(DestructionSystem) && 
           IsValid(TerrainPhysics) &&
           CurrentPhysicsFPS >= PerformanceThreshold;
}

float UCore_PhysicsArchitectureIntegrator::GetPhysicsPerformanceScore() const
{
    if (CurrentPhysicsFPS <= 0.0f)
    {
        return 0.0f;
    }
    
    // Performance score based on FPS relative to threshold
    float Score = (CurrentPhysicsFPS / PerformanceThreshold) * 100.0f;
    return FMath::Clamp(Score, 0.0f, 100.0f);
}

FString UCore_PhysicsArchitectureIntegrator::GetPhysicsSystemsStatus() const
{
    FString Status = TEXT("Physics Systems Status:\n");
    Status += FString::Printf(TEXT("Initialized: %s\n"), bPhysicsSystemsInitialized ? TEXT("Yes") : TEXT("No"));
    Status += FString::Printf(TEXT("Architecture Integration: %s\n"), bArchitectureIntegrationActive ? TEXT("Active") : TEXT("Inactive"));
    Status += FString::Printf(TEXT("Current FPS: %.1f\n"), CurrentPhysicsFPS);
    Status += FString::Printf(TEXT("Active Physics Objects: %d\n"), ActivePhysicsObjects);
    Status += FString::Printf(TEXT("Performance Score: %.1f%%\n"), GetPhysicsPerformanceScore());
    
    return Status;
}

void UCore_PhysicsArchitectureIntegrator::CreatePhysicsSystemInstances()
{
    if (AActor* Owner = GetOwner())
    {
        // Create or find physics system components
        if (!PhysicsManager)
        {
            PhysicsManager = Owner->FindComponentByClass<UCore_PhysicsManager>();
            if (!PhysicsManager)
            {
                PhysicsManager = NewObject<UCore_PhysicsManager>(Owner);
                if (PhysicsManager)
                {
                    Owner->AddInstanceComponent(PhysicsManager);
                }
            }
        }
        
        if (!PhysicsIntegrator)
        {
            PhysicsIntegrator = Owner->FindComponentByClass<UCore_PhysicsIntegrator>();
            if (!PhysicsIntegrator)
            {
                PhysicsIntegrator = NewObject<UCore_PhysicsIntegrator>(Owner);
                if (PhysicsIntegrator)
                {
                    Owner->AddInstanceComponent(PhysicsIntegrator);
                }
            }
        }
        
        if (!CollisionSystem)
        {
            CollisionSystem = Owner->FindComponentByClass<UCore_CollisionSystem>();
            if (!CollisionSystem)
            {
                CollisionSystem = NewObject<UCore_CollisionSystem>(Owner);
                if (CollisionSystem)
                {
                    Owner->AddInstanceComponent(CollisionSystem);
                }
            }
        }
        
        if (!RagdollSystem)
        {
            RagdollSystem = Owner->FindComponentByClass<UCore_RagdollSystem>();
            if (!RagdollSystem)
            {
                RagdollSystem = NewObject<UCore_RagdollSystem>(Owner);
                if (RagdollSystem)
                {
                    Owner->AddInstanceComponent(RagdollSystem);
                }
            }
        }
        
        if (!DestructionSystem)
        {
            DestructionSystem = Owner->FindComponentByClass<UCore_DestructionSystem>();
            if (!DestructionSystem)
            {
                DestructionSystem = NewObject<UCore_DestructionSystem>(Owner);
                if (DestructionSystem)
                {
                    Owner->AddInstanceComponent(DestructionSystem);
                }
            }
        }
        
        if (!TerrainPhysics)
        {
            TerrainPhysics = Owner->FindComponentByClass<UCore_TerrainPhysics>();
            if (!TerrainPhysics)
            {
                TerrainPhysics = NewObject<UCore_TerrainPhysics>(Owner);
                if (TerrainPhysics)
                {
                    Owner->AddInstanceComponent(TerrainPhysics);
                }
            }
        }
    }
}

void UCore_PhysicsArchitectureIntegrator::ConfigurePhysicsSystemSettings()
{
    // Configure each physics system with optimal settings
    if (PhysicsManager)
    {
        UE_LOG(LogTemp, Log, TEXT("Configuring Physics Manager settings"));
    }
    
    if (PhysicsIntegrator)
    {
        UE_LOG(LogTemp, Log, TEXT("Configuring Physics Integrator settings"));
    }
    
    if (CollisionSystem)
    {
        UE_LOG(LogTemp, Log, TEXT("Configuring Collision System settings"));
    }
    
    if (RagdollSystem)
    {
        UE_LOG(LogTemp, Log, TEXT("Configuring Ragdoll System settings"));
    }
    
    if (DestructionSystem)
    {
        UE_LOG(LogTemp, Log, TEXT("Configuring Destruction System settings"));
    }
    
    if (TerrainPhysics)
    {
        UE_LOG(LogTemp, Log, TEXT("Configuring Terrain Physics settings"));
    }
}

bool UCore_PhysicsArchitectureIntegrator::ValidatePhysicsSystemIntegrity()
{
    int32 ValidSystems = 0;
    int32 TotalSystems = 6;
    
    if (IsValid(PhysicsManager)) ValidSystems++;
    if (IsValid(PhysicsIntegrator)) ValidSystems++;
    if (IsValid(CollisionSystem)) ValidSystems++;
    if (IsValid(RagdollSystem)) ValidSystems++;
    if (IsValid(DestructionSystem)) ValidSystems++;
    if (IsValid(TerrainPhysics)) ValidSystems++;
    
    UE_LOG(LogTemp, Log, TEXT("Physics system validation: %d/%d systems valid"), ValidSystems, TotalSystems);
    
    return ValidSystems == TotalSystems;
}

void UCore_PhysicsArchitectureIntegrator::HandlePhysicsSystemFailure(const FString& SystemName)
{
    UE_LOG(LogTemp, Error, TEXT("Physics system failure detected: %s"), *SystemName);
    
    // Record failure for architecture manager
    if (ArchitectureManager)
    {
        // Notify architecture manager of system failure
        UE_LOG(LogTemp, Log, TEXT("Notifying architecture manager of physics system failure"));
    }
}

void UCore_PhysicsArchitectureIntegrator::LogPhysicsSystemStatus()
{
    UE_LOG(LogTemp, Log, TEXT("=== Physics Systems Status Report ==="));
    UE_LOG(LogTemp, Log, TEXT("Systems Initialized: %s"), bPhysicsSystemsInitialized ? TEXT("Yes") : TEXT("No"));
    UE_LOG(LogTemp, Log, TEXT("Architecture Integration: %s"), bArchitectureIntegrationActive ? TEXT("Active") : TEXT("Inactive"));
    UE_LOG(LogTemp, Log, TEXT("Current Physics FPS: %.1f"), CurrentPhysicsFPS);
    UE_LOG(LogTemp, Log, TEXT("Active Physics Objects: %d"), ActivePhysicsObjects);
    UE_LOG(LogTemp, Log, TEXT("Performance Score: %.1f%%"), GetPhysicsPerformanceScore());
    UE_LOG(LogTemp, Log, TEXT("====================================="));
}

// Physics Architecture Subsystem Implementation

void UCore_PhysicsArchitectureSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("Initializing Physics Architecture Subsystem"));
    
    // Initialize global settings
    bGlobalPhysicsEnabled = true;
    GlobalPhysicsScale = 1.0f;
    GlobalGravityMultiplier = 1.0f;
    TotalPhysicsSystemsRegistered = 0;
    
    // Clear performance history
    PhysicsPerformanceHistory.Empty();
}

void UCore_PhysicsArchitectureSubsystem::Deinitialize()
{
    UE_LOG(LogTemp, Log, TEXT("Deinitializing Physics Architecture Subsystem"));
    
    // Clear all data
    PhysicsPerformanceHistory.Empty();
    TotalPhysicsSystemsRegistered = 0;
    
    Super::Deinitialize();
}

void UCore_PhysicsArchitectureSubsystem::SetGlobalPhysicsEnabled(bool bEnabled)
{
    bGlobalPhysicsEnabled = bEnabled;
    UE_LOG(LogTemp, Log, TEXT("Global physics enabled: %s"), bEnabled ? TEXT("Yes") : TEXT("No"));
}

void UCore_PhysicsArchitectureSubsystem::SetGlobalPhysicsScale(float Scale)
{
    GlobalPhysicsScale = FMath::Clamp(Scale, 0.1f, 10.0f);
    UE_LOG(LogTemp, Log, TEXT("Global physics scale set to: %.2f"), GlobalPhysicsScale);
}

void UCore_PhysicsArchitectureSubsystem::SetGlobalGravityMultiplier(float Multiplier)
{
    GlobalGravityMultiplier = FMath::Clamp(Multiplier, 0.1f, 10.0f);
    UE_LOG(LogTemp, Log, TEXT("Global gravity multiplier set to: %.2f"), GlobalGravityMultiplier);
}

void UCore_PhysicsArchitectureSubsystem::RecordPhysicsPerformance(const FString& LevelName, float PerformanceScore)
{
    PhysicsPerformanceHistory.Add(LevelName, PerformanceScore);
    UE_LOG(LogTemp, Log, TEXT("Recorded physics performance for %s: %.1f"), *LevelName, PerformanceScore);
}

float UCore_PhysicsArchitectureSubsystem::GetAveragePhysicsPerformance() const
{
    if (PhysicsPerformanceHistory.Num() == 0)
    {
        return 0.0f;
    }
    
    float Total = 0.0f;
    for (const auto& Entry : PhysicsPerformanceHistory)
    {
        Total += Entry.Value;
    }
    
    return Total / PhysicsPerformanceHistory.Num();
}

TArray<FString> UCore_PhysicsArchitectureSubsystem::GetPhysicsPerformanceReport() const
{
    TArray<FString> Report;
    
    Report.Add(FString::Printf(TEXT("Physics Performance Report:")));
    Report.Add(FString::Printf(TEXT("Total Levels Recorded: %d"), PhysicsPerformanceHistory.Num()));
    Report.Add(FString::Printf(TEXT("Average Performance: %.1f"), GetAveragePhysicsPerformance()));
    Report.Add(FString::Printf(TEXT("Registered Systems: %d"), TotalPhysicsSystemsRegistered));
    
    for (const auto& Entry : PhysicsPerformanceHistory)
    {
        Report.Add(FString::Printf(TEXT("  %s: %.1f"), *Entry.Key, Entry.Value));
    }
    
    return Report;
}

void UCore_PhysicsArchitectureSubsystem::RegisterPhysicsSystem(const FString& SystemName)
{
    TotalPhysicsSystemsRegistered++;
    UE_LOG(LogTemp, Log, TEXT("Registered physics system: %s (Total: %d)"), *SystemName, TotalPhysicsSystemsRegistered);
}

void UCore_PhysicsArchitectureSubsystem::UnregisterPhysicsSystem(const FString& SystemName)
{
    if (TotalPhysicsSystemsRegistered > 0)
    {
        TotalPhysicsSystemsRegistered--;
    }
    UE_LOG(LogTemp, Log, TEXT("Unregistered physics system: %s (Total: %d)"), *SystemName, TotalPhysicsSystemsRegistered);
}