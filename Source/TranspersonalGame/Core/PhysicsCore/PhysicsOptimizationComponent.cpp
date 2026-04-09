#include "PhysicsOptimizationComponent.h"
#include "PhysicsSystemManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Components/PrimitiveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Engine/StaticMesh.h"

UPhysicsOptimizationComponent::UPhysicsOptimizationComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10Hz by default
    
    OptimizationLevel = EPhysicsOptimizationLevel::Medium;
    bEnableDistanceBasedLOD = true;
    bEnablePerformanceBasedLOD = true;
    bEnableMemoryOptimization = true;
    
    TargetFrameTime = 16.67f; // 60 FPS
    MaxPhysicsTime = 8.0f;
    
    LODUpdateFrequency = 0.5f;
    HysteresisDistance = 50.0f;
    MetricsUpdateFrequency = 1.0f;
    
    // Initialize default LOD levels
    LODLevels.SetNum(5);
    
    // Full Detail (0-500m)
    LODLevels[0].DistanceThreshold = 500.0f;
    LODLevels[0].LODState = EPhysicsLODState::FullDetail;
    LODLevels[0].CollisionSimplificationFactor = 1.0f;
    LODLevels[0].bDisableComplexCollision = false;
    LODLevels[0].TickFrequencyMultiplier = 1.0f;
    
    // Reduced Detail (500-1000m)
    LODLevels[1].DistanceThreshold = 1000.0f;
    LODLevels[1].LODState = EPhysicsLODState::ReducedDetail;
    LODLevels[1].CollisionSimplificationFactor = 0.75f;
    LODLevels[1].bDisableComplexCollision = false;
    LODLevels[1].TickFrequencyMultiplier = 0.75f;
    
    // Simplified Physics (1000-2000m)
    LODLevels[2].DistanceThreshold = 2000.0f;
    LODLevels[2].LODState = EPhysicsLODState::SimplifiedPhysics;
    LODLevels[2].CollisionSimplificationFactor = 0.5f;
    LODLevels[2].bDisableComplexCollision = true;
    LODLevels[2].TickFrequencyMultiplier = 0.5f;
    
    // Static Only (2000-3000m)
    LODLevels[3].DistanceThreshold = 3000.0f;
    LODLevels[3].LODState = EPhysicsLODState::StaticOnly;
    LODLevels[3].CollisionSimplificationFactor = 0.25f;
    LODLevels[3].bDisableComplexCollision = true;
    LODLevels[3].TickFrequencyMultiplier = 0.1f;
    
    // Disabled (3000m+)
    LODLevels[4].DistanceThreshold = FLT_MAX;
    LODLevels[4].LODState = EPhysicsLODState::Disabled;
    LODLevels[4].CollisionSimplificationFactor = 0.0f;
    LODLevels[4].bDisableComplexCollision = true;
    LODLevels[4].TickFrequencyMultiplier = 0.0f;
}

void UPhysicsOptimizationComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Find the physics system manager
    if (UWorld* World = GetWorld())
    {
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            if (UPhysicsSystemManager* Manager = ActorItr->FindComponentByClass<UPhysicsSystemManager>())
            {
                PhysicsManager = Manager;
                break;
            }
        }
    }
    
    // Initialize metrics
    ResetMetrics();
    
    UE_LOG(LogTemp, Log, TEXT("PhysicsOptimizationComponent: BeginPlay - Optimization Level: %d"), 
           (int32)OptimizationLevel);
}

void UPhysicsOptimizationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bOptimizationEnabled)
        return;
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Update LOD system
    if (bEnableDistanceBasedLOD || bEnablePerformanceBasedLOD)
    {
        UpdateLODSystem(DeltaTime);
    }
    
    // Update performance metrics
    UpdatePerformanceMetrics(DeltaTime);
    
    // Log metrics if enabled
    if (bEnableMetricsLogging && (CurrentTime - LastMetricsUpdateTime) >= MetricsUpdateFrequency)
    {
        LogOptimizationMetrics();
        LastMetricsUpdateTime = CurrentTime;
    }
}

void UPhysicsOptimizationComponent::UpdateLODSystem(float DeltaTime)
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    if (!ShouldUpdateLOD(CurrentTime))
        return;
    
    // Calculate optimal LOD state
    EPhysicsLODState OptimalState = CalculateOptimalLODState();
    
    // Check if we need to transition
    if (OptimalState != CurrentLODState && !bLODTransitionInProgress)
    {
        // Apply hysteresis to prevent thrashing
        float DistanceDelta = FMath::Abs(DistanceToPlayer - PreviousDistanceToPlayer);
        if (DistanceDelta > HysteresisDistance || bEnablePerformanceBasedLOD)
        {
            TransitionToLODState(OptimalState);
        }
    }
    
    LastLODUpdateTime = CurrentTime;
    PreviousDistanceToPlayer = DistanceToPlayer;
}

void UPhysicsOptimizationComponent::UpdatePerformanceMetrics(float DeltaTime)
{
    AccumulatedFrameTime += DeltaTime;
    FrameCount++;
    
    // Update physics timing (simplified - would need integration with actual physics timing)
    AccumulatedPhysicsTime += DeltaTime * 0.3f; // Estimate 30% of frame time for physics
    
    // Update metrics every second
    if (AccumulatedFrameTime >= 1.0f)
    {
        CurrentMetrics.PhysicsUpdateTime = (AccumulatedPhysicsTime / FrameCount) * 1000.0f; // Convert to ms
        
        // Count physics bodies in the scene
        if (AActor* Owner = GetOwner())
        {
            TArray<UPrimitiveComponent*> PrimitiveComponents;
            Owner->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
            
            CurrentMetrics.ActivePhysicsBodies = 0;
            CurrentMetrics.SleepingBodies = 0;
            CurrentMetrics.OptimizedBodies = 0;
            
            for (UPrimitiveComponent* Primitive : PrimitiveComponents)
            {
                if (Primitive && Primitive->IsSimulatingPhysics())
                {
                    if (Primitive->IsAnyRigidBodyAwake())
                    {
                        CurrentMetrics.ActivePhysicsBodies++;
                    }
                    else
                    {
                        CurrentMetrics.SleepingBodies++;
                    }
                    
                    if (CurrentLODState != EPhysicsLODState::FullDetail)
                    {
                        CurrentMetrics.OptimizedBodies++;
                    }
                }
            }
        }
        
        // Reset accumulators
        AccumulatedFrameTime = 0.0f;
        AccumulatedPhysicsTime = 0.0f;
        FrameCount = 0;
    }
}

EPhysicsLODState UPhysicsOptimizationComponent::CalculateOptimalLODState() const
{
    DistanceToPlayer = GetDistanceToPlayer();
    
    // Performance-based LOD override
    if (bEnablePerformanceBasedLOD && !IsPerformanceTargetMet())
    {
        // If performance is poor, use more aggressive optimization
        if (CurrentLODState < EPhysicsLODState::StaticOnly)
        {
            return static_cast<EPhysicsLODState>(static_cast<int32>(CurrentLODState) + 1);
        }
    }
    
    // Distance-based LOD
    if (bEnableDistanceBasedLOD)
    {
        for (int32 i = 0; i < LODLevels.Num(); ++i)
        {
            if (DistanceToPlayer <= LODLevels[i].DistanceThreshold)
            {
                return LODLevels[i].LODState;
            }
        }
    }
    
    return EPhysicsLODState::Disabled;
}

float UPhysicsOptimizationComponent::GetDistanceToPlayer() const
{
    if (UWorld* World = GetWorld())
    {
        if (APlayerController* PC = World->GetFirstPlayerController())
        {
            if (APawn* PlayerPawn = PC->GetPawn())
            {
                if (AActor* Owner = GetOwner())
                {
                    return FVector::Dist(Owner->GetActorLocation(), PlayerPawn->GetActorLocation());
                }
            }
        }
    }
    
    return 0.0f;
}

bool UPhysicsOptimizationComponent::ShouldUpdateLOD(float CurrentTime) const
{
    return (CurrentTime - LastLODUpdateTime) >= LODUpdateFrequency;
}

void UPhysicsOptimizationComponent::TransitionToLODState(EPhysicsLODState NewState)
{
    if (NewState == CurrentLODState || bLODTransitionInProgress)
        return;
    
    if (!IsLODTransitionValid(CurrentLODState, NewState))
    {
        UE_LOG(LogTemp, Warning, TEXT("PhysicsOptimization: Invalid LOD transition from %d to %d"), 
               (int32)CurrentLODState, (int32)NewState);
        return;
    }
    
    BeginLODTransition(CurrentLODState, NewState);
    
    // Find the settings for the new state
    for (const FPhysicsLODSettings& Settings : LODLevels)
    {
        if (Settings.LODState == NewState)
        {
            ApplyLODSettings(Settings);
            break;
        }
    }
    
    PreviousLODState = CurrentLODState;
    CurrentLODState = NewState;
    
    CompleteLODTransition();
    
    UE_LOG(LogTemp, Log, TEXT("PhysicsOptimization: LOD transition from %d to %d at distance %.1f"), 
           (int32)PreviousLODState, (int32)CurrentLODState, DistanceToPlayer);
}

void UPhysicsOptimizationComponent::ApplyLODSettings(const FPhysicsLODSettings& Settings)
{
    AActor* Owner = GetOwner();
    if (!Owner) return;
    
    TArray<UPrimitiveComponent*> PrimitiveComponents;
    Owner->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
    
    for (UPrimitiveComponent* Primitive : PrimitiveComponents)
    {
        if (!Primitive) continue;
        
        UpdateCollisionSettings(Primitive, Settings);
        
        // Update tick frequency
        if (Settings.bReduceTickFrequency)
        {
            Primitive->PrimaryComponentTick.TickInterval = 
                Primitive->PrimaryComponentTick.TickInterval / Settings.TickFrequencyMultiplier;
        }
        
        // Handle physics state based on LOD
        switch (Settings.LODState)
        {
            case EPhysicsLODState::FullDetail:
                // No changes needed
                break;
                
            case EPhysicsLODState::ReducedDetail:
                // Reduce simulation precision
                if (Primitive->IsSimulatingPhysics())
                {
                    // Could reduce solver iterations here
                }
                break;
                
            case EPhysicsLODState::SimplifiedPhysics:
                // Use simplified collision
                if (Settings.bDisableComplexCollision)
                {
                    Primitive->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
                    Primitive->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Ignore);
                }
                break;
                
            case EPhysicsLODState::StaticOnly:
                // Disable physics simulation
                Primitive->SetSimulatePhysics(false);
                break;
                
            case EPhysicsLODState::Disabled:
                // Disable all collision
                Primitive->SetCollisionEnabled(ECollisionEnabled::NoCollision);
                break;
        }
    }
}

void UPhysicsOptimizationComponent::UpdateCollisionSettings(UPrimitiveComponent* Component, const FPhysicsLODSettings& Settings)
{
    if (!Component) return;
    
    // Apply collision simplification
    if (Settings.CollisionSimplificationFactor < 1.0f)
    {
        // This would require more complex collision mesh manipulation
        // For now, we'll just adjust collision response
        if (Settings.CollisionSimplificationFactor < 0.5f)
        {
            Component->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
        }
    }
    
    // Disable complex collision if requested
    if (Settings.bDisableComplexCollision)
    {
        Component->SetCollisionObjectType(ECC_WorldStatic);
    }
}

bool UPhysicsOptimizationComponent::IsPerformanceTargetMet() const
{
    float CurrentFrameTime = GetWorld()->GetDeltaSeconds() * 1000.0f; // Convert to ms
    return CurrentFrameTime <= TargetFrameTime && CurrentMetrics.PhysicsUpdateTime <= MaxPhysicsTime;
}

void UPhysicsOptimizationComponent::BeginLODTransition(EPhysicsLODState FromState, EPhysicsLODState ToState)
{
    bLODTransitionInProgress = true;
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("PhysicsOptimization: Beginning LOD transition from %d to %d"), 
           (int32)FromState, (int32)ToState);
}

void UPhysicsOptimizationComponent::CompleteLODTransition()
{
    bLODTransitionInProgress = false;
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("PhysicsOptimization: Completed LOD transition to %d"), 
           (int32)CurrentLODState);
}

bool UPhysicsOptimizationComponent::IsLODTransitionValid(EPhysicsLODState FromState, EPhysicsLODState ToState) const
{
    // All transitions are valid for now
    return true;
}

void UPhysicsOptimizationComponent::LogOptimizationMetrics() const
{
    UE_LOG(LogTemp, Log, TEXT("PhysicsOptimization Metrics - Active: %d, Sleeping: %d, Optimized: %d, PhysicsTime: %.2fms"), 
           CurrentMetrics.ActivePhysicsBodies, CurrentMetrics.SleepingBodies, 
           CurrentMetrics.OptimizedBodies, CurrentMetrics.PhysicsUpdateTime);
}

void UPhysicsOptimizationComponent::ResetMetrics()
{
    CurrentMetrics = FPhysicsOptimizationMetrics();
    AccumulatedPhysicsTime = 0.0f;
    AccumulatedFrameTime = 0.0f;
    FrameCount = 0;
}

// Blueprint callable functions
void UPhysicsOptimizationComponent::SetOptimizationLevel(EPhysicsOptimizationLevel NewLevel)
{
    OptimizationLevel = NewLevel;
    
    // Adjust settings based on optimization level
    switch (NewLevel)
    {
        case EPhysicsOptimizationLevel::None:
            bOptimizationEnabled = false;
            break;
            
        case EPhysicsOptimizationLevel::Low:
            bOptimizationEnabled = true;
            LODUpdateFrequency = 1.0f;
            TargetFrameTime = 20.0f; // 50 FPS
            break;
            
        case EPhysicsOptimizationLevel::Medium:
            bOptimizationEnabled = true;
            LODUpdateFrequency = 0.5f;
            TargetFrameTime = 16.67f; // 60 FPS
            break;
            
        case EPhysicsOptimizationLevel::High:
            bOptimizationEnabled = true;
            LODUpdateFrequency = 0.25f;
            TargetFrameTime = 16.67f; // 60 FPS
            break;
            
        case EPhysicsOptimizationLevel::Aggressive:
            bOptimizationEnabled = true;
            LODUpdateFrequency = 0.1f;
            TargetFrameTime = 13.33f; // 75 FPS
            break;
    }
    
    UE_LOG(LogTemp, Log, TEXT("PhysicsOptimization: Set optimization level to %d"), (int32)NewLevel);
}

void UPhysicsOptimizationComponent::ForceUpdateLOD()
{
    LastLODUpdateTime = 0.0f; // Force next update
    UE_LOG(LogTemp, Log, TEXT("PhysicsOptimization: Forced LOD update"));
}

void UPhysicsOptimizationComponent::OptimizePhysicsForTarget(float TargetFPS)
{
    TargetFrameTime = 1000.0f / TargetFPS; // Convert FPS to ms
    MaxPhysicsTime = TargetFrameTime * 0.5f; // Physics should use max 50% of frame time
    
    UE_LOG(LogTemp, Log, TEXT("PhysicsOptimization: Set target FPS to %.1f (%.2fms frame time)"), 
           TargetFPS, TargetFrameTime);
}

EPhysicsLODState UPhysicsOptimizationComponent::GetCurrentLODState() const
{
    return CurrentLODState;
}

FPhysicsOptimizationMetrics UPhysicsOptimizationComponent::GetOptimizationMetrics() const
{
    return CurrentMetrics;
}

void UPhysicsOptimizationComponent::EnableOptimization(bool bEnable)
{
    bOptimizationEnabled = bEnable;
    UE_LOG(LogTemp, Log, TEXT("PhysicsOptimization: Optimization %s"), 
           bEnable ? TEXT("ENABLED") : TEXT("DISABLED"));
}

void UPhysicsOptimizationComponent::SetLODDistanceOverride(float Distance, EPhysicsLODState LODState)
{
    // Find and update the appropriate LOD level
    for (FPhysicsLODSettings& Settings : LODLevels)
    {
        if (Settings.LODState == LODState)
        {
            Settings.DistanceThreshold = Distance;
            UE_LOG(LogTemp, Log, TEXT("PhysicsOptimization: Set LOD %d distance to %.1f"), 
                   (int32)LODState, Distance);
            break;
        }
    }
}