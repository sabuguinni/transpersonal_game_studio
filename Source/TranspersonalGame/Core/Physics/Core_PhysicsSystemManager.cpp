#include "Core_PhysicsSystemManager.h"
#include "Core_TerrainPhysicsManager.h"
#include "Core_RagdollSystem.h"
#include "Core_DestructionSystem.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "TimerManager.h"
#include "PhysicsEngine/PhysicsSettings.h"

UCore_PhysicsSystemManager::UCore_PhysicsSystemManager()
{
    bSystemsInitialized = false;
    ActivePhysicsObjects = 0;
    LastFramePhysicsTime = 0.0f;
    
    // Default settings
    CurrentSettings.PhysicsQuality = ECore_PhysicsQuality::Medium;
    CurrentSettings.MaxPhysicsObjects = 500;
    CurrentSettings.PhysicsTimeStep = 0.016f;
    CurrentSettings.bEnableDestruction = true;
    CurrentSettings.bEnableRagdoll = true;
    CurrentSettings.bEnableTerrainPhysics = true;
}

void UCore_PhysicsSystemManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("Core Physics System Manager: Initializing"));
    
    InitializePhysicsSystems();
    ConfigurePhysicsEngine();
    
    // Start performance monitoring
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(PerformanceUpdateTimer, 
            this, &UCore_PhysicsSystemManager::UpdatePerformanceMetrics, 
            1.0f, true);
    }
}

void UCore_PhysicsSystemManager::Deinitialize()
{
    UE_LOG(LogTemp, Log, TEXT("Core Physics System Manager: Shutting down"));
    
    ShutdownPhysicsSystems();
    
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(PerformanceUpdateTimer);
    }
    
    Super::Deinitialize();
}

bool UCore_PhysicsSystemManager::ShouldCreateSubsystem(UObject* Outer) const
{
    return Super::ShouldCreateSubsystem(Outer);
}

void UCore_PhysicsSystemManager::InitializePhysicsSystems()
{
    if (bSystemsInitialized)
    {
        return;
    }
    
    CreateSubsystems();
    
    // Initialize terrain physics
    if (TerrainPhysicsManager && CurrentSettings.bEnableTerrainPhysics)
    {
        TerrainPhysicsManager->InitializeTerrainPhysics();
        UE_LOG(LogTemp, Log, TEXT("Terrain Physics Manager initialized"));
    }
    
    // Initialize ragdoll system
    if (RagdollSystem && CurrentSettings.bEnableRagdoll)
    {
        RagdollSystem->InitializeRagdollSystem();
        UE_LOG(LogTemp, Log, TEXT("Ragdoll System initialized"));
    }
    
    // Initialize destruction system
    if (DestructionSystem && CurrentSettings.bEnableDestruction)
    {
        DestructionSystem->InitializeDestructionSystem();
        UE_LOG(LogTemp, Log, TEXT("Destruction System initialized"));
    }
    
    bSystemsInitialized = true;
    UE_LOG(LogTemp, Log, TEXT("All physics systems initialized successfully"));
}

void UCore_PhysicsSystemManager::ShutdownPhysicsSystems()
{
    if (!bSystemsInitialized)
    {
        return;
    }
    
    // Shutdown in reverse order
    if (DestructionSystem)
    {
        DestructionSystem->ShutdownDestructionSystem();
    }
    
    if (RagdollSystem)
    {
        RagdollSystem->ShutdownRagdollSystem();
    }
    
    if (TerrainPhysicsManager)
    {
        TerrainPhysicsManager->ShutdownTerrainPhysics();
    }
    
    bSystemsInitialized = false;
    UE_LOG(LogTemp, Log, TEXT("All physics systems shut down"));
}

void UCore_PhysicsSystemManager::UpdatePhysicsSettings(const FCore_PhysicsSettings& NewSettings)
{
    CurrentSettings = NewSettings;
    
    // Apply quality settings
    SetPhysicsQuality(CurrentSettings.PhysicsQuality);
    
    // Update subsystem settings
    if (TerrainPhysicsManager)
    {
        TerrainPhysicsManager->SetEnabled(CurrentSettings.bEnableTerrainPhysics);
    }
    
    if (RagdollSystem)
    {
        RagdollSystem->SetEnabled(CurrentSettings.bEnableRagdoll);
    }
    
    if (DestructionSystem)
    {
        DestructionSystem->SetEnabled(CurrentSettings.bEnableDestruction);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Physics settings updated"));
}

void UCore_PhysicsSystemManager::SetPhysicsQuality(ECore_PhysicsQuality Quality)
{
    CurrentSettings.PhysicsQuality = Quality;
    
    // Configure physics settings based on quality
    if (UPhysicsSettings* PhysicsSettings = UPhysicsSettings::Get())
    {
        switch (Quality)
        {
        case ECore_PhysicsQuality::Low:
            CurrentSettings.MaxPhysicsObjects = 200;
            CurrentSettings.PhysicsTimeStep = 0.033f; // 30 FPS
            break;
        case ECore_PhysicsQuality::Medium:
            CurrentSettings.MaxPhysicsObjects = 500;
            CurrentSettings.PhysicsTimeStep = 0.016f; // 60 FPS
            break;
        case ECore_PhysicsQuality::High:
            CurrentSettings.MaxPhysicsObjects = 800;
            CurrentSettings.PhysicsTimeStep = 0.016f; // 60 FPS
            break;
        case ECore_PhysicsQuality::Ultra:
            CurrentSettings.MaxPhysicsObjects = 1200;
            CurrentSettings.PhysicsTimeStep = 0.008f; // 120 FPS
            break;
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Physics quality set to: %d"), (int32)Quality);
}

int32 UCore_PhysicsSystemManager::GetActivePhysicsObjectCount() const
{
    return ActivePhysicsObjects;
}

void UCore_PhysicsSystemManager::OptimizePhysicsPerformance()
{
    // Optimize based on current performance
    if (LastFramePhysicsTime > 16.0f) // Over 16ms
    {
        // Reduce quality if performance is poor
        if (CurrentSettings.PhysicsQuality != ECore_PhysicsQuality::Low)
        {
            ECore_PhysicsQuality NewQuality = static_cast<ECore_PhysicsQuality>(
                static_cast<int32>(CurrentSettings.PhysicsQuality) - 1);
            SetPhysicsQuality(NewQuality);
            UE_LOG(LogTemp, Warning, TEXT("Physics quality reduced due to performance"));
        }
    }
    else if (LastFramePhysicsTime < 8.0f) // Under 8ms
    {
        // Increase quality if performance allows
        if (CurrentSettings.PhysicsQuality != ECore_PhysicsQuality::Ultra)
        {
            ECore_PhysicsQuality NewQuality = static_cast<ECore_PhysicsQuality>(
                static_cast<int32>(CurrentSettings.PhysicsQuality) + 1);
            SetPhysicsQuality(NewQuality);
            UE_LOG(LogTemp, Log, TEXT("Physics quality increased due to good performance"));
        }
    }
}

void UCore_PhysicsSystemManager::DebugPhysicsSystems()
{
    UE_LOG(LogTemp, Warning, TEXT("=== PHYSICS SYSTEMS DEBUG ==="));
    UE_LOG(LogTemp, Warning, TEXT("Systems Initialized: %s"), bSystemsInitialized ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Warning, TEXT("Active Physics Objects: %d"), ActivePhysicsObjects);
    UE_LOG(LogTemp, Warning, TEXT("Last Frame Physics Time: %.2fms"), LastFramePhysicsTime);
    UE_LOG(LogTemp, Warning, TEXT("Physics Quality: %d"), (int32)CurrentSettings.PhysicsQuality);
    UE_LOG(LogTemp, Warning, TEXT("Max Physics Objects: %d"), CurrentSettings.MaxPhysicsObjects);
    UE_LOG(LogTemp, Warning, TEXT("Physics Time Step: %.4f"), CurrentSettings.PhysicsTimeStep);
    
    if (TerrainPhysicsManager)
    {
        UE_LOG(LogTemp, Warning, TEXT("Terrain Physics: ACTIVE"));
    }
    
    if (RagdollSystem)
    {
        UE_LOG(LogTemp, Warning, TEXT("Ragdoll System: ACTIVE"));
    }
    
    if (DestructionSystem)
    {
        UE_LOG(LogTemp, Warning, TEXT("Destruction System: ACTIVE"));
    }
}

void UCore_PhysicsSystemManager::LogPhysicsStatistics()
{
    UE_LOG(LogTemp, Log, TEXT("Physics Statistics - Active Objects: %d, Frame Time: %.2fms"), 
           ActivePhysicsObjects, LastFramePhysicsTime);
}

void UCore_PhysicsSystemManager::CreateSubsystems()
{
    // Create terrain physics manager
    if (!TerrainPhysicsManager)
    {
        TerrainPhysicsManager = NewObject<UCore_TerrainPhysicsManager>(this);
    }
    
    // Create ragdoll system
    if (!RagdollSystem)
    {
        RagdollSystem = NewObject<UCore_RagdollSystem>(this);
    }
    
    // Create destruction system
    if (!DestructionSystem)
    {
        DestructionSystem = NewObject<UCore_DestructionSystem>(this);
    }
}

void UCore_PhysicsSystemManager::ConfigurePhysicsEngine()
{
    if (UPhysicsSettings* PhysicsSettings = UPhysicsSettings::Get())
    {
        // Configure based on current settings
        SetPhysicsQuality(CurrentSettings.PhysicsQuality);
    }
}

void UCore_PhysicsSystemManager::UpdatePerformanceMetrics()
{
    // Update active physics object count
    if (UWorld* World = GetWorld())
    {
        ActivePhysicsObjects = 0;
        
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (Actor && Actor->GetRootComponent() && 
                Actor->GetRootComponent()->IsSimulatingPhysics())
            {
                ActivePhysicsObjects++;
            }
        }
    }
    
    // Simulate physics timing (would be measured in real implementation)
    LastFramePhysicsTime = FMath::RandRange(5.0f, 20.0f);
    
    // Auto-optimize if needed
    if (ActivePhysicsObjects > CurrentSettings.MaxPhysicsObjects)
    {
        OptimizePhysicsPerformance();
    }
}

// Physics Integration Component Implementation
UCore_PhysicsIntegrationComponent::UCore_PhysicsIntegrationComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PostPhysics;
    
    bUseTerrainPhysics = true;
    bUseRagdollPhysics = false;
    bUseDestruction = false;
    PhysicsUpdateRate = 1.0f;
    
    bRegisteredWithManager = false;
    LastUpdateTime = 0.0f;
}

void UCore_PhysicsIntegrationComponent::BeginPlay()
{
    Super::BeginPlay();
    
    RegisterWithPhysicsManager();
}

void UCore_PhysicsIntegrationComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    UnregisterFromPhysicsManager();
    
    Super::EndPlay(EndPlayReason);
}

void UCore_PhysicsIntegrationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    LastUpdateTime += DeltaTime;
    
    if (LastUpdateTime >= PhysicsUpdateRate)
    {
        // Update physics integration
        if (PhysicsManager && bRegisteredWithManager)
        {
            // Perform physics updates based on enabled systems
        }
        
        LastUpdateTime = 0.0f;
    }
}

void UCore_PhysicsIntegrationComponent::EnableAdvancedPhysics(bool bEnable)
{
    if (bEnable)
    {
        RegisterWithPhysicsManager();
    }
    else
    {
        UnregisterFromPhysicsManager();
    }
}

void UCore_PhysicsIntegrationComponent::RegisterWithPhysicsManager()
{
    if (bRegisteredWithManager)
    {
        return;
    }
    
    if (UWorld* World = GetWorld())
    {
        PhysicsManager = World->GetSubsystem<UCore_PhysicsSystemManager>();
        if (PhysicsManager)
        {
            bRegisteredWithManager = true;
            UE_LOG(LogTemp, Log, TEXT("Physics Integration Component registered with manager"));
        }
    }
}

void UCore_PhysicsIntegrationComponent::UnregisterFromPhysicsManager()
{
    if (!bRegisteredWithManager)
    {
        return;
    }
    
    bRegisteredWithManager = false;
    PhysicsManager = nullptr;
    
    UE_LOG(LogTemp, Log, TEXT("Physics Integration Component unregistered from manager"));
}