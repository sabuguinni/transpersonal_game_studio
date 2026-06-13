#include "Perf_LODManager.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/Engine.h"

UPerf_LODManager::UPerf_LODManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;
    
    LODSettings.HighDetailDistance = 1000.0f;
    LODSettings.MediumDetailDistance = 3000.0f;
    LODSettings.LowDetailDistance = 8000.0f;
    LODSettings.CullDistance = 15000.0f;
    LODSettings.bEnableDistanceCulling = true;
    LODSettings.bEnableFrustumCulling = true;
    
    UpdateFrequency = 0.1f;
    TimeSinceLastUpdate = 0.0f;
    bIsEnabled = true;
}

void UPerf_LODManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Auto-register nearby actors on start
    if (UWorld* World = GetWorld())
    {
        APawn* PlayerPawn = GetPlayerPawn();
        if (PlayerPawn)
        {
            FVector PlayerLocation = PlayerPawn->GetActorLocation();
            
            // Find all actors within initial registration range
            for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
            {
                AActor* Actor = *ActorItr;
                if (Actor && Actor != PlayerPawn && Actor != GetOwner())
                {
                    float Distance = FVector::Dist(Actor->GetActorLocation(), PlayerLocation);
                    if (Distance <= LODSettings.CullDistance * 1.5f) // Register slightly beyond cull distance
                    {
                        RegisterActor(Actor);
                    }
                }
            }
        }
    }
}

void UPerf_LODManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bIsEnabled)
        return;
    
    TimeSinceLastUpdate += DeltaTime;
    
    if (TimeSinceLastUpdate >= UpdateFrequency)
    {
        TimeSinceLastUpdate = 0.0f;
        
        APawn* PlayerPawn = GetPlayerPawn();
        if (PlayerPawn)
        {
            FVector PlayerLocation = PlayerPawn->GetActorLocation();
            
            // Update LOD for all registered actors
            for (int32 i = RegisteredActors.Num() - 1; i >= 0; i--)
            {
                FPerf_ActorLODData& ActorData = RegisteredActors[i];
                
                if (!ActorData.Actor.IsValid())
                {
                    RegisteredActors.RemoveAt(i);
                    continue;
                }
                
                float Distance = FVector::Dist(ActorData.Actor->GetActorLocation(), PlayerLocation);
                ActorData.LastDistanceToPlayer = Distance;
                ActorData.LastUpdateTime = GetWorld()->GetTimeSeconds();
                
                UpdateActorLOD(ActorData, Distance);
            }
        }
    }
}

void UPerf_LODManager::RegisterActor(AActor* Actor)
{
    if (!Actor)
        return;
    
    // Check if already registered
    for (const FPerf_ActorLODData& ExistingData : RegisteredActors)
    {
        if (ExistingData.Actor == Actor)
            return;
    }
    
    FPerf_ActorLODData NewData;
    NewData.Actor = Actor;
    NewData.LastDistanceToPlayer = 0.0f;
    NewData.CurrentLODLevel = EPerf_LODLevel::High;
    NewData.bIsVisible = true;
    NewData.LastUpdateTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    
    RegisteredActors.Add(NewData);
}

void UPerf_LODManager::UnregisterActor(AActor* Actor)
{
    if (!Actor)
        return;
    
    for (int32 i = RegisteredActors.Num() - 1; i >= 0; i--)
    {
        if (RegisteredActors[i].Actor == Actor)
        {
            RegisteredActors.RemoveAt(i);
            break;
        }
    }
}

void UPerf_LODManager::UpdateLODForActor(AActor* Actor, float DistanceToPlayer)
{
    if (!Actor)
        return;
    
    // Find the actor data
    FPerf_ActorLODData* ActorData = nullptr;
    for (FPerf_ActorLODData& Data : RegisteredActors)
    {
        if (Data.Actor == Actor)
        {
            ActorData = &Data;
            break;
        }
    }
    
    if (ActorData)
    {
        UpdateActorLOD(*ActorData, DistanceToPlayer);
    }
}

void UPerf_LODManager::SetLODSettings(const FPerf_LODSettings& NewSettings)
{
    LODSettings = NewSettings;
    ForceUpdateAllActors();
}

void UPerf_LODManager::ForceUpdateAllActors()
{
    APawn* PlayerPawn = GetPlayerPawn();
    if (!PlayerPawn)
        return;
    
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    
    for (FPerf_ActorLODData& ActorData : RegisteredActors)
    {
        if (ActorData.Actor.IsValid())
        {
            float Distance = FVector::Dist(ActorData.Actor->GetActorLocation(), PlayerLocation);
            ActorData.LastDistanceToPlayer = Distance;
            UpdateActorLOD(ActorData, Distance);
        }
    }
}

void UPerf_LODManager::SetUpdateFrequency(float NewFrequency)
{
    UpdateFrequency = FMath::Max(0.01f, NewFrequency);
    PrimaryComponentTick.TickInterval = UpdateFrequency;
}

APawn* UPerf_LODManager::GetPlayerPawn() const
{
    if (UWorld* World = GetWorld())
    {
        if (APlayerController* PC = World->GetFirstPlayerController())
        {
            return PC->GetPawn();
        }
    }
    return nullptr;
}

void UPerf_LODManager::UpdateActorLOD(FPerf_ActorLODData& ActorData, float DistanceToPlayer)
{
    if (!ActorData.Actor.IsValid())
        return;
    
    EPerf_LODLevel NewLODLevel = EPerf_LODLevel::High;
    bool bShouldBeVisible = true;
    
    // Determine LOD level based on distance
    if (DistanceToPlayer > LODSettings.CullDistance && LODSettings.bEnableDistanceCulling)
    {
        NewLODLevel = EPerf_LODLevel::Culled;
        bShouldBeVisible = false;
    }
    else if (DistanceToPlayer > LODSettings.LowDetailDistance)
    {
        NewLODLevel = EPerf_LODLevel::Low;
    }
    else if (DistanceToPlayer > LODSettings.MediumDetailDistance)
    {
        NewLODLevel = EPerf_LODLevel::Medium;
    }
    else if (DistanceToPlayer > LODSettings.HighDetailDistance)
    {
        NewLODLevel = EPerf_LODLevel::High;
    }
    else
    {
        NewLODLevel = EPerf_LODLevel::Ultra;
    }
    
    // Apply changes if different from current state
    if (NewLODLevel != ActorData.CurrentLODLevel || bShouldBeVisible != ActorData.bIsVisible)
    {
        ActorData.CurrentLODLevel = NewLODLevel;
        ActorData.bIsVisible = bShouldBeVisible;
        
        SetActorLODLevel(ActorData.Actor.Get(), NewLODLevel);
        ActorData.Actor->SetActorHiddenInGame(!bShouldBeVisible);
    }
}

void UPerf_LODManager::SetActorLODLevel(AActor* Actor, EPerf_LODLevel LODLevel)
{
    if (!Actor)
        return;
    
    // Handle Static Mesh Components
    TArray<UStaticMeshComponent*> StaticMeshComps;
    Actor->GetComponents<UStaticMeshComponent>(StaticMeshComps);
    
    for (UStaticMeshComponent* MeshComp : StaticMeshComps)
    {
        if (MeshComp)
        {
            switch (LODLevel)
            {
                case EPerf_LODLevel::Ultra:
                    MeshComp->SetForcedLodModel(0);
                    MeshComp->SetCastShadow(true);
                    break;
                case EPerf_LODLevel::High:
                    MeshComp->SetForcedLodModel(1);
                    MeshComp->SetCastShadow(true);
                    break;
                case EPerf_LODLevel::Medium:
                    MeshComp->SetForcedLodModel(2);
                    MeshComp->SetCastShadow(false);
                    break;
                case EPerf_LODLevel::Low:
                    MeshComp->SetForcedLodModel(3);
                    MeshComp->SetCastShadow(false);
                    break;
                case EPerf_LODLevel::Culled:
                    // Actor will be hidden, no need to set LOD
                    break;
            }
        }
    }
    
    // Handle Skeletal Mesh Components
    TArray<USkeletalMeshComponent*> SkeletalMeshComps;
    Actor->GetComponents<USkeletalMeshComponent>(SkeletalMeshComps);
    
    for (USkeletalMeshComponent* SkMeshComp : SkeletalMeshComps)
    {
        if (SkMeshComp)
        {
            switch (LODLevel)
            {
                case EPerf_LODLevel::Ultra:
                    SkMeshComp->SetForcedLOD(1);
                    SkMeshComp->SetCastShadow(true);
                    break;
                case EPerf_LODLevel::High:
                    SkMeshComp->SetForcedLOD(2);
                    SkMeshComp->SetCastShadow(true);
                    break;
                case EPerf_LODLevel::Medium:
                    SkMeshComp->SetForcedLOD(3);
                    SkMeshComp->SetCastShadow(false);
                    break;
                case EPerf_LODLevel::Low:
                    SkMeshComp->SetForcedLOD(4);
                    SkMeshComp->SetCastShadow(false);
                    break;
                case EPerf_LODLevel::Culled:
                    // Actor will be hidden, no need to set LOD
                    break;
            }
        }
    }
}

void UPerf_LODManager::CleanupInvalidActors()
{
    for (int32 i = RegisteredActors.Num() - 1; i >= 0; i--)
    {
        if (!RegisteredActors[i].Actor.IsValid())
        {
            RegisteredActors.RemoveAt(i);
        }
    }
}