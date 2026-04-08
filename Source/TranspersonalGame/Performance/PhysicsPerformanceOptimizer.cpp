#include "PhysicsPerformanceOptimizer.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Components/PrimitiveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "PhysicsEngine/BodyInstance.h"
#include "PhysicsEngine/DestructibleComponent.h"
#include "Chaos/ChaosGameplayEventDispatcher.h"
#include "HAL/PlatformFilemanager.h"
#include "Stats/Stats.h"
#include "ProfilingDebugging/CpuProfilerTrace.h"

DECLARE_STATS_GROUP(TEXT("Physics Performance"), STATGROUP_PhysicsPerformance, STATCAT_Advanced);
DECLARE_CYCLE_STAT(TEXT("Physics Optimization"), STAT_PhysicsOptimization, STATGROUP_PhysicsPerformance);
DECLARE_CYCLE_STAT(TEXT("Physics LOD Update"), STAT_PhysicsLODUpdate, STATGROUP_PhysicsPerformance);
DECLARE_CYCLE_STAT(TEXT("Destruction Optimization"), STAT_DestructionOptimization, STATGROUP_PhysicsPerformance);
DECLARE_CYCLE_STAT(TEXT("Collision Optimization"), STAT_CollisionOptimization, STATGROUP_PhysicsPerformance);
DECLARE_CYCLE_STAT(TEXT("Ragdoll Optimization"), STAT_RagdollOptimization, STATGROUP_PhysicsPerformance);

DECLARE_DWORD_COUNTER_STAT(TEXT("Active Physics Bodies"), STAT_ActivePhysicsBodies, STATGROUP_PhysicsPerformance);
DECLARE_DWORD_COUNTER_STAT(TEXT("Active Destruction Chunks"), STAT_ActiveDestructionChunks, STATGROUP_PhysicsPerformance);
DECLARE_DWORD_COUNTER_STAT(TEXT("Active Ragdolls"), STAT_ActiveRagdolls, STATGROUP_PhysicsPerformance);
DECLARE_DWORD_COUNTER_STAT(TEXT("Collision Tests"), STAT_CollisionTests, STATGROUP_PhysicsPerformance);

UPhysicsPerformanceOptimizer::UPhysicsPerformanceOptimizer()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    // Initialize default physics budget based on PC high-end target
    PhysicsBudget.TargetFrameTime = 16.67f; // 60 FPS
    PhysicsBudget.PhysicsAllocation = 2.0f; // 2ms for physics
    PhysicsBudget.MemoryBudgetMB = 512.0f; // 512MB for physics
    
    // Initialize default LOD distances for prehistoric environment
    PhysicsLODDistances = {
        500.0f,   // Disabled distance - very close objects get full physics
        2000.0f,  // Minimal distance - basic collision only
        5000.0f,  // Reduced distance - simplified physics
        15000.0f, // Standard distance - normal physics
        40000.0f  // High distance - full detail for important objects
    };
    
    // Performance limits for Jurassic ecosystem
    MaxActivePhysicsBodies = 8000;     // Reduced from 10000 for safety
    MaxActiveDestructionChunks = 3000; // Reduced from 5000 for safety
    MaxActiveRagdolls = 75;            // Reduced from 100 for safety
    MaxCollisionTestsPerFrame = 400;   // Reduced from 500 for safety
    
    // Optimization settings
    bEnableAdaptiveTickRate = true;
    bEnableDistanceCulling = true;
    bEnableImportanceOptimization = true;
    EmergencyOptimizationThreshold = 1.3f; // More aggressive threshold
    
    CurrentPerformanceTarget = EPerformanceTarget::PC_HighEnd;
    
    UE_LOG(LogTemp, Log, TEXT("PhysicsPerformanceOptimizer: Initialized for Jurassic Survival Game"));
}

void UPhysicsPerformanceOptimizer::BeginPlay()
{
    Super::BeginPlay();
    
    InitializePerformanceMonitoring();
    InitializePhysicsOptimization(CurrentPerformanceTarget);
    
    UE_LOG(LogTemp, Log, TEXT("PhysicsPerformanceOptimizer: BeginPlay completed"));
}

void UPhysicsPerformanceOptimizer::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    SCOPE_CYCLE_COUNTER(STAT_PhysicsOptimization);
    
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update performance metrics every frame
    UpdatePhysicsMetrics();
    
    // Perform frame optimization
    OptimizePhysicsForFrame();
    
    // Check if emergency optimization is needed
    if (CurrentMetrics.PhysicsFrameTime > PhysicsBudget.PhysicsAllocation * EmergencyOptimizationThreshold)
    {
        ForceEmergencyOptimization();
    }
    
    // Update stats for profiling
    SET_DWORD_STAT(STAT_ActivePhysicsBodies, CurrentMetrics.ActivePhysicsBodies);
    SET_DWORD_STAT(STAT_ActiveDestructionChunks, CurrentMetrics.ActiveDestructionChunks);
    SET_DWORD_STAT(STAT_ActiveRagdolls, CurrentMetrics.ActiveRagdolls);
    SET_DWORD_STAT(STAT_CollisionTests, CurrentMetrics.CollisionTestsThisFrame);
}

void UPhysicsPerformanceOptimizer::InitializePhysicsOptimization(EPerformanceTarget PerformanceTarget)
{
    CurrentPerformanceTarget = PerformanceTarget;
    
    // Configure physics budget based on performance target
    switch (PerformanceTarget)
    {
        case EPerformanceTarget::Console_Standard:
            PhysicsBudget.TargetFrameTime = 33.33f; // 30 FPS
            PhysicsBudget.PhysicsAllocation = 4.0f; // 4ms for physics
            PhysicsBudget.MemoryBudgetMB = 256.0f;
            MaxActivePhysicsBodies = 5000;
            MaxActiveDestructionChunks = 2000;
            MaxActiveRagdolls = 50;
            break;
            
        case EPerformanceTarget::Console_Enhanced:
            PhysicsBudget.TargetFrameTime = 16.67f; // 60 FPS
            PhysicsBudget.PhysicsAllocation = 3.0f; // 3ms for physics
            PhysicsBudget.MemoryBudgetMB = 384.0f;
            MaxActivePhysicsBodies = 6500;
            MaxActiveDestructionChunks = 2500;
            MaxActiveRagdolls = 65;
            break;
            
        case EPerformanceTarget::PC_Standard:
            PhysicsBudget.TargetFrameTime = 16.67f; // 60 FPS
            PhysicsBudget.PhysicsAllocation = 2.5f; // 2.5ms for physics
            PhysicsBudget.MemoryBudgetMB = 512.0f;
            MaxActivePhysicsBodies = 7500;
            MaxActiveDestructionChunks = 3000;
            MaxActiveRagdolls = 75;
            break;
            
        case EPerformanceTarget::PC_HighEnd:
        default:
            PhysicsBudget.TargetFrameTime = 16.67f; // 60 FPS
            PhysicsBudget.PhysicsAllocation = 2.0f; // 2ms for physics
            PhysicsBudget.MemoryBudgetMB = 768.0f;
            MaxActivePhysicsBodies = 8000;
            MaxActiveDestructionChunks = 3000;
            MaxActiveRagdolls = 75;
            break;
    }
    
    UE_LOG(LogTemp, Log, TEXT("PhysicsPerformanceOptimizer: Initialized for target %d"), (int32)PerformanceTarget);
    UE_LOG(LogTemp, Log, TEXT("Physics Budget: %.2fms, Memory: %.0fMB"), PhysicsBudget.PhysicsAllocation, PhysicsBudget.MemoryBudgetMB);
}

void UPhysicsPerformanceOptimizer::OptimizePhysicsForFrame()
{
    SCOPE_CYCLE_COUNTER(STAT_PhysicsOptimization);
    
    if (!GetWorld())
    {
        return;
    }
    
    // Get player location for distance-based optimization
    FVector PlayerLocation = FVector::ZeroVector;
    if (APawn* PlayerPawn = GetWorld()->GetFirstPlayerController()->GetPawn())
    {
        PlayerLocation = PlayerPawn->GetActorLocation();
    }
    
    // Perform different optimization passes
    if (bEnableDistanceCulling)
    {
        CullDistantPhysicsObjects(PlayerLocation);
    }
    
    OptimizeDestructionSystem();
    OptimizeCollisionDetection();
    OptimizeRagdollPhysics();
    
    // Apply adaptive tick rates if enabled
    if (bEnableAdaptiveTickRate)
    {
        ApplyAdaptivePhysicsTick();
    }
}

void UPhysicsPerformanceOptimizer::SetPhysicsLOD(AActor* Actor, FVector ViewerLocation, float ImportanceMultiplier)
{
    SCOPE_CYCLE_COUNTER(STAT_PhysicsLODUpdate);
    
    if (!Actor || !IsValid(Actor))
    {
        return;
    }
    
    // Calculate distance to viewer
    float Distance = FVector::Dist(Actor->GetActorLocation(), ViewerLocation);
    
    // Apply importance multiplier to effective distance
    float EffectiveDistance = Distance / FMath::Max(ImportanceMultiplier, 0.1f);
    
    // Get optimal LOD level
    EPhysicsLODLevel OptimalLOD = GetOptimalPhysicsLOD(EffectiveDistance, ImportanceMultiplier);
    
    // Apply LOD to actor
    ApplyPhysicsLODToActor(Actor, OptimalLOD);
}

void UPhysicsPerformanceOptimizer::OptimizeDestructionSystem()
{
    SCOPE_CYCLE_COUNTER(STAT_DestructionOptimization);
    
    if (!GetWorld())
    {
        return;
    }
    
    ManageDestructionChunks();
    
    // Limit destruction events per frame based on performance
    if (CurrentMetrics.ActiveDestructionChunks > MaxActiveDestructionChunks * 0.8f)
    {
        // Reduce destruction complexity when approaching limits
        LimitDestructionComplexity();
    }
}

void UPhysicsPerformanceOptimizer::OptimizeCollisionDetection()
{
    SCOPE_CYCLE_COUNTER(STAT_CollisionOptimization);
    
    if (!GetWorld())
    {
        return;
    }
    
    // Implement hierarchical collision optimization
    OptimizeCollisionHierarchy();
    
    // Manage collision test budget
    if (CurrentMetrics.CollisionTestsThisFrame > MaxCollisionTestsPerFrame * 0.9f)
    {
        ReduceCollisionComplexity();
    }
}

void UPhysicsPerformanceOptimizer::OptimizeRagdollPhysics()
{
    SCOPE_CYCLE_COUNTER(STAT_RagdollOptimization);
    
    if (!GetWorld())
    {
        return;
    }
    
    // Clean up old ragdolls
    CleanupOldRagdolls();
    
    // Apply distance-based ragdoll LOD
    ApplyRagdollDistanceLOD();
    
    // Limit active ragdoll count
    if (CurrentMetrics.ActiveRagdolls > MaxActiveRagdolls)
    {
        CullExcessRagdolls();
    }
}

FPhysicsPerformanceMetrics UPhysicsPerformanceOptimizer::GetCurrentPhysicsMetrics() const
{
    return CurrentMetrics;
}

void UPhysicsPerformanceOptimizer::ForceEmergencyOptimization()
{
    UE_LOG(LogTemp, Warning, TEXT("PhysicsPerformanceOptimizer: Emergency optimization triggered! Frame time: %.2fms"), CurrentMetrics.PhysicsFrameTime);
    
    if (!GetWorld())
    {
        return;
    }
    
    // Aggressively cull physics objects
    FVector PlayerLocation = FVector::ZeroVector;
    if (APawn* PlayerPawn = GetWorld()->GetFirstPlayerController()->GetPawn())
    {
        PlayerLocation = PlayerPawn->GetActorLocation();
    }
    
    // Reduce LOD distances temporarily
    TArray<float> EmergencyLODDistances = PhysicsLODDistances;
    for (float& Distance : EmergencyLODDistances)
    {
        Distance *= 0.5f; // Halve all LOD distances
    }
    
    // Apply emergency culling
    EmergencyCullPhysicsObjects(PlayerLocation, EmergencyLODDistances);
    
    // Reduce active limits temporarily
    int32 EmergencyPhysicsBodies = MaxActivePhysicsBodies * 0.6f;
    int32 EmergencyDestructionChunks = MaxActiveDestructionChunks * 0.4f;
    int32 EmergencyRagdolls = MaxActiveRagdolls * 0.5f;
    
    // Force cleanup of excess objects
    ForceCleanupExcessObjects(EmergencyPhysicsBodies, EmergencyDestructionChunks, EmergencyRagdolls);
}

void UPhysicsPerformanceOptimizer::InitializePerformanceMonitoring()
{
    // Initialize performance tracking
    CurrentMetrics = FPhysicsPerformanceMetrics();
    
    UE_LOG(LogTemp, Log, TEXT("PhysicsPerformanceOptimizer: Performance monitoring initialized"));
}

void UPhysicsPerformanceOptimizer::UpdatePhysicsMetrics()
{
    if (!GetWorld())
    {
        return;
    }
    
    // Get physics world
    FPhysScene* PhysScene = GetWorld()->GetPhysicsScene();
    if (!PhysScene)
    {
        return;
    }
    
    // Update frame timing (simplified - would need more detailed profiling in real implementation)
    CurrentMetrics.PhysicsFrameTime = GEngine->GetMaxTickRate() > 0 ? (1000.0f / GEngine->GetMaxTickRate()) * 0.12f : 2.0f;
    
    // Count active physics bodies
    CurrentMetrics.ActivePhysicsBodies = CountActivePhysicsBodies();
    
    // Count active destruction chunks
    CurrentMetrics.ActiveDestructionChunks = CountActiveDestructionChunks();
    
    // Count active ragdolls
    CurrentMetrics.ActiveRagdolls = CountActiveRagdolls();
    
    // Track collision tests (simplified)
    CurrentMetrics.CollisionTestsThisFrame = FMath::RandRange(200, 600);
    
    // Estimate physics memory usage
    CurrentMetrics.PhysicsMemoryUsageMB = EstimatePhysicsMemoryUsage();
    
    // Check if within budget
    CurrentMetrics.bWithinBudget = (CurrentMetrics.PhysicsFrameTime <= PhysicsBudget.PhysicsAllocation) &&
                                   (CurrentMetrics.PhysicsMemoryUsageMB <= PhysicsBudget.MemoryBudgetMB);
    
    // Calculate performance efficiency
    if (PhysicsBudget.PhysicsAllocation > 0)
    {
        CurrentMetrics.PerformanceEfficiency = FMath::Clamp(PhysicsBudget.PhysicsAllocation / FMath::Max(CurrentMetrics.PhysicsFrameTime, 0.1f), 0.0f, 1.0f);
    }
}

EPhysicsLODLevel UPhysicsPerformanceOptimizer::GetOptimalPhysicsLOD(float Distance, float Importance) const
{
    // Apply importance weighting to distance
    float WeightedDistance = Distance / FMath::Max(Importance, 0.1f);
    
    if (WeightedDistance < PhysicsLODDistances[0])
    {
        return EPhysicsLODLevel::Maximum;
    }
    else if (WeightedDistance < PhysicsLODDistances[1])
    {
        return EPhysicsLODLevel::High;
    }
    else if (WeightedDistance < PhysicsLODDistances[2])
    {
        return EPhysicsLODLevel::Standard;
    }
    else if (WeightedDistance < PhysicsLODDistances[3])
    {
        return EPhysicsLODLevel::Reduced;
    }
    else if (WeightedDistance < PhysicsLODDistances[4])
    {
        return EPhysicsLODLevel::Minimal;
    }
    else
    {
        return EPhysicsLODLevel::Disabled;
    }
}

void UPhysicsPerformanceOptimizer::ApplyPhysicsLODToActor(AActor* Actor, EPhysicsLODLevel LODLevel)
{
    if (!Actor || !IsValid(Actor))
    {
        return;
    }
    
    // Get all primitive components
    TArray<UPrimitiveComponent*> PrimitiveComponents;
    Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
    
    for (UPrimitiveComponent* PrimComp : PrimitiveComponents)
    {
        if (!PrimComp || !IsValid(PrimComp))
        {
            continue;
        }
        
        switch (LODLevel)
        {
            case EPhysicsLODLevel::Disabled:
                PrimComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
                PrimComp->SetSimulatePhysics(false);
                break;
                
            case EPhysicsLODLevel::Minimal:
                PrimComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
                PrimComp->SetSimulatePhysics(false);
                break;
                
            case EPhysicsLODLevel::Reduced:
                PrimComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
                PrimComp->SetSimulatePhysics(true);
                // Reduce physics complexity
                if (FBodyInstance* BodyInstance = PrimComp->GetBodyInstance())
                {
                    BodyInstance->SetResponseToAllChannels(ECR_Block);
                }
                break;
                
            case EPhysicsLODLevel::Standard:
            case EPhysicsLODLevel::High:
            case EPhysicsLODLevel::Maximum:
            default:
                PrimComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
                PrimComp->SetSimulatePhysics(true);
                // Full physics detail
                break;
        }
    }
}

void UPhysicsPerformanceOptimizer::CullDistantPhysicsObjects(FVector ViewerLocation)
{
    if (!GetWorld())
    {
        return;
    }
    
    // Iterate through all actors and apply distance-based culling
    for (TActorIterator<AActor> ActorIterator(GetWorld()); ActorIterator; ++ActorIterator)
    {
        AActor* Actor = *ActorIterator;
        if (!Actor || !IsValid(Actor))
        {
            continue;
        }
        
        // Skip important actors (player, key gameplay objects)
        if (ShouldSkipPhysicsOptimization(Actor))
        {
            continue;
        }
        
        // Apply distance-based LOD
        SetPhysicsLOD(Actor, ViewerLocation, GetActorImportance(Actor));
    }
}

void UPhysicsPerformanceOptimizer::ManageDestructionChunks()
{
    // Implementation would manage destruction chunk lifecycle
    // This is a simplified version
    
    if (!GetWorld())
    {
        return;
    }
    
    // Count and manage destruction chunks
    int32 ChunkCount = 0;
    TArray<AActor*> DestructionActors;
    
    for (TActorIterator<AActor> ActorIterator(GetWorld()); ActorIterator; ++ActorIterator)
    {
        AActor* Actor = *ActorIterator;
        if (Actor && Actor->GetName().Contains(TEXT("Destruction")))
        {
            DestructionActors.Add(Actor);
            ChunkCount++;
        }
    }
    
    // If we have too many chunks, remove the oldest/furthest ones
    if (ChunkCount > MaxActiveDestructionChunks)
    {
        int32 ExcessChunks = ChunkCount - MaxActiveDestructionChunks;
        for (int32 i = 0; i < ExcessChunks && i < DestructionActors.Num(); i++)
        {
            if (DestructionActors[i] && IsValid(DestructionActors[i]))
            {
                DestructionActors[i]->Destroy();
            }
        }
    }
}

int32 UPhysicsPerformanceOptimizer::CountActivePhysicsBodies() const
{
    if (!GetWorld())
    {
        return 0;
    }
    
    int32 Count = 0;
    for (TActorIterator<AActor> ActorIterator(GetWorld()); ActorIterator; ++ActorIterator)
    {
        AActor* Actor = *ActorIterator;
        if (!Actor || !IsValid(Actor))
        {
            continue;
        }
        
        TArray<UPrimitiveComponent*> PrimitiveComponents;
        Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
        
        for (UPrimitiveComponent* PrimComp : PrimitiveComponents)
        {
            if (PrimComp && PrimComp->IsSimulatingPhysics())
            {
                Count++;
            }
        }
    }
    
    return Count;
}

int32 UPhysicsPerformanceOptimizer::CountActiveDestructionChunks() const
{
    if (!GetWorld())
    {
        return 0;
    }
    
    int32 Count = 0;
    for (TActorIterator<AActor> ActorIterator(GetWorld()); ActorIterator; ++ActorIterator)
    {
        AActor* Actor = *ActorIterator;
        if (Actor && Actor->GetName().Contains(TEXT("Destruction")))
        {
            Count++;
        }
    }
    
    return Count;
}

int32 UPhysicsPerformanceOptimizer::CountActiveRagdolls() const
{
    if (!GetWorld())
    {
        return 0;
    }
    
    int32 Count = 0;
    for (TActorIterator<APawn> PawnIterator(GetWorld()); PawnIterator; ++PawnIterator)
    {
        APawn* Pawn = *PawnIterator;
        if (!Pawn || !IsValid(Pawn))
        {
            continue;
        }
        
        if (USkeletalMeshComponent* SkelMesh = Pawn->FindComponentByClass<USkeletalMeshComponent>())
        {
            if (SkelMesh->IsSimulatingPhysics())
            {
                Count++;
            }
        }
    }
    
    return Count;
}

float UPhysicsPerformanceOptimizer::EstimatePhysicsMemoryUsage() const
{
    // Simplified estimation - in real implementation would query actual memory usage
    float EstimatedMB = 0.0f;
    
    EstimatedMB += CurrentMetrics.ActivePhysicsBodies * 0.05f; // ~50KB per physics body
    EstimatedMB += CurrentMetrics.ActiveDestructionChunks * 0.02f; // ~20KB per destruction chunk
    EstimatedMB += CurrentMetrics.ActiveRagdolls * 0.5f; // ~500KB per ragdoll
    
    return EstimatedMB;
}

bool UPhysicsPerformanceOptimizer::ShouldSkipPhysicsOptimization(AActor* Actor) const
{
    if (!Actor)
    {
        return true;
    }
    
    // Skip player and important gameplay actors
    if (Actor->IsA<APawn>() || Actor->GetName().Contains(TEXT("Player")))
    {
        return true;
    }
    
    // Skip actors marked as always important
    if (Actor->Tags.Contains(TEXT("PhysicsImportant")))
    {
        return true;
    }
    
    return false;
}

float UPhysicsPerformanceOptimizer::GetActorImportance(AActor* Actor) const
{
    if (!Actor)
    {
        return 1.0f;
    }
    
    // Higher importance for certain actor types
    if (Actor->IsA<APawn>())
    {
        return 3.0f; // Dinosaurs and creatures are very important
    }
    
    if (Actor->GetName().Contains(TEXT("Dinosaur")) || Actor->GetName().Contains(TEXT("Creature")))
    {
        return 2.5f;
    }
    
    if (Actor->GetName().Contains(TEXT("Tree")) || Actor->GetName().Contains(TEXT("Rock")))
    {
        return 1.5f; // Environment objects moderately important
    }
    
    return 1.0f; // Default importance
}

void UPhysicsPerformanceOptimizer::ApplyAdaptivePhysicsTick()
{
    // Implement adaptive physics tick rates based on performance
    // This would adjust physics simulation frequency dynamically
}

void UPhysicsPerformanceOptimizer::LimitDestructionComplexity()
{
    // Reduce destruction complexity when performance is stressed
}

void UPhysicsPerformanceOptimizer::OptimizeCollisionHierarchy()
{
    // Implement hierarchical collision optimization
}

void UPhysicsPerformanceOptimizer::ReduceCollisionComplexity()
{
    // Reduce collision detection complexity
}

void UPhysicsPerformanceOptimizer::CleanupOldRagdolls()
{
    // Clean up ragdolls that have been inactive for too long
}

void UPhysicsPerformanceOptimizer::ApplyRagdollDistanceLOD()
{
    // Apply distance-based LOD to ragdoll physics
}

void UPhysicsPerformanceOptimizer::CullExcessRagdolls()
{
    // Remove excess ragdolls when limit is exceeded
}

void UPhysicsPerformanceOptimizer::EmergencyCullPhysicsObjects(FVector ViewerLocation, const TArray<float>& EmergencyLODDistances)
{
    // Emergency culling with reduced LOD distances
}

void UPhysicsPerformanceOptimizer::ForceCleanupExcessObjects(int32 MaxBodies, int32 MaxChunks, int32 MaxRagdolls)
{
    // Force cleanup of excess physics objects during emergency optimization
}