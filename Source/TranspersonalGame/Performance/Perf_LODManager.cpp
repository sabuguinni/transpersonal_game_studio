#include "Perf_LODManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/SkeletalMesh.h"
#include "DrawDebugHelpers.h"

UPerf_LODManager::UPerf_LODManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;
    
    UpdateFrequency = 0.1f;
    CurrentLODLevel = EPerf_LODLevel::High;
    DistanceToPlayer = 0.0f;
    ManagedActorsCount = 0;
    LastUpdateTime = 0.0f;
    CachedPlayerPawn = nullptr;
}

void UPerf_LODManager::BeginPlay()
{
    Super::BeginPlay();
    
    CachePlayerPawn();
    LastUpdateTime = GetWorld()->GetTimeSeconds();
    
    // Register owner actor for LOD management
    if (AActor* Owner = GetOwner())
    {
        RegisterActorForLOD(Owner);
    }
}

void UPerf_LODManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastUpdateTime >= UpdateFrequency)
    {
        if (!CachedPlayerPawn)
        {
            CachePlayerPawn();
        }
        
        UpdateAllManagedActors();
        CleanupInvalidActors();
        LastUpdateTime = CurrentTime;
    }
}

void UPerf_LODManager::UpdateLODForActor(AActor* Actor)
{
    if (!Actor || !CachedPlayerPawn)
    {
        return;
    }
    
    float Distance = GetDistanceToPlayer(Actor);
    EPerf_LODLevel NewLODLevel = CalculateLODLevel(Distance);
    
    // Check frustum culling if enabled
    if (LODSettings.bEnableFrustumCulling && !IsActorInPlayerView(Actor))
    {
        NewLODLevel = EPerf_LODLevel::Culled;
    }
    
    SetLODLevel(Actor, NewLODLevel);
    
    if (Actor == GetOwner())
    {
        CurrentLODLevel = NewLODLevel;
        DistanceToPlayer = Distance;
    }
}

void UPerf_LODManager::UpdateAllManagedActors()
{
    for (int32 i = ManagedActors.Num() - 1; i >= 0; i--)
    {
        if (ManagedActors[i].IsValid())
        {
            UpdateLODForActor(ManagedActors[i].Get());
        }
        else
        {
            ManagedActors.RemoveAt(i);
        }
    }
    
    ManagedActorsCount = ManagedActors.Num();
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
        ManagedActorsCount = ManagedActors.Num();
        
        // Immediately update LOD for new actor
        UpdateLODForActor(Actor);
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
    ManagedActorsCount = ManagedActors.Num();
}

EPerf_LODLevel UPerf_LODManager::CalculateLODLevel(float Distance) const
{
    if (!LODSettings.bEnableDistanceCulling)
    {
        return EPerf_LODLevel::High;
    }
    
    if (Distance <= LODSettings.HighDetailDistance)
    {
        return EPerf_LODLevel::High;
    }
    else if (Distance <= LODSettings.MediumDetailDistance)
    {
        return EPerf_LODLevel::Medium;
    }
    else if (Distance <= LODSettings.LowDetailDistance)
    {
        return EPerf_LODLevel::Low;
    }
    else if (Distance <= LODSettings.CullDistance)
    {
        return EPerf_LODLevel::Low;
    }
    else
    {
        return EPerf_LODLevel::Culled;
    }
}

void UPerf_LODManager::SetLODLevel(AActor* Actor, EPerf_LODLevel LODLevel)
{
    if (!Actor)
    {
        return;
    }
    
    // Handle culling
    if (LODLevel == EPerf_LODLevel::Culled)
    {
        Actor->SetActorHiddenInGame(true);
        Actor->SetActorEnableCollision(false);
        Actor->SetActorTickEnabled(false);
        return;
    }
    else
    {
        Actor->SetActorHiddenInGame(false);
        Actor->SetActorEnableCollision(true);
        Actor->SetActorTickEnabled(true);
    }
    
    ApplyLODToComponents(Actor, LODLevel);
}

void UPerf_LODManager::OptimizeStaticMesh(UStaticMeshComponent* MeshComp, EPerf_LODLevel LODLevel)
{
    if (!MeshComp || !MeshComp->GetStaticMesh())
    {
        return;
    }
    
    int32 LODIndex = static_cast<int32>(LODLevel);
    int32 MaxLOD = MeshComp->GetStaticMesh()->GetNumLODs() - 1;
    
    // Clamp LOD index to available LODs
    LODIndex = FMath::Clamp(LODIndex, 0, MaxLOD);
    
    MeshComp->SetForcedLodModel(LODIndex + 1); // UE5 LOD is 1-based
    
    // Adjust shadow casting based on LOD level
    switch (LODLevel)
    {
        case EPerf_LODLevel::High:
            MeshComp->SetCastShadow(true);
            MeshComp->bCastDynamicShadow = true;
            break;
        case EPerf_LODLevel::Medium:
            MeshComp->SetCastShadow(true);
            MeshComp->bCastDynamicShadow = false;
            break;
        case EPerf_LODLevel::Low:
            MeshComp->SetCastShadow(false);
            MeshComp->bCastDynamicShadow = false;
            break;
        default:
            break;
    }
}

void UPerf_LODManager::OptimizeSkeletalMesh(USkeletalMeshComponent* MeshComp, EPerf_LODLevel LODLevel)
{
    if (!MeshComp || !MeshComp->GetSkeletalMeshAsset())
    {
        return;
    }
    
    int32 LODIndex = static_cast<int32>(LODLevel);
    
    MeshComp->SetForcedLOD(LODIndex + 1); // UE5 LOD is 1-based
    
    // Adjust animation and physics based on LOD level
    switch (LODLevel)
    {
        case EPerf_LODLevel::High:
            MeshComp->SetUpdateAnimationInEditor(true);
            MeshComp->SetCastShadow(true);
            break;
        case EPerf_LODLevel::Medium:
            MeshComp->SetUpdateAnimationInEditor(true);
            MeshComp->SetCastShadow(true);
            break;
        case EPerf_LODLevel::Low:
            MeshComp->SetUpdateAnimationInEditor(false);
            MeshComp->SetCastShadow(false);
            break;
        default:
            break;
    }
}

float UPerf_LODManager::GetDistanceToPlayer(AActor* Actor) const
{
    if (!Actor || !CachedPlayerPawn)
    {
        return 0.0f;
    }
    
    return FVector::Dist(Actor->GetActorLocation(), CachedPlayerPawn->GetActorLocation());
}

bool UPerf_LODManager::IsActorInPlayerView(AActor* Actor) const
{
    if (!Actor || !CachedPlayerPawn)
    {
        return false;
    }
    
    // Simple frustum check - can be enhanced with proper camera frustum
    FVector PlayerLocation = CachedPlayerPawn->GetActorLocation();
    FVector PlayerForward = CachedPlayerPawn->GetActorForwardVector();
    FVector ToActor = (Actor->GetActorLocation() - PlayerLocation).GetSafeNormal();
    
    float DotProduct = FVector::DotProduct(PlayerForward, ToActor);
    
    // Actor is in front of player (within 120 degree cone)
    return DotProduct > -0.5f;
}

void UPerf_LODManager::CachePlayerPawn()
{
    if (UWorld* World = GetWorld())
    {
        if (APlayerController* PC = World->GetFirstPlayerController())
        {
            CachedPlayerPawn = PC->GetPawn();
        }
    }
}

void UPerf_LODManager::CleanupInvalidActors()
{
    for (int32 i = ManagedActors.Num() - 1; i >= 0; i--)
    {
        if (!ManagedActors[i].IsValid())
        {
            ManagedActors.RemoveAt(i);
        }
    }
    
    ManagedActorsCount = ManagedActors.Num();
}

void UPerf_LODManager::ApplyLODToComponents(AActor* Actor, EPerf_LODLevel LODLevel)
{
    if (!Actor)
    {
        return;
    }
    
    // Apply LOD to all static mesh components
    TArray<UStaticMeshComponent*> StaticMeshComponents;
    Actor->GetComponents<UStaticMeshComponent>(StaticMeshComponents);
    
    for (UStaticMeshComponent* MeshComp : StaticMeshComponents)
    {
        OptimizeStaticMesh(MeshComp, LODLevel);
    }
    
    // Apply LOD to all skeletal mesh components
    TArray<USkeletalMeshComponent*> SkeletalMeshComponents;
    Actor->GetComponents<USkeletalMeshComponent>(SkeletalMeshComponents);
    
    for (USkeletalMeshComponent* MeshComp : SkeletalMeshComponents)
    {
        OptimizeSkeletalMesh(MeshComp, LODLevel);
    }
}