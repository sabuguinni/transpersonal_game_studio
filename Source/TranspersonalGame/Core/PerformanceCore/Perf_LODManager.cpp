#include "Perf_LODManager.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Engine/StaticMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"

UPerf_LODManager::UPerf_LODManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;
    
    LODSettings = FPerf_LODSettings();
    bEnableLODSystem = true;
    UpdateFrequency = 0.1f;
    CurrentLODLevel = EPerf_LODLevel::High;
    DistanceToPlayer = 0.0f;
    LastUpdateTime = 0.0f;
    PlayerPawn = nullptr;
}

void UPerf_LODManager::BeginPlay()
{
    Super::BeginPlay();
    
    FindPlayerPawn();
    LastUpdateTime = GetWorld()->GetTimeSeconds();
    
    // Initial LOD calculation
    UpdateLODLevel();
}

void UPerf_LODManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bEnableLODSystem)
        return;
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastUpdateTime >= UpdateFrequency)
    {
        UpdateLODLevel();
        LastUpdateTime = CurrentTime;
    }
}

void UPerf_LODManager::UpdateLODLevel()
{
    if (!PlayerPawn)
    {
        FindPlayerPawn();
        if (!PlayerPawn)
            return;
    }
    
    DistanceToPlayer = CalculateDistanceToPlayer();
    EPerf_LODLevel NewLODLevel = CalculateLODLevel(DistanceToPlayer);
    
    if (NewLODLevel != CurrentLODLevel)
    {
        ApplyLODLevel(NewLODLevel);
        CurrentLODLevel = NewLODLevel;
    }
    
    UpdateManagedActors();
}

EPerf_LODLevel UPerf_LODManager::CalculateLODLevel(float Distance)
{
    if (Distance <= LODSettings.HighDetailDistance)
        return EPerf_LODLevel::High;
    else if (Distance <= LODSettings.MediumDetailDistance)
        return EPerf_LODLevel::Medium;
    else if (Distance <= LODSettings.LowDetailDistance)
        return EPerf_LODLevel::Low;
    else if (Distance <= LODSettings.CullDistance)
        return EPerf_LODLevel::Culled;
    else
        return EPerf_LODLevel::Culled;
}

void UPerf_LODManager::ApplyLODLevel(EPerf_LODLevel NewLODLevel)
{
    AActor* Owner = GetOwner();
    if (!Owner)
        return;
    
    // Apply LOD to static mesh components
    TArray<UStaticMeshComponent*> MeshComponents;
    Owner->GetComponents<UStaticMeshComponent>(MeshComponents);
    
    for (UStaticMeshComponent* MeshComp : MeshComponents)
    {
        if (!MeshComp)
            continue;
        
        switch (NewLODLevel)
        {
            case EPerf_LODLevel::High:
                MeshComp->SetVisibility(true);
                MeshComp->SetForcedLodModel(0);
                break;
            case EPerf_LODLevel::Medium:
                MeshComp->SetVisibility(true);
                MeshComp->SetForcedLodModel(1);
                break;
            case EPerf_LODLevel::Low:
                MeshComp->SetVisibility(true);
                MeshComp->SetForcedLodModel(2);
                break;
            case EPerf_LODLevel::Culled:
                MeshComp->SetVisibility(false);
                break;
        }
    }
}

void UPerf_LODManager::RegisterManagedActor(AActor* Actor)
{
    if (Actor && !ManagedActors.Contains(Actor))
    {
        ManagedActors.Add(Actor);
    }
}

void UPerf_LODManager::UnregisterManagedActor(AActor* Actor)
{
    if (Actor)
    {
        ManagedActors.Remove(Actor);
    }
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

float UPerf_LODManager::CalculateDistanceToPlayer()
{
    if (!PlayerPawn || !GetOwner())
        return 0.0f;
    
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    
    return FVector::Dist(OwnerLocation, PlayerLocation);
}

void UPerf_LODManager::UpdateManagedActors()
{
    // Update LOD for all managed actors
    for (AActor* Actor : ManagedActors)
    {
        if (!Actor)
            continue;
        
        UPerf_LODManager* LODComponent = Actor->FindComponentByClass<UPerf_LODManager>();
        if (LODComponent && LODComponent != this)
        {
            LODComponent->UpdateLODLevel();
        }
    }
}