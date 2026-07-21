#include "Perf_LODManager.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Kismet/GameplayStatics.h"

UPerf_LODManager::UPerf_LODManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;
    
    UpdateFrequency = 0.1f;
    bAutoManageLOD = true;
    CullingDistance = 10000.0f;
    TimeSinceLastUpdate = 0.0f;
    PlayerPawn = nullptr;

    // Initialize default LOD levels
    LODLevels.Empty();
    
    FPerf_LODLevel LOD0;
    LOD0.Distance = 0.0f;
    LOD0.LODIndex = 0;
    LOD0.bShouldRender = true;
    LODLevels.Add(LOD0);

    FPerf_LODLevel LOD1;
    LOD1.Distance = 2000.0f;
    LOD1.LODIndex = 1;
    LOD1.bShouldRender = true;
    LODLevels.Add(LOD1);

    FPerf_LODLevel LOD2;
    LOD2.Distance = 5000.0f;
    LOD2.LODIndex = 2;
    LOD2.bShouldRender = true;
    LODLevels.Add(LOD2);

    FPerf_LODLevel LOD3;
    LOD3.Distance = 8000.0f;
    LOD3.LODIndex = 3;
    LOD3.bShouldRender = false;
    LODLevels.Add(LOD3);
}

void UPerf_LODManager::BeginPlay()
{
    Super::BeginPlay();
    FindPlayerPawn();
}

void UPerf_LODManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!bAutoManageLOD)
    {
        return;
    }

    TimeSinceLastUpdate += DeltaTime;
    
    if (TimeSinceLastUpdate >= UpdateFrequency)
    {
        UpdateAllManagedActors();
        TimeSinceLastUpdate = 0.0f;
    }
}

void UPerf_LODManager::UpdateLODForActor(AActor* Actor, float DistanceToPlayer)
{
    if (!Actor)
    {
        return;
    }

    // Check if actor should be culled
    if (DistanceToPlayer > CullingDistance)
    {
        CullActor(Actor, true);
        return;
    }
    else
    {
        CullActor(Actor, false);
    }

    // Find appropriate LOD level
    int32 TargetLOD = GetCurrentLODLevel(DistanceToPlayer);
    SetLODLevel(Actor, TargetLOD);
}

void UPerf_LODManager::SetLODLevel(AActor* Actor, int32 LODIndex)
{
    if (!Actor)
    {
        return;
    }

    // Handle StaticMeshActor LOD
    if (AStaticMeshActor* MeshActor = Cast<AStaticMeshActor>(Actor))
    {
        if (UStaticMeshComponent* MeshComp = MeshActor->GetStaticMeshComponent())
        {
            MeshComp->SetForcedLodModel(LODIndex + 1); // UE5 uses 1-based LOD indexing
        }
    }

    // Handle other actor types with mesh components
    TArray<UStaticMeshComponent*> MeshComponents;
    Actor->GetComponents<UStaticMeshComponent>(MeshComponents);
    
    for (UStaticMeshComponent* MeshComp : MeshComponents)
    {
        if (MeshComp)
        {
            MeshComp->SetForcedLodModel(LODIndex + 1);
        }
    }
}

void UPerf_LODManager::CullActor(AActor* Actor, bool bShouldCull)
{
    if (!Actor)
    {
        return;
    }

    Actor->SetActorHiddenInGame(bShouldCull);
    Actor->SetActorEnableCollision(!bShouldCull);
}

int32 UPerf_LODManager::GetCurrentLODLevel(float Distance)
{
    for (int32 i = LODLevels.Num() - 1; i >= 0; i--)
    {
        if (Distance >= LODLevels[i].Distance)
        {
            return LODLevels[i].LODIndex;
        }
    }
    
    return 0; // Default to highest quality LOD
}

void UPerf_LODManager::RegisterActorForLOD(AActor* Actor)
{
    if (!Actor)
    {
        return;
    }

    TWeakObjectPtr<AActor> WeakActor(Actor);
    if (!ManagedActors.Contains(WeakActor))
    {
        ManagedActors.Add(WeakActor);
    }
}

void UPerf_LODManager::UnregisterActorFromLOD(AActor* Actor)
{
    if (!Actor)
    {
        return;
    }

    TWeakObjectPtr<AActor> WeakActor(Actor);
    ManagedActors.Remove(WeakActor);
}

void UPerf_LODManager::FindPlayerPawn()
{
    if (UWorld* World = GetWorld())
    {
        if (APlayerController* PC = World->GetFirstPlayerController())
        {
            PlayerPawn = PC->GetPawn();
        }
    }
}

void UPerf_LODManager::UpdateAllManagedActors()
{
    if (!PlayerPawn)
    {
        FindPlayerPawn();
        if (!PlayerPawn)
        {
            return;
        }
    }

    FVector PlayerLocation = PlayerPawn->GetActorLocation();

    // Clean up invalid weak pointers
    ManagedActors.RemoveAll([](const TWeakObjectPtr<AActor>& WeakActor)
    {
        return !WeakActor.IsValid();
    });

    // Update LOD for all managed actors
    for (const TWeakObjectPtr<AActor>& WeakActor : ManagedActors)
    {
        if (AActor* Actor = WeakActor.Get())
        {
            float Distance = CalculateDistanceToPlayer(Actor);
            UpdateLODForActor(Actor, Distance);
        }
    }
}

float UPerf_LODManager::CalculateDistanceToPlayer(AActor* Actor)
{
    if (!Actor || !PlayerPawn)
    {
        return 0.0f;
    }

    return FVector::Dist(Actor->GetActorLocation(), PlayerPawn->GetActorLocation());
}