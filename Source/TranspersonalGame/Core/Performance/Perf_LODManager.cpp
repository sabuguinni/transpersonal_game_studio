#include "Perf_LODManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

UPerf_LODManager::UPerf_LODManager()
{
    LODSettings = FPerf_LODSettings();
    bEnabled = true;
}

void UPerf_LODManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("LOD Manager initialized"));
    
    // Start the LOD update timer
    StartLODUpdateTimer();
    
    // Auto-register existing actors
    if (GetWorld())
    {
        FTimerHandle DelayedRegistrationTimer;
        GetWorld()->GetTimerManager().SetTimer(DelayedRegistrationTimer, this, &UPerf_LODManager::AutoRegisterAllActors, 1.0f, false);
    }
}

void UPerf_LODManager::Deinitialize()
{
    StopLODUpdateTimer();
    ManagedActors.Empty();
    
    Super::Deinitialize();
}

void UPerf_LODManager::UpdateLODSystem()
{
    if (!bEnabled || !GetWorld())
    {
        return;
    }

    APawn* PlayerPawn = GetPlayerPawn();
    if (!PlayerPawn)
    {
        return;
    }

    // Update LOD for all managed actors
    for (int32 i = ManagedActors.Num() - 1; i >= 0; --i)
    {
        FPerf_ActorLODData& ActorData = ManagedActors[i];
        
        // Remove invalid actors
        if (!ActorData.Actor.IsValid())
        {
            ManagedActors.RemoveAt(i);
            continue;
        }

        UpdateActorLOD(ActorData);
    }

    // Enforce maximum visible actor limit
    if (GetVisibleActorCount() > LODSettings.MaxVisibleActors)
    {
        // Sort by distance and cull furthest actors
        ManagedActors.Sort([](const FPerf_ActorLODData& A, const FPerf_ActorLODData& B) {
            return A.DistanceToPlayer < B.DistanceToPlayer;
        });

        int32 VisibleCount = 0;
        for (FPerf_ActorLODData& ActorData : ManagedActors)
        {
            if (VisibleCount < LODSettings.MaxVisibleActors && !ActorData.bIsCulled)
            {
                VisibleCount++;
            }
            else if (!ActorData.bIsCulled)
            {
                CullActor(ActorData.Actor.Get(), true);
            }
        }
    }
}

void UPerf_LODManager::RegisterActor(AActor* Actor)
{
    if (!Actor || !IsValid(Actor))
    {
        return;
    }

    // Check if already registered
    for (const FPerf_ActorLODData& ActorData : ManagedActors)
    {
        if (ActorData.Actor == Actor)
        {
            return;
        }
    }

    // Add new actor
    FPerf_ActorLODData NewActorData;
    NewActorData.Actor = Actor;
    NewActorData.DistanceToPlayer = CalculateDistanceToPlayer(Actor);
    NewActorData.CurrentLODLevel = 0;
    NewActorData.bIsVisible = true;
    NewActorData.bIsCulled = false;

    ManagedActors.Add(NewActorData);
    
    UE_LOG(LogTemp, Log, TEXT("Registered actor for LOD management: %s"), *Actor->GetName());
}

void UPerf_LODManager::UnregisterActor(AActor* Actor)
{
    if (!Actor)
    {
        return;
    }

    for (int32 i = ManagedActors.Num() - 1; i >= 0; --i)
    {
        if (ManagedActors[i].Actor == Actor)
        {
            ManagedActors.RemoveAt(i);
            UE_LOG(LogTemp, Log, TEXT("Unregistered actor from LOD management: %s"), *Actor->GetName());
            break;
        }
    }
}

void UPerf_LODManager::SetLODSettings(const FPerf_LODSettings& NewSettings)
{
    LODSettings = NewSettings;
    
    // Restart timer with new frequency
    if (bEnabled)
    {
        StopLODUpdateTimer();
        StartLODUpdateTimer();
    }
}

void UPerf_LODManager::SetEnabled(bool bNewEnabled)
{
    bEnabled = bNewEnabled;
    
    if (bEnabled)
    {
        StartLODUpdateTimer();
    }
    else
    {
        StopLODUpdateTimer();
        
        // Reset all actors to highest LOD
        for (FPerf_ActorLODData& ActorData : ManagedActors)
        {
            if (ActorData.Actor.IsValid())
            {
                SetActorLODLevel(ActorData.Actor.Get(), 0);
                CullActor(ActorData.Actor.Get(), false);
            }
        }
    }
}

void UPerf_LODManager::SetActorLODLevel(AActor* Actor, int32 LODLevel)
{
    if (!Actor || !IsValid(Actor))
    {
        return;
    }

    // Apply LOD to static mesh components
    TArray<UStaticMeshComponent*> StaticMeshComponents;
    Actor->GetComponents<UStaticMeshComponent>(StaticMeshComponents);
    
    for (UStaticMeshComponent* MeshComp : StaticMeshComponents)
    {
        ApplyLODToStaticMesh(MeshComp, LODLevel);
    }

    // Apply LOD to skeletal mesh components
    TArray<USkeletalMeshComponent*> SkeletalMeshComponents;
    Actor->GetComponents<USkeletalMeshComponent>(SkeletalMeshComponents);
    
    for (USkeletalMeshComponent* MeshComp : SkeletalMeshComponents)
    {
        ApplyLODToSkeletalMesh(MeshComp, LODLevel);
    }

    // Update actor data
    for (FPerf_ActorLODData& ActorData : ManagedActors)
    {
        if (ActorData.Actor == Actor)
        {
            ActorData.CurrentLODLevel = LODLevel;
            break;
        }
    }
}

int32 UPerf_LODManager::GetActorLODLevel(AActor* Actor) const
{
    if (!Actor)
    {
        return 0;
    }

    for (const FPerf_ActorLODData& ActorData : ManagedActors)
    {
        if (ActorData.Actor == Actor)
        {
            return ActorData.CurrentLODLevel;
        }
    }

    return 0;
}

void UPerf_LODManager::CullActor(AActor* Actor, bool bShouldCull)
{
    if (!Actor || !IsValid(Actor))
    {
        return;
    }

    Actor->SetActorHiddenInGame(bShouldCull);
    Actor->SetActorEnableCollision(!bShouldCull);

    // Update actor data
    for (FPerf_ActorLODData& ActorData : ManagedActors)
    {
        if (ActorData.Actor == Actor)
        {
            ActorData.bIsCulled = bShouldCull;
            ActorData.bIsVisible = !bShouldCull;
            break;
        }
    }
}

int32 UPerf_LODManager::GetVisibleActorCount() const
{
    int32 VisibleCount = 0;
    for (const FPerf_ActorLODData& ActorData : ManagedActors)
    {
        if (ActorData.bIsVisible && !ActorData.bIsCulled)
        {
            VisibleCount++;
        }
    }
    return VisibleCount;
}

int32 UPerf_LODManager::GetCulledActorCount() const
{
    int32 CulledCount = 0;
    for (const FPerf_ActorLODData& ActorData : ManagedActors)
    {
        if (ActorData.bIsCulled)
        {
            CulledCount++;
        }
    }
    return CulledCount;
}

TArray<FPerf_ActorLODData> UPerf_LODManager::GetLODStatistics() const
{
    return ManagedActors;
}

void UPerf_LODManager::AutoRegisterAllActors()
{
    if (!GetWorld())
    {
        return;
    }

    for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && IsValid(Actor))
        {
            // Skip essential actors
            FString ActorLabel = Actor->GetActorLabel().ToLower();
            if (ActorLabel.Contains(TEXT("playerstart")) || 
                ActorLabel.Contains(TEXT("light")) || 
                ActorLabel.Contains(TEXT("sky")) || 
                ActorLabel.Contains(TEXT("fog")))
            {
                continue;
            }

            RegisterActor(Actor);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Auto-registered %d actors for LOD management"), ManagedActors.Num());
}

void UPerf_LODManager::AutoRegisterActorsOfClass(UClass* ActorClass)
{
    if (!ActorClass || !GetWorld())
    {
        return;
    }

    for (TActorIterator<AActor> ActorItr(GetWorld(), ActorClass); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && IsValid(Actor))
        {
            RegisterActor(Actor);
        }
    }
}

void UPerf_LODManager::UpdateActorLOD(FPerf_ActorLODData& ActorData)
{
    if (!ActorData.Actor.IsValid())
    {
        return;
    }

    AActor* Actor = ActorData.Actor.Get();
    ActorData.DistanceToPlayer = CalculateDistanceToPlayer(Actor);
    
    // Determine if actor should be culled
    if (ActorData.DistanceToPlayer > LODSettings.CullDistance)
    {
        if (!ActorData.bIsCulled)
        {
            CullActor(Actor, true);
        }
        return;
    }
    else if (ActorData.bIsCulled)
    {
        CullActor(Actor, false);
    }

    // Determine LOD level
    int32 NewLODLevel = DetermineLODLevel(ActorData.DistanceToPlayer);
    if (NewLODLevel != ActorData.CurrentLODLevel)
    {
        SetActorLODLevel(Actor, NewLODLevel);
    }
}

float UPerf_LODManager::CalculateDistanceToPlayer(AActor* Actor) const
{
    if (!Actor)
    {
        return 0.0f;
    }

    APawn* PlayerPawn = GetPlayerPawn();
    if (!PlayerPawn)
    {
        return 0.0f;
    }

    return FVector::Dist(Actor->GetActorLocation(), PlayerPawn->GetActorLocation());
}

int32 UPerf_LODManager::DetermineLODLevel(float Distance) const
{
    if (Distance <= LODSettings.HighQualityDistance)
    {
        return 0; // Highest quality
    }
    else if (Distance <= LODSettings.MediumQualityDistance)
    {
        return 1; // Medium quality
    }
    else if (Distance <= LODSettings.LowQualityDistance)
    {
        return 2; // Low quality
    }
    else
    {
        return 3; // Lowest quality
    }
}

void UPerf_LODManager::ApplyLODToStaticMesh(UStaticMeshComponent* MeshComp, int32 LODLevel)
{
    if (!MeshComp || !IsValid(MeshComp))
    {
        return;
    }

    // Force specific LOD level
    MeshComp->SetForcedLodModel(LODLevel + 1); // UE5 uses 1-based indexing for forced LOD
    
    // Adjust other quality settings based on LOD level
    switch (LODLevel)
    {
    case 0: // High quality
        MeshComp->SetCastShadow(true);
        MeshComp->SetReceivesDecals(true);
        break;
    case 1: // Medium quality
        MeshComp->SetCastShadow(true);
        MeshComp->SetReceivesDecals(false);
        break;
    case 2: // Low quality
        MeshComp->SetCastShadow(false);
        MeshComp->SetReceivesDecals(false);
        break;
    case 3: // Lowest quality
        MeshComp->SetCastShadow(false);
        MeshComp->SetReceivesDecals(false);
        break;
    }
}

void UPerf_LODManager::ApplyLODToSkeletalMesh(USkeletalMeshComponent* MeshComp, int32 LODLevel)
{
    if (!MeshComp || !IsValid(MeshComp))
    {
        return;
    }

    // Force specific LOD level
    MeshComp->SetForcedLOD(LODLevel + 1); // UE5 uses 1-based indexing for forced LOD
    
    // Adjust animation and quality settings
    switch (LODLevel)
    {
    case 0: // High quality
        MeshComp->SetCastShadow(true);
        MeshComp->SetUpdateAnimationInEditor(true);
        break;
    case 1: // Medium quality
        MeshComp->SetCastShadow(true);
        MeshComp->SetUpdateAnimationInEditor(true);
        break;
    case 2: // Low quality
        MeshComp->SetCastShadow(false);
        MeshComp->SetUpdateAnimationInEditor(false);
        break;
    case 3: // Lowest quality
        MeshComp->SetCastShadow(false);
        MeshComp->SetUpdateAnimationInEditor(false);
        break;
    }
}

APawn* UPerf_LODManager::GetPlayerPawn() const
{
    if (!GetWorld())
    {
        return nullptr;
    }

    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (!PC)
    {
        return nullptr;
    }

    return PC->GetPawn();
}

void UPerf_LODManager::StartLODUpdateTimer()
{
    if (!GetWorld())
    {
        return;
    }

    GetWorld()->GetTimerManager().SetTimer(
        LODUpdateTimerHandle,
        this,
        &UPerf_LODManager::UpdateLODSystem,
        LODSettings.UpdateFrequency,
        true
    );
}

void UPerf_LODManager::StopLODUpdateTimer()
{
    if (!GetWorld())
    {
        return;
    }

    GetWorld()->GetTimerManager().ClearTimer(LODUpdateTimerHandle);
}