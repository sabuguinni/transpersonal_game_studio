#include "Perf_LODManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/StaticMeshActor.h"
#include "TimerManager.h"

APerf_LODManager::APerf_LODManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // 10 FPS for performance monitoring
    
    // Set default LOD settings
    LODSettings.MaxDrawDistance = 10000.0f;
    LODSettings.CullingDistance = 15000.0f;
    LODSettings.MaxPhysicsActors = 100;
    LODSettings.bEnableDistanceCulling = true;
    LODSettings.bOptimizeCollision = true;
    
    CurrentLODLevel = EPerf_LODLevel::High;
    PerformanceCheckInterval = 1.0f;
    TargetFrameRate = 60.0f;
    MinAcceptableFrameRate = 45.0f;
    
    LastPerformanceCheck = 0.0f;
    FrameTimeAccumulator = 0.0f;
    FrameCount = 0;
    AverageFrameTime = 0.0f;
}

void APerf_LODManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Find player pawn
    PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
    if (!PlayerPawn)
    {
        UE_LOG(LogTemp, Warning, TEXT("LODManager: Player pawn not found"));
    }
    
    // Auto-register nearby actors for LOD management
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(this, AActor::StaticClass(), AllActors);
    
    for (AActor* Actor : AllActors)
    {
        if (Actor && Actor != this && Actor->IsA<AStaticMeshActor>())
        {
            RegisterActorForLOD(Actor);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("LODManager: Initialized with %d managed actors"), ManagedActors.Num());
}

void APerf_LODManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    UpdatePerformanceMetrics(DeltaTime);
    
    // Check performance and adjust LOD periodically
    LastPerformanceCheck += DeltaTime;
    if (LastPerformanceCheck >= PerformanceCheckInterval)
    {
        AutoAdjustLODLevel();
        UpdateDistanceCulling();
        LastPerformanceCheck = 0.0f;
    }
}

void APerf_LODManager::UpdatePerformanceMetrics(float DeltaTime)
{
    FrameTimeAccumulator += DeltaTime;
    FrameCount++;
    
    // Calculate average frame time over 60 frames
    if (FrameCount >= 60)
    {
        AverageFrameTime = FrameTimeAccumulator / FrameCount;
        FrameTimeAccumulator = 0.0f;
        FrameCount = 0;
    }
}

void APerf_LODManager::RegisterActorForLOD(AActor* Actor)
{
    if (Actor && !ManagedActors.Contains(Actor))
    {
        ManagedActors.Add(Actor);
        ApplyLODToActor(Actor, CurrentLODLevel);
    }
}

void APerf_LODManager::UnregisterActorFromLOD(AActor* Actor)
{
    if (Actor)
    {
        ManagedActors.Remove(Actor);
    }
}

void APerf_LODManager::SetLODLevel(EPerf_LODLevel NewLODLevel)
{
    if (CurrentLODLevel != NewLODLevel)
    {
        CurrentLODLevel = NewLODLevel;
        
        // Apply new LOD level to all managed actors
        for (AActor* Actor : ManagedActors)
        {
            if (IsValid(Actor))
            {
                ApplyLODToActor(Actor, CurrentLODLevel);
            }
        }
        
        UE_LOG(LogTemp, Log, TEXT("LODManager: Set LOD level to %d"), (int32)CurrentLODLevel);
    }
}

void APerf_LODManager::ApplyLODToActor(AActor* Actor, EPerf_LODLevel LODLevel)
{
    if (!IsValid(Actor))
        return;
    
    // Get mesh components
    TArray<UStaticMeshComponent*> StaticMeshComps;
    Actor->GetComponents<UStaticMeshComponent>(StaticMeshComps);
    
    TArray<USkeletalMeshComponent*> SkeletalMeshComps;
    Actor->GetComponents<USkeletalMeshComponent>(SkeletalMeshComps);
    
    float LODDistanceMultiplier = 1.0f;
    bool bEnableCollision = true;
    
    switch (LODLevel)
    {
        case EPerf_LODLevel::Ultra:
            LODDistanceMultiplier = 1.5f;
            bEnableCollision = true;
            break;
        case EPerf_LODLevel::High:
            LODDistanceMultiplier = 1.0f;
            bEnableCollision = true;
            break;
        case EPerf_LODLevel::Medium:
            LODDistanceMultiplier = 0.8f;
            bEnableCollision = true;
            break;
        case EPerf_LODLevel::Low:
            LODDistanceMultiplier = 0.6f;
            bEnableCollision = false;
            break;
        case EPerf_LODLevel::Minimal:
            LODDistanceMultiplier = 0.4f;
            bEnableCollision = false;
            break;
    }
    
    // Apply settings to static mesh components
    for (UStaticMeshComponent* MeshComp : StaticMeshComps)
    {
        if (MeshComp)
        {
            float MaxDistance = LODSettings.MaxDrawDistance * LODDistanceMultiplier;
            MeshComp->SetCullDistance(MaxDistance);
            
            if (LODSettings.bOptimizeCollision)
            {
                OptimizeActorCollision(Actor, bEnableCollision);
            }
        }
    }
    
    // Apply settings to skeletal mesh components
    for (USkeletalMeshComponent* SkeletalComp : SkeletalMeshComps)
    {
        if (SkeletalComp)
        {
            float MaxDistance = LODSettings.MaxDrawDistance * LODDistanceMultiplier;
            SkeletalComp->SetCullDistance(MaxDistance);
        }
    }
}

void APerf_LODManager::OptimizeActorCollision(AActor* Actor, bool bOptimize)
{
    if (!IsValid(Actor))
        return;
    
    TArray<UPrimitiveComponent*> PrimitiveComps;
    Actor->GetComponents<UPrimitiveComponent>(PrimitiveComps);
    
    for (UPrimitiveComponent* PrimComp : PrimitiveComps)
    {
        if (PrimComp)
        {
            if (bOptimize)
            {
                // Enable collision but optimize it
                PrimComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
                PrimComp->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
            }
            else
            {
                // Disable collision for distant objects
                PrimComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            }
        }
    }
}

float APerf_LODManager::GetDistanceToPlayer(AActor* Actor) const
{
    if (!IsValid(Actor) || !IsValid(PlayerPawn))
        return 99999.0f;
    
    return FVector::Dist(Actor->GetActorLocation(), PlayerPawn->GetActorLocation());
}

void APerf_LODManager::UpdateDistanceCulling()
{
    if (!LODSettings.bEnableDistanceCulling || !IsValid(PlayerPawn))
        return;
    
    for (AActor* Actor : ManagedActors)
    {
        if (!IsValid(Actor))
            continue;
        
        float Distance = GetDistanceToPlayer(Actor);
        
        // Hide actors beyond culling distance
        bool bShouldBeVisible = Distance <= LODSettings.CullingDistance;
        Actor->SetActorHiddenInGame(!bShouldBeVisible);
        
        // Disable tick for very distant actors
        bool bShouldTick = Distance <= LODSettings.MaxDrawDistance;
        Actor->SetActorTickEnabled(bShouldTick);
    }
}

void APerf_LODManager::OptimizePhysicsActors()
{
    TArray<AActor*> PhysicsActors;
    
    for (AActor* Actor : ManagedActors)
    {
        if (!IsValid(Actor))
            continue;
        
        TArray<UPrimitiveComponent*> PrimitiveComps;
        Actor->GetComponents<UPrimitiveComponent>(PrimitiveComps);
        
        for (UPrimitiveComponent* PrimComp : PrimitiveComps)
        {
            if (PrimComp && PrimComp->IsSimulatingPhysics())
            {
                PhysicsActors.Add(Actor);
                break;
            }
        }
    }
    
    // Disable physics for excess actors (keep closest to player)
    if (PhysicsActors.Num() > LODSettings.MaxPhysicsActors)
    {
        // Sort by distance to player
        PhysicsActors.Sort([this](const AActor& A, const AActor& B) {
            float DistA = GetDistanceToPlayer(const_cast<AActor*>(&A));
            float DistB = GetDistanceToPlayer(const_cast<AActor*>(&B));
            return DistA < DistB;
        });
        
        // Disable physics for distant actors
        for (int32 i = LODSettings.MaxPhysicsActors; i < PhysicsActors.Num(); i++)
        {
            AActor* Actor = PhysicsActors[i];
            TArray<UPrimitiveComponent*> PrimitiveComps;
            Actor->GetComponents<UPrimitiveComponent>(PrimitiveComps);
            
            for (UPrimitiveComponent* PrimComp : PrimitiveComps)
            {
                if (PrimComp && PrimComp->IsSimulatingPhysics())
                {
                    PrimComp->SetSimulatePhysics(false);
                }
            }
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("LODManager: Optimized physics for %d actors"), PhysicsActors.Num());
}

float APerf_LODManager::GetCurrentFrameRate() const
{
    if (AverageFrameTime > 0.0f)
    {
        return 1.0f / AverageFrameTime;
    }
    return 0.0f;
}

void APerf_LODManager::AutoAdjustLODLevel()
{
    float CurrentFPS = GetCurrentFrameRate();
    
    if (CurrentFPS <= 0.0f)
        return; // Not enough data yet
    
    EPerf_LODLevel NewLODLevel = CurrentLODLevel;
    
    if (CurrentFPS < MinAcceptableFrameRate)
    {
        // Performance is poor, reduce LOD level
        switch (CurrentLODLevel)
        {
            case EPerf_LODLevel::Ultra:
                NewLODLevel = EPerf_LODLevel::High;
                break;
            case EPerf_LODLevel::High:
                NewLODLevel = EPerf_LODLevel::Medium;
                break;
            case EPerf_LODLevel::Medium:
                NewLODLevel = EPerf_LODLevel::Low;
                break;
            case EPerf_LODLevel::Low:
                NewLODLevel = EPerf_LODLevel::Minimal;
                break;
            case EPerf_LODLevel::Minimal:
                // Already at minimum
                break;
        }
    }
    else if (CurrentFPS > TargetFrameRate + 10.0f)
    {
        // Performance is good, can increase LOD level
        switch (CurrentLODLevel)
        {
            case EPerf_LODLevel::Minimal:
                NewLODLevel = EPerf_LODLevel::Low;
                break;
            case EPerf_LODLevel::Low:
                NewLODLevel = EPerf_LODLevel::Medium;
                break;
            case EPerf_LODLevel::Medium:
                NewLODLevel = EPerf_LODLevel::High;
                break;
            case EPerf_LODLevel::High:
                NewLODLevel = EPerf_LODLevel::Ultra;
                break;
            case EPerf_LODLevel::Ultra:
                // Already at maximum
                break;
        }
    }
    
    if (NewLODLevel != CurrentLODLevel)
    {
        SetLODLevel(NewLODLevel);
        UE_LOG(LogTemp, Log, TEXT("LODManager: Auto-adjusted LOD from %d to %d (FPS: %.1f)"), 
               (int32)CurrentLODLevel, (int32)NewLODLevel, CurrentFPS);
    }
}