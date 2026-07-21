#include "Perf_LODManager.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"

UPerf_LODManager::UPerf_LODManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = UpdateFrequency;
    
    // Initialize default LOD zones
    FPerf_LODZone HighDetailZone;
    HighDetailZone.Center = FVector::ZeroVector;
    HighDetailZone.Radius = 1000.0f;
    HighDetailZone.LODLevel = EPerf_LODLevel::High;
    HighDetailZone.MaxActorsInZone = 100;
    LODZones.Add(HighDetailZone);

    FPerf_LODZone MediumDetailZone;
    MediumDetailZone.Center = FVector::ZeroVector;
    MediumDetailZone.Radius = 3000.0f;
    MediumDetailZone.LODLevel = EPerf_LODLevel::Medium;
    MediumDetailZone.MaxActorsInZone = 200;
    LODZones.Add(MediumDetailZone);

    FPerf_LODZone LowDetailZone;
    LowDetailZone.Center = FVector::ZeroVector;
    LowDetailZone.Radius = 6000.0f;
    LowDetailZone.LODLevel = EPerf_LODLevel::Low;
    LowDetailZone.MaxActorsInZone = 300;
    LODZones.Add(LowDetailZone);
}

void UPerf_LODManager::BeginPlay()
{
    Super::BeginPlay();
    InitializeLODZones();
    LastUpdateTime = GetWorld()->GetTimeSeconds();
}

void UPerf_LODManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastUpdateTime >= UpdateFrequency)
    {
        // Process a batch of actors each tick to spread the load
        int32 ActorsProcessed = 0;
        int32 StartIndex = CurrentProcessIndex;
        
        while (ActorsProcessed < MaxActorsToProcess && CurrentProcessIndex < ManagedActors.Num())
        {
            if (ManagedActors[CurrentProcessIndex].IsValid())
            {
                ProcessActorLOD(ManagedActors[CurrentProcessIndex].Get());
                ActorsProcessed++;
            }
            else
            {
                // Remove invalid actors
                ManagedActors.RemoveAt(CurrentProcessIndex);
                CurrentProcessIndex--;
            }
            CurrentProcessIndex++;
        }

        // Reset index if we've processed all actors
        if (CurrentProcessIndex >= ManagedActors.Num())
        {
            CurrentProcessIndex = 0;
            CullDistantActors();
        }

        LastUpdateTime = CurrentTime;
    }
}

EPerf_LODLevel UPerf_LODManager::GetLODLevelForActor(AActor* Actor)
{
    if (!Actor)
    {
        return EPerf_LODLevel::Culled;
    }

    float DistanceToPlayer = GetDistanceToPlayer(Actor);
    
    // Check if actor should be culled
    if (DistanceToPlayer > CullingDistance)
    {
        return EPerf_LODLevel::Culled;
    }

    // Check LOD zones in order (smallest to largest)
    for (const FPerf_LODZone& Zone : LODZones)
    {
        if (IsActorInZone(Actor, Zone))
        {
            return Zone.LODLevel;
        }
    }

    return EPerf_LODLevel::Low;
}

void UPerf_LODManager::UpdateActorLOD(AActor* Actor, EPerf_LODLevel NewLODLevel)
{
    if (!Actor)
    {
        return;
    }

    // Update static mesh components
    TArray<UStaticMeshComponent*> StaticMeshComponents;
    Actor->GetComponents<UStaticMeshComponent>(StaticMeshComponents);
    
    for (UStaticMeshComponent* MeshComp : StaticMeshComponents)
    {
        if (MeshComp)
        {
            switch (NewLODLevel)
            {
                case EPerf_LODLevel::High:
                    MeshComp->SetForcedLodModel(0);
                    MeshComp->SetVisibility(true);
                    break;
                case EPerf_LODLevel::Medium:
                    MeshComp->SetForcedLodModel(1);
                    MeshComp->SetVisibility(true);
                    break;
                case EPerf_LODLevel::Low:
                    MeshComp->SetForcedLodModel(2);
                    MeshComp->SetVisibility(true);
                    break;
                case EPerf_LODLevel::Culled:
                    MeshComp->SetVisibility(false);
                    break;
            }
        }
    }

    // Update skeletal mesh components
    TArray<USkeletalMeshComponent*> SkeletalMeshComponents;
    Actor->GetComponents<USkeletalMeshComponent>(SkeletalMeshComponents);
    
    for (USkeletalMeshComponent* SkelMeshComp : SkeletalMeshComponents)
    {
        if (SkelMeshComp)
        {
            switch (NewLODLevel)
            {
                case EPerf_LODLevel::High:
                    SkelMeshComp->SetForcedLOD(0);
                    SkelMeshComp->SetVisibility(true);
                    break;
                case EPerf_LODLevel::Medium:
                    SkelMeshComp->SetForcedLOD(1);
                    SkelMeshComp->SetVisibility(true);
                    break;
                case EPerf_LODLevel::Low:
                    SkelMeshComp->SetForcedLOD(2);
                    SkelMeshComp->SetVisibility(true);
                    break;
                case EPerf_LODLevel::Culled:
                    SkelMeshComp->SetVisibility(false);
                    break;
            }
        }
    }
}

void UPerf_LODManager::RegisterActorForLOD(AActor* Actor)
{
    if (Actor && !ManagedActors.Contains(Actor))
    {
        ManagedActors.Add(Actor);
    }
}

void UPerf_LODManager::UnregisterActorFromLOD(AActor* Actor)
{
    if (Actor)
    {
        ManagedActors.RemoveAll([Actor](const TWeakObjectPtr<AActor>& WeakActor)
        {
            return WeakActor.Get() == Actor;
        });
    }
}

float UPerf_LODManager::GetCurrentFrameRate() const
{
    if (GEngine && GEngine->GetGameViewport())
    {
        return 1.0f / GetWorld()->GetDeltaSeconds();
    }
    return 0.0f;
}

void UPerf_LODManager::ProcessActorLOD(AActor* Actor)
{
    if (!Actor)
    {
        return;
    }

    EPerf_LODLevel NewLODLevel = GetLODLevelForActor(Actor);
    UpdateActorLOD(Actor, NewLODLevel);
}

void UPerf_LODManager::CullDistantActors()
{
    // Remove actors that are too far away
    ManagedActors.RemoveAll([this](const TWeakObjectPtr<AActor>& WeakActor)
    {
        AActor* Actor = WeakActor.Get();
        if (!Actor)
        {
            return true; // Remove invalid actors
        }

        float Distance = GetDistanceToPlayer(Actor);
        if (Distance > CullingDistance)
        {
            UpdateActorLOD(Actor, EPerf_LODLevel::Culled);
            return true; // Remove from managed list
        }
        return false;
    });
}

void UPerf_LODManager::InitializeLODZones()
{
    // Update LOD zone centers based on player position
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (PlayerPawn)
    {
        FVector PlayerLocation = PlayerPawn->GetActorLocation();
        for (FPerf_LODZone& Zone : LODZones)
        {
            Zone.Center = PlayerLocation;
        }
    }
}

bool UPerf_LODManager::IsActorInZone(AActor* Actor, const FPerf_LODZone& Zone)
{
    if (!Actor)
    {
        return false;
    }

    float Distance = FVector::Dist(Actor->GetActorLocation(), Zone.Center);
    return Distance <= Zone.Radius;
}

float UPerf_LODManager::GetDistanceToPlayer(AActor* Actor)
{
    if (!Actor)
    {
        return MAX_FLT;
    }

    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn)
    {
        return MAX_FLT;
    }

    return FVector::Dist(Actor->GetActorLocation(), PlayerPawn->GetActorLocation());
}