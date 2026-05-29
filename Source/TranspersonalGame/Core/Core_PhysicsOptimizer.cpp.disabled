#include "Core_PhysicsOptimizer.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/Actor.h"

DEFINE_LOG_CATEGORY_STATIC(LogPhysicsOptimizer, Log, All);

UCore_PhysicsOptimizer::UCore_PhysicsOptimizer()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Check every 100ms
    
    // Initialize optimization parameters
    MaxPhysicsActors = 500;
    OptimizationRadius = 5000.0f;
    LODDistanceNear = 1000.0f;
    LODDistanceMedium = 3000.0f;
    LODDistanceFar = 5000.0f;
    
    bEnableDistanceCulling = true;
    bEnableLODOptimization = true;
    bEnableCollisionOptimization = true;
    
    PhysicsActorCount = 0;
    OptimizedActorCount = 0;
    CulledActorCount = 0;
    
    LastOptimizationTime = 0.0f;
    OptimizationInterval = 1.0f; // Optimize every second
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
    InitializeOptimizationSystem();
}

void UCore_PhysicsOptimizer::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!CachedWorld)
        return;
    
    float CurrentTime = CachedWorld->GetTimeSeconds();
    
    // Run optimization at specified intervals
    if (CurrentTime - LastOptimizationTime >= OptimizationInterval)
    {
        PerformPhysicsOptimization();
        LastOptimizationTime = CurrentTime;
    }
}

void UCore_PhysicsOptimizer::InitializeOptimizationSystem()
{
    if (!CachedWorld)
        return;
    
    UE_LOG(LogPhysicsOptimizer, Log, TEXT("Initializing physics optimization system"));
    
    // Scan for physics actors in the world
    ScanPhysicsActors();
    
    // Set up optimization groups
    SetupOptimizationGroups();
    
    UE_LOG(LogPhysicsOptimizer, Log, TEXT("Physics optimization system initialized with %d actors"), PhysicsActorCount);
}

void UCore_PhysicsOptimizer::ScanPhysicsActors()
{
    if (!CachedWorld)
        return;
    
    PhysicsActors.Empty();
    PhysicsActorCount = 0;
    
    // Iterate through all actors in the world
    for (TActorIterator<AActor> ActorItr(CachedWorld); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor || Actor->IsPendingKill())
            continue;
        
        // Check if actor has physics components
        if (HasPhysicsComponents(Actor))
        {
            FPhysicsActorInfo ActorInfo;
            ActorInfo.Actor = Actor;
            ActorInfo.OriginalCollisionEnabled = true;
            ActorInfo.CurrentLODLevel = EEng_PhysicsLOD::High;
            ActorInfo.LastOptimizationTime = 0.0f;
            
            PhysicsActors.Add(ActorInfo);
            PhysicsActorCount++;
        }
    }
    
    UE_LOG(LogPhysicsOptimizer, Log, TEXT("Found %d physics actors"), PhysicsActorCount);
}

bool UCore_PhysicsOptimizer::HasPhysicsComponents(AActor* Actor)
{
    if (!Actor)
        return false;
    
    // Check for static mesh components with collision
    TArray<UStaticMeshComponent*> MeshComponents;
    Actor->GetComponents<UStaticMeshComponent>(MeshComponents);
    
    for (UStaticMeshComponent* MeshComp : MeshComponents)
    {
        if (MeshComp && MeshComp->GetCollisionEnabled() != ECollisionEnabled::NoCollision)
        {
            return true;
        }
    }
    
    // Check for other primitive components
    TArray<UPrimitiveComponent*> PrimitiveComponents;
    Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
    
    for (UPrimitiveComponent* PrimComp : PrimitiveComponents)
    {
        if (PrimComp && PrimComp->GetCollisionEnabled() != ECollisionEnabled::NoCollision)
        {
            return true;
        }
    }
    
    return false;
}

void UCore_PhysicsOptimizer::SetupOptimizationGroups()
{
    // Clear existing groups
    NearActors.Empty();
    MediumActors.Empty();
    FarActors.Empty();
    CulledActors.Empty();
    
    UE_LOG(LogPhysicsOptimizer, Log, TEXT("Optimization groups set up"));
}

void UCore_PhysicsOptimizer::PerformPhysicsOptimization()
{
    if (!CachedWorld)
        return;
    
    // Get player location for distance calculations
    FVector PlayerLocation = GetPlayerLocation();
    
    // Reset counters
    OptimizedActorCount = 0;
    CulledActorCount = 0;
    
    // Clear optimization groups
    NearActors.Empty();
    MediumActors.Empty();
    FarActors.Empty();
    CulledActors.Empty();
    
    // Optimize each physics actor
    for (FPhysicsActorInfo& ActorInfo : PhysicsActors)
    {
        if (!ActorInfo.Actor || ActorInfo.Actor->IsPendingKill())
            continue;
        
        OptimizeActor(ActorInfo, PlayerLocation);
    }
    
    UE_LOG(LogPhysicsOptimizer, VeryVerbose, TEXT("Physics optimization complete: %d optimized, %d culled"), 
           OptimizedActorCount, CulledActorCount);
}

FVector UCore_PhysicsOptimizer::GetPlayerLocation()
{
    if (!CachedWorld)
        return FVector::ZeroVector;
    
    // Try to get player pawn location
    APawn* PlayerPawn = CachedWorld->GetFirstPlayerController() ? 
                       CachedWorld->GetFirstPlayerController()->GetPawn() : nullptr;
    
    if (PlayerPawn)
    {
        return PlayerPawn->GetActorLocation();
    }
    
    // Fallback to world origin
    return FVector::ZeroVector;
}

void UCore_PhysicsOptimizer::OptimizeActor(FPhysicsActorInfo& ActorInfo, const FVector& PlayerLocation)
{
    if (!ActorInfo.Actor)
        return;
    
    FVector ActorLocation = ActorInfo.Actor->GetActorLocation();
    float Distance = FVector::Dist(PlayerLocation, ActorLocation);
    
    // Determine LOD level based on distance
    EEng_PhysicsLOD NewLODLevel = DetermineLODLevel(Distance);
    
    // Apply optimization if LOD level changed
    if (NewLODLevel != ActorInfo.CurrentLODLevel)
    {
        ApplyLODOptimization(ActorInfo, NewLODLevel);
        ActorInfo.CurrentLODLevel = NewLODLevel;
        OptimizedActorCount++;
    }
    
    // Add to appropriate group
    switch (NewLODLevel)
    {
        case EEng_PhysicsLOD::High:
            NearActors.Add(ActorInfo.Actor);
            break;
        case EEng_PhysicsLOD::Medium:
            MediumActors.Add(ActorInfo.Actor);
            break;
        case EEng_PhysicsLOD::Low:
            FarActors.Add(ActorInfo.Actor);
            break;
        case EEng_PhysicsLOD::Disabled:
            CulledActors.Add(ActorInfo.Actor);
            CulledActorCount++;
            break;
    }
}

EEng_PhysicsLOD UCore_PhysicsOptimizer::DetermineLODLevel(float Distance)
{
    if (!bEnableDistanceCulling)
        return EEng_PhysicsLOD::High;
    
    if (Distance <= LODDistanceNear)
        return EEng_PhysicsLOD::High;
    else if (Distance <= LODDistanceMedium)
        return EEng_PhysicsLOD::Medium;
    else if (Distance <= LODDistanceFar)
        return EEng_PhysicsLOD::Low;
    else
        return EEng_PhysicsLOD::Disabled;
}

void UCore_PhysicsOptimizer::ApplyLODOptimization(FPhysicsActorInfo& ActorInfo, EEng_PhysicsLOD LODLevel)
{
    if (!ActorInfo.Actor)
        return;
    
    // Get all primitive components
    TArray<UPrimitiveComponent*> PrimitiveComponents;
    ActorInfo.Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
    
    for (UPrimitiveComponent* PrimComp : PrimitiveComponents)
    {
        if (!PrimComp)
            continue;
        
        switch (LODLevel)
        {
            case EEng_PhysicsLOD::High:
                // Full physics and collision
                if (bEnableCollisionOptimization)
                {
                    PrimComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
                    PrimComp->SetSimulatePhysics(true);
                }
                break;
                
            case EEng_PhysicsLOD::Medium:
                // Query only, no physics simulation
                if (bEnableCollisionOptimization)
                {
                    PrimComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
                    PrimComp->SetSimulatePhysics(false);
                }
                break;
                
            case EEng_PhysicsLOD::Low:
                // Minimal collision
                if (bEnableCollisionOptimization)
                {
                    PrimComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
                    PrimComp->SetSimulatePhysics(false);
                }
                break;
                
            case EEng_PhysicsLOD::Disabled:
                // No collision or physics
                if (bEnableCollisionOptimization)
                {
                    PrimComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
                    PrimComp->SetSimulatePhysics(false);
                }
                break;
        }
    }
}

void UCore_PhysicsOptimizer::GetOptimizationStats(FEng_PhysicsOptimizationStats& OutStats)
{
    OutStats.TotalPhysicsActors = PhysicsActorCount;
    OutStats.OptimizedActors = OptimizedActorCount;
    OutStats.CulledActors = CulledActorCount;
    OutStats.NearActors = NearActors.Num();
    OutStats.MediumActors = MediumActors.Num();
    OutStats.FarActors = FarActors.Num();
    OutStats.OptimizationRadius = OptimizationRadius;
    OutStats.bOptimizationEnabled = bEnableLODOptimization;
}

void UCore_PhysicsOptimizer::SetOptimizationParameters(float InOptimizationRadius, float InNearDistance, 
                                                      float InMediumDistance, float InFarDistance)
{
    OptimizationRadius = InOptimizationRadius;
    LODDistanceNear = InNearDistance;
    LODDistanceMedium = InMediumDistance;
    LODDistanceFar = InFarDistance;
    
    UE_LOG(LogPhysicsOptimizer, Log, TEXT("Optimization parameters updated: Radius=%.1f, Near=%.1f, Medium=%.1f, Far=%.1f"), 
           OptimizationRadius, LODDistanceNear, LODDistanceMedium, LODDistanceFar);
}

void UCore_PhysicsOptimizer::EnableOptimization(bool bEnable)
{
    bEnableLODOptimization = bEnable;
    bEnableDistanceCulling = bEnable;
    bEnableCollisionOptimization = bEnable;
    
    UE_LOG(LogPhysicsOptimizer, Log, TEXT("Physics optimization %s"), bEnable ? TEXT("enabled") : TEXT("disabled"));
    
    if (!bEnable)
    {
        // Restore all actors to high LOD
        RestoreAllActors();
    }
}

void UCore_PhysicsOptimizer::RestoreAllActors()
{
    for (FPhysicsActorInfo& ActorInfo : PhysicsActors)
    {
        if (ActorInfo.Actor && !ActorInfo.Actor->IsPendingKill())
        {
            ApplyLODOptimization(ActorInfo, EEng_PhysicsLOD::High);
            ActorInfo.CurrentLODLevel = EEng_PhysicsLOD::High;
        }
    }
    
    UE_LOG(LogPhysicsOptimizer, Log, TEXT("All physics actors restored to high LOD"));
}