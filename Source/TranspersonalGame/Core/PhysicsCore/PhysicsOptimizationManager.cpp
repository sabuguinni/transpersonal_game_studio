#include "PhysicsOptimizationManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Chaos/ChaosEngineInterface.h"
#include "TranspersonalGame/TranspersonalGame.h"

DEFINE_LOG_CATEGORY_STATIC(LogPhysicsOptimization, Log, All);

UCore_PhysicsOptimizationManager::UCore_PhysicsOptimizationManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Tick every 100ms for optimization
    
    // Default optimization settings
    MaxPhysicsObjects = 500;
    PhysicsLODDistance = 2000.0f;
    CullingDistance = 5000.0f;
    PerformanceTargetFPS = 60.0f;
    AdaptiveQualityEnabled = true;
    
    // Performance thresholds
    HighPerformanceThreshold = 55.0f;
    LowPerformanceThreshold = 45.0f;
    CriticalPerformanceThreshold = 30.0f;
    
    // Optimization parameters
    MaxSimulationSteps = 8;
    MinSimulationSteps = 2;
    CurrentSimulationSteps = 6;
    
    // Culling settings
    CullingCheckInterval = 0.5f;
    LastCullingCheck = 0.0f;
    
    // Statistics
    ActivePhysicsObjects = 0;
    CulledPhysicsObjects = 0;
    OptimizationLevel = ECore_PhysicsOptimizationLevel::Medium;
}

void UCore_PhysicsOptimizationManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogPhysicsOptimization, Log, TEXT("Physics Optimization Manager initialized"));
    
    // Initialize performance monitoring
    InitializePerformanceMonitoring();
    
    // Set up initial optimization state
    UpdateOptimizationLevel();
}

void UCore_PhysicsOptimizationManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update performance metrics
    UpdatePerformanceMetrics(DeltaTime);
    
    // Check if we need to adjust optimization level
    if (AdaptiveQualityEnabled)
    {
        CheckAndUpdateOptimizationLevel();
    }
    
    // Perform culling check at intervals
    LastCullingCheck += DeltaTime;
    if (LastCullingCheck >= CullingCheckInterval)
    {
        PerformDistanceCulling();
        LastCullingCheck = 0.0f;
    }
    
    // Update physics LOD
    UpdatePhysicsLOD();
}

void UCore_PhysicsOptimizationManager::InitializePerformanceMonitoring()
{
    CurrentFPS = 60.0f;
    AverageFPS = 60.0f;
    FrameTimeHistory.Empty();
    FrameTimeHistory.Reserve(60); // Store last 60 frames for averaging
    
    UE_LOG(LogPhysicsOptimization, Log, TEXT("Performance monitoring initialized"));
}

void UCore_PhysicsOptimizationManager::UpdatePerformanceMetrics(float DeltaTime)
{
    // Calculate current FPS
    if (DeltaTime > 0.0f)
    {
        CurrentFPS = 1.0f / DeltaTime;
        
        // Add to history
        FrameTimeHistory.Add(DeltaTime);
        if (FrameTimeHistory.Num() > 60)
        {
            FrameTimeHistory.RemoveAt(0);
        }
        
        // Calculate average FPS
        if (FrameTimeHistory.Num() > 0)
        {
            float AverageFrameTime = 0.0f;
            for (float FrameTime : FrameTimeHistory)
            {
                AverageFrameTime += FrameTime;
            }
            AverageFrameTime /= FrameTimeHistory.Num();
            AverageFPS = 1.0f / AverageFrameTime;
        }
    }
}

void UCore_PhysicsOptimizationManager::CheckAndUpdateOptimizationLevel()
{
    ECore_PhysicsOptimizationLevel NewLevel = OptimizationLevel;
    
    if (AverageFPS < CriticalPerformanceThreshold)
    {
        NewLevel = ECore_PhysicsOptimizationLevel::Ultra;
    }
    else if (AverageFPS < LowPerformanceThreshold)
    {
        NewLevel = ECore_PhysicsOptimizationLevel::High;
    }
    else if (AverageFPS < HighPerformanceThreshold)
    {
        NewLevel = ECore_PhysicsOptimizationLevel::Medium;
    }
    else
    {
        NewLevel = ECore_PhysicsOptimizationLevel::Low;
    }
    
    if (NewLevel != OptimizationLevel)
    {
        SetOptimizationLevel(NewLevel);
        UE_LOG(LogPhysicsOptimization, Warning, TEXT("Optimization level changed to %d due to FPS: %.1f"), 
               (int32)NewLevel, AverageFPS);
    }
}

void UCore_PhysicsOptimizationManager::SetOptimizationLevel(ECore_PhysicsOptimizationLevel NewLevel)
{
    OptimizationLevel = NewLevel;
    UpdateOptimizationLevel();
    
    // Broadcast optimization level change
    OnOptimizationLevelChanged.Broadcast(OptimizationLevel);
}

void UCore_PhysicsOptimizationManager::UpdateOptimizationLevel()
{
    switch (OptimizationLevel)
    {
        case ECore_PhysicsOptimizationLevel::Low:
            CurrentSimulationSteps = MaxSimulationSteps;
            PhysicsLODDistance = 3000.0f;
            CullingDistance = 6000.0f;
            break;
            
        case ECore_PhysicsOptimizationLevel::Medium:
            CurrentSimulationSteps = (MaxSimulationSteps + MinSimulationSteps) / 2;
            PhysicsLODDistance = 2000.0f;
            CullingDistance = 5000.0f;
            break;
            
        case ECore_PhysicsOptimizationLevel::High:
            CurrentSimulationSteps = MinSimulationSteps + 1;
            PhysicsLODDistance = 1500.0f;
            CullingDistance = 4000.0f;
            break;
            
        case ECore_PhysicsOptimizationLevel::Ultra:
            CurrentSimulationSteps = MinSimulationSteps;
            PhysicsLODDistance = 1000.0f;
            CullingDistance = 3000.0f;
            break;
    }
    
    // Apply physics settings
    ApplyPhysicsSettings();
    
    UE_LOG(LogPhysicsOptimization, Log, TEXT("Updated optimization level: %d, Steps: %d"), 
           (int32)OptimizationLevel, CurrentSimulationSteps);
}

void UCore_PhysicsOptimizationManager::ApplyPhysicsSettings()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Apply simulation step settings
    if (UPhysicsSettings* PhysicsSettings = GetMutableDefault<UPhysicsSettings>())
    {
        PhysicsSettings->MaxSubsteps = CurrentSimulationSteps;
        PhysicsSettings->MaxSubstepDeltaTime = 0.016f / CurrentSimulationSteps;
    }
}

void UCore_PhysicsOptimizationManager::PerformDistanceCulling()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    APawn* PlayerPawn = World->GetFirstPlayerController() ? World->GetFirstPlayerController()->GetPawn() : nullptr;
    if (!PlayerPawn)
    {
        return;
    }
    
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    ActivePhysicsObjects = 0;
    CulledPhysicsObjects = 0;
    
    // Iterate through all actors with physics
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor || Actor == PlayerPawn)
        {
            continue;
        }
        
        // Check static mesh components
        TArray<UStaticMeshComponent*> StaticMeshComponents;
        Actor->GetComponents<UStaticMeshComponent>(StaticMeshComponents);
        
        for (UStaticMeshComponent* MeshComp : StaticMeshComponents)
        {
            if (MeshComp && MeshComp->IsSimulatingPhysics())
            {
                float Distance = FVector::Dist(PlayerLocation, Actor->GetActorLocation());
                
                if (Distance > CullingDistance)
                {
                    // Disable physics simulation
                    MeshComp->SetSimulatePhysics(false);
                    CulledPhysicsObjects++;
                }
                else if (Distance <= PhysicsLODDistance)
                {
                    // Enable full physics simulation
                    if (!MeshComp->IsSimulatingPhysics())
                    {
                        MeshComp->SetSimulatePhysics(true);
                    }
                    ActivePhysicsObjects++;
                }
                else
                {
                    // Reduced physics simulation (LOD)
                    if (!MeshComp->IsSimulatingPhysics())
                    {
                        MeshComp->SetSimulatePhysics(true);
                    }
                    ActivePhysicsObjects++;
                }
            }
        }
    }
    
    // Enforce maximum physics objects limit
    if (ActivePhysicsObjects > MaxPhysicsObjects)
    {
        ReducePhysicsObjects(ActivePhysicsObjects - MaxPhysicsObjects);
    }
}

void UCore_PhysicsOptimizationManager::UpdatePhysicsLOD()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    APawn* PlayerPawn = World->GetFirstPlayerController() ? World->GetFirstPlayerController()->GetPawn() : nullptr;
    if (!PlayerPawn)
    {
        return;
    }
    
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    
    // Update physics LOD based on distance
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor || Actor == PlayerPawn)
        {
            continue;
        }
        
        float Distance = FVector::Dist(PlayerLocation, Actor->GetActorLocation());
        
        // Apply LOD to skeletal mesh components (for ragdolls)
        TArray<USkeletalMeshComponent*> SkeletalMeshComponents;
        Actor->GetComponents<USkeletalMeshComponent>(SkeletalMeshComponents);
        
        for (USkeletalMeshComponent* SkeletalMeshComp : SkeletalMeshComponents)
        {
            if (SkeletalMeshComp)
            {
                if (Distance > PhysicsLODDistance * 2.0f)
                {
                    // Disable physics on distant skeletal meshes
                    SkeletalMeshComp->SetAllBodiesSimulatePhysics(false);
                }
                else if (Distance > PhysicsLODDistance)
                {
                    // Reduced physics simulation
                    SkeletalMeshComp->SetAllBodiesSimulatePhysics(true);
                    // Could add additional LOD logic here
                }
                else
                {
                    // Full physics simulation
                    SkeletalMeshComp->SetAllBodiesSimulatePhysics(true);
                }
            }
        }
    }
}

void UCore_PhysicsOptimizationManager::ReducePhysicsObjects(int32 ObjectsToReduce)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    APawn* PlayerPawn = World->GetFirstPlayerController() ? World->GetFirstPlayerController()->GetPawn() : nullptr;
    if (!PlayerPawn)
    {
        return;
    }
    
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    TArray<TPair<float, UStaticMeshComponent*>> PhysicsComponents;
    
    // Collect all physics components with their distances
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor || Actor == PlayerPawn)
        {
            continue;
        }
        
        TArray<UStaticMeshComponent*> StaticMeshComponents;
        Actor->GetComponents<UStaticMeshComponent>(StaticMeshComponents);
        
        for (UStaticMeshComponent* MeshComp : StaticMeshComponents)
        {
            if (MeshComp && MeshComp->IsSimulatingPhysics())
            {
                float Distance = FVector::Dist(PlayerLocation, Actor->GetActorLocation());
                PhysicsComponents.Add(TPair<float, UStaticMeshComponent*>(Distance, MeshComp));
            }
        }
    }
    
    // Sort by distance (farthest first)
    PhysicsComponents.Sort([](const TPair<float, UStaticMeshComponent*>& A, const TPair<float, UStaticMeshComponent*>& B)
    {
        return A.Key > B.Key;
    });
    
    // Disable physics on the farthest objects
    int32 DisabledCount = 0;
    for (const auto& Pair : PhysicsComponents)
    {
        if (DisabledCount >= ObjectsToReduce)
        {
            break;
        }
        
        Pair.Value->SetSimulatePhysics(false);
        DisabledCount++;
        CulledPhysicsObjects++;
    }
    
    UE_LOG(LogPhysicsOptimization, Warning, TEXT("Reduced %d physics objects to maintain performance"), DisabledCount);
}

void UCore_PhysicsOptimizationManager::EnableAdaptiveQuality(bool bEnable)
{
    AdaptiveQualityEnabled = bEnable;
    UE_LOG(LogPhysicsOptimization, Log, TEXT("Adaptive quality %s"), bEnable ? TEXT("enabled") : TEXT("disabled"));
}

void UCore_PhysicsOptimizationManager::SetMaxPhysicsObjects(int32 MaxObjects)
{
    MaxPhysicsObjects = FMath::Max(10, MaxObjects);
    UE_LOG(LogPhysicsOptimization, Log, TEXT("Max physics objects set to %d"), MaxPhysicsObjects);
}

void UCore_PhysicsOptimizationManager::SetPhysicsLODDistance(float Distance)
{
    PhysicsLODDistance = FMath::Max(100.0f, Distance);
    UE_LOG(LogPhysicsOptimization, Log, TEXT("Physics LOD distance set to %.1f"), PhysicsLODDistance);
}

void UCore_PhysicsOptimizationManager::SetCullingDistance(float Distance)
{
    CullingDistance = FMath::Max(PhysicsLODDistance, Distance);
    UE_LOG(LogPhysicsOptimization, Log, TEXT("Culling distance set to %.1f"), CullingDistance);
}

FCore_PhysicsOptimizationStats UCore_PhysicsOptimizationManager::GetOptimizationStats() const
{
    FCore_PhysicsOptimizationStats Stats;
    Stats.CurrentFPS = CurrentFPS;
    Stats.AverageFPS = AverageFPS;
    Stats.OptimizationLevel = OptimizationLevel;
    Stats.ActivePhysicsObjects = ActivePhysicsObjects;
    Stats.CulledPhysicsObjects = CulledPhysicsObjects;
    Stats.CurrentSimulationSteps = CurrentSimulationSteps;
    Stats.PhysicsLODDistance = PhysicsLODDistance;
    Stats.CullingDistance = CullingDistance;
    
    return Stats;
}

void UCore_PhysicsOptimizationManager::ResetOptimization()
{
    OptimizationLevel = ECore_PhysicsOptimizationLevel::Medium;
    UpdateOptimizationLevel();
    
    // Re-enable physics on all objects
    UWorld* World = GetWorld();
    if (World)
    {
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (!Actor)
            {
                continue;
            }
            
            TArray<UStaticMeshComponent*> StaticMeshComponents;
            Actor->GetComponents<UStaticMeshComponent>(StaticMeshComponents);
            
            for (UStaticMeshComponent* MeshComp : StaticMeshComponents)
            {
                if (MeshComp && MeshComp->GetStaticMesh())
                {
                    // Reset to default physics state based on mesh settings
                    bool bShouldSimulate = MeshComp->GetStaticMesh()->GetBodySetup() != nullptr;
                    MeshComp->SetSimulatePhysics(bShouldSimulate);
                }
            }
        }
    }
    
    CulledPhysicsObjects = 0;
    UE_LOG(LogPhysicsOptimization, Log, TEXT("Physics optimization reset"));
}