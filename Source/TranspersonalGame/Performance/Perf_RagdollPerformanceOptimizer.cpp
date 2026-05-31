#include "Perf_RagdollPerformanceOptimizer.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SkeletalMeshComponent.h"
#include "PhysicsEngine/BodyInstance.h"
#include "Engine/Engine.h"

UPerf_RagdollPerformanceOptimizer::UPerf_RagdollPerformanceOptimizer()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update every 100ms for performance
    
    CurrentFrameTime = 0.0f;
    TotalActiveRagdolls = 0;
    PhysicsSimulationTime = 0.0f;
    LastPerformanceCheck = 0.0f;
    AccumulatedFrameTime = 0.0f;
    FrameCount = 0;
    bEmergencyMode = false;
    EmergencyModeStartTime = 0.0f;
}

void UPerf_RagdollPerformanceOptimizer::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize performance tracking
    LastPerformanceCheck = GetWorld()->GetTimeSeconds();
    
    UE_LOG(LogTemp, Log, TEXT("RagdollPerformanceOptimizer: Initialized with max %d ragdolls"), 
           PerformanceSettings.MaxActiveRagdolls);
}

void UPerf_RagdollPerformanceOptimizer::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update performance metrics
    CurrentFrameTime = DeltaTime;
    AccumulatedFrameTime += DeltaTime;
    FrameCount++;
    
    // Update performance budget every frame
    UpdatePerformanceBudget(DeltaTime);
    
    // Cull expired ragdolls
    CullExpiredRagdolls();
    
    // Update LOD for all active ragdolls
    float CurrentTime = GetWorld()->GetTimeSeconds();
    for (FPerf_RagdollInstanceData& RagdollData : ActiveRagdolls)
    {
        if (RagdollData.RagdollComponent.IsValid())
        {
            UpdateRagdollLOD(RagdollData, DeltaTime);
        }
    }
    
    // Performance analysis every second
    if (CurrentTime - LastPerformanceCheck >= 1.0f)
    {
        CalculatePhysicsPerformance();
        LastPerformanceCheck = CurrentTime;
    }
    
    // Emergency performance check
    float AverageFrameTime = FrameCount > 0 ? AccumulatedFrameTime / FrameCount : 0.0f;
    if (AverageFrameTime > 0.0167f) // Above 60fps threshold
    {
        if (!bEmergencyMode)
        {
            bEmergencyMode = true;
            EmergencyModeStartTime = CurrentTime;
            EmergencyPerformanceCleanup();
        }
    }
    else if (bEmergencyMode && CurrentTime - EmergencyModeStartTime > 5.0f)
    {
        bEmergencyMode = false;
        UE_LOG(LogTemp, Log, TEXT("RagdollPerformanceOptimizer: Exiting emergency mode"));
    }
}

void UPerf_RagdollPerformanceOptimizer::RegisterRagdoll(USkeletalMeshComponent* RagdollComponent)
{
    if (!RagdollComponent || !RagdollComponent->IsValidLowLevel())
    {
        return;
    }
    
    // Check if we can create new ragdoll
    if (!ShouldCreateNewRagdoll())
    {
        UE_LOG(LogTemp, Warning, TEXT("RagdollPerformanceOptimizer: Cannot create new ragdoll - budget exceeded"));
        return;
    }
    
    // Create new ragdoll instance data
    FPerf_RagdollInstanceData NewRagdoll;
    NewRagdoll.RagdollComponent = RagdollComponent;
    NewRagdoll.CreationTime = GetWorld()->GetTimeSeconds();
    NewRagdoll.LastUpdateTime = NewRagdoll.CreationTime;
    NewRagdoll.ActiveBoneCount = RagdollComponent->GetNumBones();
    NewRagdoll.CurrentLOD = EPerf_OptimizationLevel::High;
    
    ActiveRagdolls.Add(NewRagdoll);
    TotalActiveRagdolls = ActiveRagdolls.Num();
    
    UE_LOG(LogTemp, Log, TEXT("RagdollPerformanceOptimizer: Registered ragdoll %s (%d total)"), 
           *RagdollComponent->GetName(), TotalActiveRagdolls);
}

void UPerf_RagdollPerformanceOptimizer::UnregisterRagdoll(USkeletalMeshComponent* RagdollComponent)
{
    if (!RagdollComponent)
    {
        return;
    }
    
    // Find and remove ragdoll from tracking
    for (int32 i = ActiveRagdolls.Num() - 1; i >= 0; i--)
    {
        if (ActiveRagdolls[i].RagdollComponent == RagdollComponent)
        {
            ActiveRagdolls.RemoveAt(i);
            break;
        }
    }
    
    TotalActiveRagdolls = ActiveRagdolls.Num();
    
    UE_LOG(LogTemp, Log, TEXT("RagdollPerformanceOptimizer: Unregistered ragdoll %s (%d remaining)"), 
           *RagdollComponent->GetName(), TotalActiveRagdolls);
}

void UPerf_RagdollPerformanceOptimizer::OptimizeRagdollLOD(USkeletalMeshComponent* RagdollComponent, float DistanceToPlayer)
{
    if (!RagdollComponent || !RagdollComponent->IsValidLowLevel())
    {
        return;
    }
    
    EPerf_OptimizationLevel OptimalLOD = GetOptimalLODForDistance(DistanceToPlayer);
    
    // Find ragdoll in active list
    for (FPerf_RagdollInstanceData& RagdollData : ActiveRagdolls)
    {
        if (RagdollData.RagdollComponent == RagdollComponent)
        {
            if (RagdollData.CurrentLOD != OptimalLOD)
            {
                RagdollData.CurrentLOD = OptimalLOD;
                UpdatePhysicsSettings(RagdollComponent, OptimalLOD);
                
                UE_LOG(LogTemp, VeryVerbose, TEXT("RagdollPerformanceOptimizer: Updated LOD for %s to %d"), 
                       *RagdollComponent->GetName(), (int32)OptimalLOD);
            }
            break;
        }
    }
}

void UPerf_RagdollPerformanceOptimizer::UpdatePerformanceBudget(float DeltaTime)
{
    // Calculate current performance cost
    float CurrentCost = GetRagdollPerformanceCost();
    
    // If we're over budget, start aggressive optimization
    if (CurrentCost > 1.0f) // Over 100% budget
    {
        SetPerformanceMode(EPerf_OptimizationLevel::Low);
    }
    else if (CurrentCost > 0.8f) // Over 80% budget
    {
        SetPerformanceMode(EPerf_OptimizationLevel::Medium);
    }
    else
    {
        SetPerformanceMode(EPerf_OptimizationLevel::High);
    }
}

void UPerf_RagdollPerformanceOptimizer::CullExpiredRagdolls()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    for (int32 i = ActiveRagdolls.Num() - 1; i >= 0; i--)
    {
        FPerf_RagdollInstanceData& RagdollData = ActiveRagdolls[i];
        
        // Remove if component is invalid
        if (!RagdollData.RagdollComponent.IsValid())
        {
            ActiveRagdolls.RemoveAt(i);
            continue;
        }
        
        // Remove if expired
        float Age = CurrentTime - RagdollData.CreationTime;
        if (Age > PerformanceSettings.RagdollLifetime)
        {
            USkeletalMeshComponent* Component = RagdollData.RagdollComponent.Get();
            if (Component && Component->IsValidLowLevel())
            {
                Component->SetSimulatePhysics(false);
                UE_LOG(LogTemp, Log, TEXT("RagdollPerformanceOptimizer: Culled expired ragdoll %s"), 
                       *Component->GetName());
            }
            ActiveRagdolls.RemoveAt(i);
        }
    }
    
    TotalActiveRagdolls = ActiveRagdolls.Num();
}

float UPerf_RagdollPerformanceOptimizer::GetRagdollPerformanceCost() const
{
    if (PerformanceSettings.MaxActiveRagdolls <= 0)
    {
        return 0.0f;
    }
    
    // Base cost from number of ragdolls
    float BaseCost = (float)TotalActiveRagdolls / (float)PerformanceSettings.MaxActiveRagdolls;
    
    // Additional cost from frame time
    float FrameTimeCost = CurrentFrameTime / 0.0167f; // 60fps target
    
    return FMath::Max(BaseCost, FrameTimeCost);
}

bool UPerf_RagdollPerformanceOptimizer::ShouldCreateNewRagdoll() const
{
    // Don't create if over budget
    if (TotalActiveRagdolls >= PerformanceSettings.MaxActiveRagdolls)
    {
        return false;
    }
    
    // Don't create if performance is poor
    if (GetRagdollPerformanceCost() > 0.9f)
    {
        return false;
    }
    
    // Don't create in emergency mode
    if (bEmergencyMode)
    {
        return false;
    }
    
    return true;
}

EPerf_OptimizationLevel UPerf_RagdollPerformanceOptimizer::GetOptimalLODForDistance(float Distance) const
{
    if (Distance <= PerformanceSettings.HighDetailDistance)
    {
        return EPerf_OptimizationLevel::High;
    }
    else if (Distance <= PerformanceSettings.MediumDetailDistance)
    {
        return EPerf_OptimizationLevel::Medium;
    }
    else
    {
        return EPerf_OptimizationLevel::Low;
    }
}

void UPerf_RagdollPerformanceOptimizer::EmergencyPerformanceCleanup()
{
    UE_LOG(LogTemp, Warning, TEXT("RagdollPerformanceOptimizer: EMERGENCY PERFORMANCE CLEANUP"));
    
    // Disable half of the ragdolls, starting with furthest
    int32 TargetCount = FMath::Max(1, PerformanceSettings.MaxActiveRagdolls / 2);
    
    // Sort by distance (would need player reference for accurate sorting)
    // For now, just remove oldest ragdolls
    while (ActiveRagdolls.Num() > TargetCount)
    {
        FPerf_RagdollInstanceData& OldestRagdoll = ActiveRagdolls[0];
        if (OldestRagdoll.RagdollComponent.IsValid())
        {
            OldestRagdoll.RagdollComponent->SetSimulatePhysics(false);
        }
        ActiveRagdolls.RemoveAt(0);
    }
    
    TotalActiveRagdolls = ActiveRagdolls.Num();
    
    UE_LOG(LogTemp, Warning, TEXT("RagdollPerformanceOptimizer: Emergency cleanup complete - %d ragdolls remaining"), 
           TotalActiveRagdolls);
}

void UPerf_RagdollPerformanceOptimizer::SetPerformanceMode(EPerf_OptimizationLevel NewMode)
{
    // Apply performance mode to all active ragdolls
    for (FPerf_RagdollInstanceData& RagdollData : ActiveRagdolls)
    {
        if (RagdollData.RagdollComponent.IsValid())
        {
            UpdatePhysicsSettings(RagdollData.RagdollComponent.Get(), NewMode);
        }
    }
}

void UPerf_RagdollPerformanceOptimizer::UpdateRagdollLOD(FPerf_RagdollInstanceData& RagdollData, float DeltaTime)
{
    if (!RagdollData.RagdollComponent.IsValid())
    {
        return;
    }
    
    USkeletalMeshComponent* Component = RagdollData.RagdollComponent.Get();
    
    // Calculate distance to player (simplified - would need player reference)
    float DistanceToPlayer = 2000.0f; // Placeholder
    
    // Update LOD based on distance
    EPerf_OptimizationLevel OptimalLOD = GetOptimalLODForDistance(DistanceToPlayer);
    if (RagdollData.CurrentLOD != OptimalLOD)
    {
        RagdollData.CurrentLOD = OptimalLOD;
        UpdatePhysicsSettings(Component, OptimalLOD);
    }
    
    RagdollData.LastUpdateTime = GetWorld()->GetTimeSeconds();
}

void UPerf_RagdollPerformanceOptimizer::CalculatePhysicsPerformance()
{
    // Calculate average frame time over the last second
    if (FrameCount > 0)
    {
        float AverageFrameTime = AccumulatedFrameTime / FrameCount;
        PhysicsSimulationTime = AverageFrameTime;
        
        // Reset counters
        AccumulatedFrameTime = 0.0f;
        FrameCount = 0;
        
        UE_LOG(LogTemp, VeryVerbose, TEXT("RagdollPerformanceOptimizer: Avg frame time: %.3fms, %d ragdolls"), 
               AverageFrameTime * 1000.0f, TotalActiveRagdolls);
    }
}

void UPerf_RagdollPerformanceOptimizer::OptimizeRagdollBones(USkeletalMeshComponent* RagdollComponent, int32 TargetBoneCount)
{
    if (!RagdollComponent || !RagdollComponent->IsValidLowLevel())
    {
        return;
    }
    
    // This would require more complex bone hierarchy analysis
    // For now, just log the optimization attempt
    UE_LOG(LogTemp, VeryVerbose, TEXT("RagdollPerformanceOptimizer: Optimizing bone count for %s to %d bones"), 
           *RagdollComponent->GetName(), TargetBoneCount);
}

void UPerf_RagdollPerformanceOptimizer::UpdatePhysicsSettings(USkeletalMeshComponent* RagdollComponent, EPerf_OptimizationLevel LOD)
{
    if (!RagdollComponent || !RagdollComponent->IsValidLowLevel())
    {
        return;
    }
    
    // Adjust physics settings based on LOD
    switch (LOD)
    {
        case EPerf_OptimizationLevel::High:
            // Full detail physics
            RagdollComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            break;
            
        case EPerf_OptimizationLevel::Medium:
            // Reduced detail physics
            RagdollComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            break;
            
        case EPerf_OptimizationLevel::Low:
            // Minimal physics
            RagdollComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
            break;
            
        case EPerf_OptimizationLevel::Disabled:
            // No physics
            RagdollComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            RagdollComponent->SetSimulatePhysics(false);
            break;
    }
}