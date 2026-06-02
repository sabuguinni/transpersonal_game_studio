#include "Core_PhysicsOptimizer.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "GameFramework/Pawn.h"
#include "PhysicsEngine/BodyInstance.h"
#include "Components/PrimitiveComponent.h"

DEFINE_LOG_CATEGORY_STATIC(LogPhysicsOptimizer, Log, All);

UCore_PhysicsOptimizer::UCore_PhysicsOptimizer()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Optimize every 100ms
    
    // Default optimization settings
    OptimizationSettings.MaxSimulationDistance = 5000.0f;
    OptimizationSettings.CullingDistance = 10000.0f;
    OptimizationSettings.MaxActiveRigidBodies = 500;
    OptimizationSettings.MaxActiveConstraints = 200;
    OptimizationSettings.bEnableLODOptimization = true;
    OptimizationSettings.bEnableDistanceCulling = true;
    OptimizationSettings.bEnableComplexityReduction = true;
    
    // Performance thresholds
    PerformanceThresholds.TargetFrameTime = 16.67f; // 60 FPS
    PerformanceThresholds.MaxPhysicsTime = 5.0f;
    PerformanceThresholds.CriticalFrameTime = 33.33f; // 30 FPS
    
    // Initialize counters
    ActiveRigidBodies = 0;
    ActiveConstraints = 0;
    OptimizedObjects = 0;
    LastOptimizationTime = 0.0f;
}

void UCore_PhysicsOptimizer::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogPhysicsOptimizer, Log, TEXT("Physics Optimizer initialized"));
    
    // Cache world reference
    CachedWorld = GetWorld();
    if (!CachedWorld)
    {
        UE_LOG(LogPhysicsOptimizer, Error, TEXT("Failed to get world reference"));
        return;
    }
    
    // Initialize optimization system
    InitializeOptimization();
}

void UCore_PhysicsOptimizer::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!CachedWorld)
    {
        return;
    }
    
    // Update performance metrics
    UpdatePerformanceMetrics(DeltaTime);
    
    // Run optimization pass
    RunOptimizationPass();
    
    // Update statistics
    UpdateOptimizationStats();
}

void UCore_PhysicsOptimizer::InitializeOptimization()
{
    if (!CachedWorld)
    {
        return;
    }
    
    UE_LOG(LogPhysicsOptimizer, Log, TEXT("Initializing physics optimization system"));
    
    // Scan for physics objects
    ScanPhysicsObjects();
    
    // Setup optimization groups
    SetupOptimizationGroups();
    
    UE_LOG(LogPhysicsOptimizer, Log, TEXT("Physics optimization initialized with %d objects"), PhysicsObjects.Num());
}

void UCore_PhysicsOptimizer::ScanPhysicsObjects()
{
    PhysicsObjects.Empty();
    
    if (!CachedWorld)
    {
        return;
    }
    
    // Iterate through all actors with physics
    for (TActorIterator<AActor> ActorItr(CachedWorld); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor || Actor->IsPendingKill())
        {
            continue;
        }
        
        // Check for physics components
        TArray<UPrimitiveComponent*> PrimitiveComponents;
        Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
        
        for (UPrimitiveComponent* Component : PrimitiveComponents)
        {
            if (Component && Component->IsSimulatingPhysics())
            {
                FCore_PhysicsObjectInfo ObjectInfo;
                ObjectInfo.Actor = Actor;
                ObjectInfo.Component = Component;
                ObjectInfo.OriginalComplexity = GetPhysicsComplexity(Component);
                ObjectInfo.CurrentComplexity = ObjectInfo.OriginalComplexity;
                ObjectInfo.LastOptimizationTime = 0.0f;
                ObjectInfo.bIsOptimized = false;
                
                PhysicsObjects.Add(ObjectInfo);
                break; // One entry per actor
            }
        }
    }
    
    UE_LOG(LogPhysicsOptimizer, Log, TEXT("Scanned %d physics objects"), PhysicsObjects.Num());
}

void UCore_PhysicsOptimizer::SetupOptimizationGroups()
{
    OptimizationGroups.Empty();
    
    // Group objects by type and complexity
    for (const FCore_PhysicsObjectInfo& ObjectInfo : PhysicsObjects)
    {
        if (!ObjectInfo.Actor.IsValid())
        {
            continue;
        }
        
        FString GroupName = GetOptimizationGroupName(ObjectInfo.Actor.Get());
        
        FCore_OptimizationGroup* Group = OptimizationGroups.Find(GroupName);
        if (!Group)
        {
            FCore_OptimizationGroup NewGroup;
            NewGroup.GroupName = GroupName;
            NewGroup.MaxObjects = GetMaxObjectsForGroup(GroupName);
            NewGroup.OptimizationPriority = GetGroupPriority(GroupName);
            OptimizationGroups.Add(GroupName, NewGroup);
            Group = &OptimizationGroups[GroupName];
        }
        
        Group->Objects.Add(ObjectInfo.Actor.Get());
    }
    
    UE_LOG(LogPhysicsOptimizer, Log, TEXT("Created %d optimization groups"), OptimizationGroups.Num());
}

void UCore_PhysicsOptimizer::RunOptimizationPass()
{
    if (!CachedWorld)
    {
        return;
    }
    
    float CurrentTime = CachedWorld->GetTimeSeconds();
    
    // Skip if optimization ran recently
    if (CurrentTime - LastOptimizationTime < 0.1f)
    {
        return;
    }
    
    LastOptimizationTime = CurrentTime;
    
    // Get player location for distance calculations
    FVector PlayerLocation = GetPlayerLocation();
    
    // Optimize based on distance and performance
    OptimizeByDistance(PlayerLocation);
    OptimizeByPerformance();
    OptimizeByComplexity();
    
    // Update active counts
    UpdateActiveCounts();
}

void UCore_PhysicsOptimizer::OptimizeByDistance(const FVector& PlayerLocation)
{
    if (!OptimizationSettings.bEnableDistanceCulling)
    {
        return;
    }
    
    for (FCore_PhysicsObjectInfo& ObjectInfo : PhysicsObjects)
    {
        if (!ObjectInfo.Actor.IsValid() || !ObjectInfo.Component.IsValid())
        {
            continue;
        }
        
        float Distance = FVector::Dist(PlayerLocation, ObjectInfo.Actor->GetActorLocation());
        
        // Disable physics for distant objects
        if (Distance > OptimizationSettings.CullingDistance)
        {
            if (ObjectInfo.Component->IsSimulatingPhysics())
            {
                ObjectInfo.Component->SetSimulatePhysics(false);
                ObjectInfo.bIsOptimized = true;
                OptimizedObjects++;
            }
        }
        // Re-enable physics for nearby objects
        else if (Distance < OptimizationSettings.MaxSimulationDistance)
        {
            if (!ObjectInfo.Component->IsSimulatingPhysics() && ObjectInfo.bIsOptimized)
            {
                ObjectInfo.Component->SetSimulatePhysics(true);
                ObjectInfo.bIsOptimized = false;
            }
        }
    }
}

void UCore_PhysicsOptimizer::OptimizeByPerformance()
{
    // Check if we're hitting performance targets
    if (CurrentFrameTime > PerformanceThresholds.CriticalFrameTime)
    {
        // Aggressive optimization needed
        ReducePhysicsComplexity(0.5f);
    }
    else if (CurrentFrameTime > PerformanceThresholds.TargetFrameTime)
    {
        // Moderate optimization
        ReducePhysicsComplexity(0.8f);
    }
    else
    {
        // Performance is good, restore complexity if possible
        RestorePhysicsComplexity();
    }
}

void UCore_PhysicsOptimizer::OptimizeByComplexity()
{
    if (!OptimizationSettings.bEnableComplexityReduction)
    {
        return;
    }
    
    // Limit active rigid bodies
    if (ActiveRigidBodies > OptimizationSettings.MaxActiveRigidBodies)
    {
        DisableExcessRigidBodies();
    }
    
    // Limit active constraints
    if (ActiveConstraints > OptimizationSettings.MaxActiveConstraints)
    {
        DisableExcessConstraints();
    }
}

void UCore_PhysicsOptimizer::ReducePhysicsComplexity(float ReductionFactor)
{
    FVector PlayerLocation = GetPlayerLocation();
    
    // Sort objects by distance from player
    PhysicsObjects.Sort([PlayerLocation](const FCore_PhysicsObjectInfo& A, const FCore_PhysicsObjectInfo& B)
    {
        if (!A.Actor.IsValid() || !B.Actor.IsValid())
        {
            return false;
        }
        
        float DistA = FVector::Dist(PlayerLocation, A.Actor->GetActorLocation());
        float DistB = FVector::Dist(PlayerLocation, B.Actor->GetActorLocation());
        return DistA > DistB; // Furthest first
    });
    
    // Reduce complexity for distant objects
    int32 ObjectsToOptimize = FMath::FloorToInt(PhysicsObjects.Num() * (1.0f - ReductionFactor));
    
    for (int32 i = 0; i < ObjectsToOptimize && i < PhysicsObjects.Num(); i++)
    {
        FCore_PhysicsObjectInfo& ObjectInfo = PhysicsObjects[i];
        
        if (ObjectInfo.Component.IsValid() && !ObjectInfo.bIsOptimized)
        {
            // Reduce collision complexity
            ReduceCollisionComplexity(ObjectInfo.Component.Get());
            ObjectInfo.bIsOptimized = true;
            OptimizedObjects++;
        }
    }
}

void UCore_PhysicsOptimizer::RestorePhysicsComplexity()
{
    for (FCore_PhysicsObjectInfo& ObjectInfo : PhysicsObjects)
    {
        if (ObjectInfo.Component.IsValid() && ObjectInfo.bIsOptimized)
        {
            // Restore original complexity
            RestoreCollisionComplexity(ObjectInfo.Component.Get(), ObjectInfo.OriginalComplexity);
            ObjectInfo.bIsOptimized = false;
        }
    }
}

void UCore_PhysicsOptimizer::UpdatePerformanceMetrics(float DeltaTime)
{
    CurrentFrameTime = DeltaTime * 1000.0f; // Convert to milliseconds
    
    // Track physics time (simplified)
    CurrentPhysicsTime = FMath::Min(CurrentFrameTime * 0.3f, PerformanceThresholds.MaxPhysicsTime);
    
    // Update averages
    static float FrameTimeAccumulator = 0.0f;
    static int32 FrameCount = 0;
    
    FrameTimeAccumulator += CurrentFrameTime;
    FrameCount++;
    
    if (FrameCount >= 60) // Update average every 60 frames
    {
        AverageFrameTime = FrameTimeAccumulator / FrameCount;
        FrameTimeAccumulator = 0.0f;
        FrameCount = 0;
    }
}

void UCore_PhysicsOptimizer::UpdateActiveCounts()
{
    ActiveRigidBodies = 0;
    ActiveConstraints = 0;
    
    for (const FCore_PhysicsObjectInfo& ObjectInfo : PhysicsObjects)
    {
        if (ObjectInfo.Component.IsValid() && ObjectInfo.Component->IsSimulatingPhysics())
        {
            ActiveRigidBodies++;
        }
    }
    
    // Count constraints (simplified)
    ActiveConstraints = ActiveRigidBodies / 4; // Rough estimate
}

void UCore_PhysicsOptimizer::UpdateOptimizationStats()
{
    // Reset counters periodically
    static float LastStatsReset = 0.0f;
    float CurrentTime = CachedWorld ? CachedWorld->GetTimeSeconds() : 0.0f;
    
    if (CurrentTime - LastStatsReset > 5.0f) // Reset every 5 seconds
    {
        OptimizedObjects = 0;
        LastStatsReset = CurrentTime;
    }
}

FVector UCore_PhysicsOptimizer::GetPlayerLocation() const
{
    if (!CachedWorld)
    {
        return FVector::ZeroVector;
    }
    
    APawn* PlayerPawn = CachedWorld->GetFirstPlayerController() ? 
        CachedWorld->GetFirstPlayerController()->GetPawn() : nullptr;
    
    return PlayerPawn ? PlayerPawn->GetActorLocation() : FVector::ZeroVector;
}

ECore_PhysicsComplexity UCore_PhysicsOptimizer::GetPhysicsComplexity(UPrimitiveComponent* Component) const
{
    if (!Component)
    {
        return ECore_PhysicsComplexity::Simple;
    }
    
    // Determine complexity based on collision settings
    if (Component->GetCollisionResponseToChannel(ECC_Pawn) == ECR_Block)
    {
        return ECore_PhysicsComplexity::Complex;
    }
    else if (Component->GetCollisionEnabled() == ECollisionEnabled::QueryAndPhysics)
    {
        return ECore_PhysicsComplexity::Medium;
    }
    
    return ECore_PhysicsComplexity::Simple;
}

FString UCore_PhysicsOptimizer::GetOptimizationGroupName(AActor* Actor) const
{
    if (!Actor)
    {
        return TEXT("Unknown");
    }
    
    // Classify by actor type
    if (Actor->IsA<APawn>())
    {
        return TEXT("Characters");
    }
    else if (Actor->IsA<AStaticMeshActor>())
    {
        return TEXT("StaticMeshes");
    }
    
    return TEXT("Other");
}

int32 UCore_PhysicsOptimizer::GetMaxObjectsForGroup(const FString& GroupName) const
{
    if (GroupName == TEXT("Characters"))
    {
        return 50;
    }
    else if (GroupName == TEXT("StaticMeshes"))
    {
        return 200;
    }
    
    return 100;
}

float UCore_PhysicsOptimizer::GetGroupPriority(const FString& GroupName) const
{
    if (GroupName == TEXT("Characters"))
    {
        return 1.0f; // Highest priority
    }
    else if (GroupName == TEXT("StaticMeshes"))
    {
        return 0.5f;
    }
    
    return 0.3f;
}

void UCore_PhysicsOptimizer::DisableExcessRigidBodies()
{
    // Implementation for disabling excess rigid bodies
    // This would involve more complex logic to selectively disable physics
    UE_LOG(LogPhysicsOptimizer, Warning, TEXT("Too many active rigid bodies (%d), optimization needed"), ActiveRigidBodies);
}

void UCore_PhysicsOptimizer::DisableExcessConstraints()
{
    // Implementation for disabling excess constraints
    UE_LOG(LogPhysicsOptimizer, Warning, TEXT("Too many active constraints (%d), optimization needed"), ActiveConstraints);
}

void UCore_PhysicsOptimizer::ReduceCollisionComplexity(UPrimitiveComponent* Component)
{
    if (!Component)
    {
        return;
    }
    
    // Simplify collision by reducing precision
    Component->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void UCore_PhysicsOptimizer::RestoreCollisionComplexity(UPrimitiveComponent* Component, ECore_PhysicsComplexity OriginalComplexity)
{
    if (!Component)
    {
        return;
    }
    
    // Restore original collision settings
    Component->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
}