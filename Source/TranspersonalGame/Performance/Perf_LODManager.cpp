#include "Perf_LODManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Animation/SkeletalMeshActor.h"

APerf_LODManager::APerf_LODManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // Default 10 FPS update
    
    LODSettings = FPerf_LODSettings();
    UpdateFrequency = 0.1f;
    MaxActorsPerFrame = 50;
    CurrentUpdateIndex = 0;
    LastUpdateTime = 0.0f;
}

void APerf_LODManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Performance LOD Manager initialized"));
    
    // Auto-register all static mesh and skeletal mesh actors in the world
    if (UWorld* World = GetWorld())
    {
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (Actor && Actor != this)
            {
                // Check if actor has mesh components
                UStaticMeshComponent* StaticMesh = Actor->FindComponentByClass<UStaticMeshComponent>();
                USkeletalMeshComponent* SkeletalMesh = Actor->FindComponentByClass<USkeletalMeshComponent>();
                
                if (StaticMesh || SkeletalMesh)
                {
                    RegisterActor(Actor);
                }
            }
        }
        
        UE_LOG(LogTemp, Warning, TEXT("LOD Manager registered %d actors"), ManagedActors.Num());
    }
}

void APerf_LODManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Check if it's time to update
    if (CurrentTime - LastUpdateTime < UpdateFrequency)
    {
        return;
    }
    
    LastUpdateTime = CurrentTime;
    
    // Update a batch of actors per frame to spread the load
    int32 ActorsToUpdate = FMath::Min(MaxActorsPerFrame, ManagedActors.Num());
    
    for (int32 i = 0; i < ActorsToUpdate && ManagedActors.IsValidIndex(CurrentUpdateIndex); ++i)
    {
        if (ManagedActors[CurrentUpdateIndex].Actor && IsValid(ManagedActors[CurrentUpdateIndex].Actor))
        {
            UpdateActorLOD(ManagedActors[CurrentUpdateIndex]);
        }
        else
        {
            // Remove invalid actors
            ManagedActors.RemoveAt(CurrentUpdateIndex);
            CurrentUpdateIndex = FMath::Max(0, CurrentUpdateIndex - 1);
        }
        
        CurrentUpdateIndex = (CurrentUpdateIndex + 1) % FMath::Max(1, ManagedActors.Num());
    }
}

void APerf_LODManager::RegisterActor(AActor* Actor)
{
    if (!Actor || Actor == this)
    {
        return;
    }
    
    // Check if already registered
    for (const FPerf_ActorLODData& Data : ManagedActors)
    {
        if (Data.Actor == Actor)
        {
            return;
        }
    }
    
    FPerf_ActorLODData NewData;
    NewData.Actor = Actor;
    NewData.CurrentLOD = EPerf_LODLevel::LOD0_Ultra;
    NewData.LastDistance = 0.0f;
    NewData.LastUpdateTime = GetWorld()->GetTimeSeconds();
    
    ManagedActors.Add(NewData);
    
    UE_LOG(LogTemp, Log, TEXT("Registered actor for LOD management: %s"), *Actor->GetName());
}

void APerf_LODManager::UnregisterActor(AActor* Actor)
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

EPerf_LODLevel APerf_LODManager::GetActorLODLevel(AActor* Actor) const
{
    for (const FPerf_ActorLODData& Data : ManagedActors)
    {
        if (Data.Actor == Actor)
        {
            return Data.CurrentLOD;
        }
    }
    
    return EPerf_LODLevel::LOD0_Ultra;
}

void APerf_LODManager::SetLODSettings(const FPerf_LODSettings& NewSettings)
{
    LODSettings = NewSettings;
    UE_LOG(LogTemp, Warning, TEXT("LOD settings updated"));
}

void APerf_LODManager::ForceUpdateAllActors()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    for (FPerf_ActorLODData& Data : ManagedActors)
    {
        if (Data.Actor && IsValid(Data.Actor))
        {
            UpdateActorLOD(Data);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Force updated all %d managed actors"), ManagedActors.Num());
}

void APerf_LODManager::SetUpdateFrequency(float NewFrequency)
{
    UpdateFrequency = FMath::Clamp(NewFrequency, 0.01f, 1.0f);
    PrimaryActorTick.TickInterval = UpdateFrequency;
}

void APerf_LODManager::UpdateActorLOD(FPerf_ActorLODData& ActorData)
{
    if (!ActorData.Actor || !IsValid(ActorData.Actor))
    {
        return;
    }
    
    float Distance = GetDistanceToPlayer(ActorData.Actor);
    EPerf_LODLevel NewLOD = CalculateLODLevel(Distance);
    
    if (NewLOD != ActorData.CurrentLOD)
    {
        EPerf_LODLevel OldLOD = ActorData.CurrentLOD;
        ActorData.CurrentLOD = NewLOD;
        
        ApplyLODToActor(ActorData.Actor, NewLOD);
        
        // Call Blueprint event
        OnActorLODChanged(ActorData.Actor, OldLOD, NewLOD);
        
        UE_LOG(LogTemp, VeryVerbose, TEXT("Actor %s LOD changed from %d to %d (distance: %.1f)"), 
               *ActorData.Actor->GetName(), (int32)OldLOD, (int32)NewLOD, Distance);
    }
    
    ActorData.LastDistance = Distance;
    ActorData.LastUpdateTime = GetWorld()->GetTimeSeconds();
}

EPerf_LODLevel APerf_LODManager::CalculateLODLevel(float Distance) const
{
    if (Distance <= LODSettings.UltraDistance)
    {
        return EPerf_LODLevel::LOD0_Ultra;
    }
    else if (Distance <= LODSettings.HighDistance)
    {
        return EPerf_LODLevel::LOD1_High;
    }
    else if (Distance <= LODSettings.MediumDistance)
    {
        return EPerf_LODLevel::LOD2_Medium;
    }
    else if (Distance <= LODSettings.LowDistance)
    {
        return EPerf_LODLevel::LOD3_Low;
    }
    else
    {
        return EPerf_LODLevel::LOD4_Culled;
    }
}

void APerf_LODManager::ApplyLODToActor(AActor* Actor, EPerf_LODLevel LODLevel)
{
    if (!Actor)
    {
        return;
    }
    
    // Handle visibility for culled level
    if (LODLevel == EPerf_LODLevel::LOD4_Culled)
    {
        Actor->SetActorHiddenInGame(true);
        Actor->SetActorEnableCollision(false);
        return;
    }
    else
    {
        Actor->SetActorHiddenInGame(false);
        Actor->SetActorEnableCollision(true);
    }
    
    // Apply LOD to static mesh components
    TArray<UStaticMeshComponent*> StaticMeshComponents;
    Actor->GetComponents<UStaticMeshComponent>(StaticMeshComponents);
    
    for (UStaticMeshComponent* MeshComp : StaticMeshComponents)
    {
        ApplyStaticMeshLOD(MeshComp, LODLevel);
    }
    
    // Apply LOD to skeletal mesh components
    TArray<USkeletalMeshComponent*> SkeletalMeshComponents;
    Actor->GetComponents<USkeletalMeshComponent>(SkeletalMeshComponents);
    
    for (USkeletalMeshComponent* MeshComp : SkeletalMeshComponents)
    {
        ApplySkeletalMeshLOD(MeshComp, LODLevel);
    }
}

void APerf_LODManager::ApplyStaticMeshLOD(UStaticMeshComponent* MeshComp, EPerf_LODLevel LODLevel)
{
    if (!MeshComp)
    {
        return;
    }
    
    switch (LODLevel)
    {
        case EPerf_LODLevel::LOD0_Ultra:
            MeshComp->SetForcedLodModel(0);
            MeshComp->SetCastShadow(true);
            break;
            
        case EPerf_LODLevel::LOD1_High:
            MeshComp->SetForcedLodModel(1);
            MeshComp->SetCastShadow(true);
            break;
            
        case EPerf_LODLevel::LOD2_Medium:
            MeshComp->SetForcedLodModel(2);
            MeshComp->SetCastShadow(false);
            break;
            
        case EPerf_LODLevel::LOD3_Low:
            MeshComp->SetForcedLodModel(3);
            MeshComp->SetCastShadow(false);
            break;
            
        case EPerf_LODLevel::LOD4_Culled:
            MeshComp->SetVisibility(false);
            break;
    }
}

void APerf_LODManager::ApplySkeletalMeshLOD(USkeletalMeshComponent* MeshComp, EPerf_LODLevel LODLevel)
{
    if (!MeshComp)
    {
        return;
    }
    
    switch (LODLevel)
    {
        case EPerf_LODLevel::LOD0_Ultra:
            MeshComp->SetForcedLOD(0);
            MeshComp->SetCastShadow(true);
            break;
            
        case EPerf_LODLevel::LOD1_High:
            MeshComp->SetForcedLOD(1);
            MeshComp->SetCastShadow(true);
            break;
            
        case EPerf_LODLevel::LOD2_Medium:
            MeshComp->SetForcedLOD(2);
            MeshComp->SetCastShadow(false);
            break;
            
        case EPerf_LODLevel::LOD3_Low:
            MeshComp->SetForcedLOD(3);
            MeshComp->SetCastShadow(false);
            break;
            
        case EPerf_LODLevel::LOD4_Culled:
            MeshComp->SetVisibility(false);
            break;
    }
}

float APerf_LODManager::GetDistanceToPlayer(AActor* Actor) const
{
    APawn* PlayerPawn = GetPlayerPawn();
    if (!PlayerPawn || !Actor)
    {
        return 10000.0f; // Default far distance
    }
    
    return FVector::Dist(PlayerPawn->GetActorLocation(), Actor->GetActorLocation());
}

APawn* APerf_LODManager::GetPlayerPawn() const
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