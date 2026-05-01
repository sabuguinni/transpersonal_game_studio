#include "Perf_LODManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "DrawDebugHelpers.h"

APerf_LODManager::APerf_LODManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f;
    
    // Set default LOD settings
    LODSettings.HighDetailDistance = 2000.0f;
    LODSettings.MediumDetailDistance = 5000.0f;
    LODSettings.LowDetailDistance = 10000.0f;
    LODSettings.CullDistance = 15000.0f;
    
    UpdateFrequency = 0.1f;
    bEnableLODSystem = true;
    MaxActorsPerFrame = 10;
    PerformanceThreshold = 30.0f;
    
    LastUpdateTime = 0.0f;
    CurrentUpdateIndex = 0;
}

void APerf_LODManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Get player pawn reference
    if (UWorld* World = GetWorld())
    {
        if (APlayerController* PC = World->GetFirstPlayerController())
        {
            PlayerPawn = PC->GetPawn();
        }
    }
    
    // Auto-register nearby actors
    RefreshManagedActors();
    
    UE_LOG(LogTemp, Warning, TEXT("LOD Manager initialized with %d managed actors"), ManagedActors.Num());
}

void APerf_LODManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (!bEnableLODSystem)
    {
        return;
    }
    
    LastUpdateTime += DeltaTime;
    
    if (LastUpdateTime >= UpdateFrequency)
    {
        UpdateLODSystem();
        LastUpdateTime = 0.0f;
    }
}

void APerf_LODManager::RegisterActor(AActor* Actor, bool bIsImportant)
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
    NewData.bIsImportantActor = bIsImportant;
    NewData.CurrentLODLevel = EPerf_LODLevel::High;
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

void APerf_LODManager::UpdateActorLOD(AActor* Actor)
{
    if (!Actor)
    {
        return;
    }
    
    EPerf_LODLevel NewLODLevel = CalculateLODLevel(Actor);
    SetLODLevel(Actor, NewLODLevel);
}

EPerf_LODLevel APerf_LODManager::CalculateLODLevel(AActor* Actor) const
{
    if (!Actor || !PlayerPawn.IsValid())
    {
        return EPerf_LODLevel::High;
    }
    
    float Distance = GetDistanceToPlayer(Actor);
    bool bInView = IsActorInView(Actor);
    
    // Adjust distances based on whether actor is in view
    float HighDist = bInView ? LODSettings.HighDetailDistance : LODSettings.HighDetailDistance * 0.5f;
    float MediumDist = bInView ? LODSettings.MediumDetailDistance : LODSettings.MediumDetailDistance * 0.7f;
    float LowDist = bInView ? LODSettings.LowDetailDistance : LODSettings.LowDetailDistance * 0.8f;
    float CullDist = LODSettings.CullDistance;
    
    if (Distance > CullDist)
    {
        return EPerf_LODLevel::Culled;
    }
    else if (Distance > LowDist)
    {
        return EPerf_LODLevel::Low;
    }
    else if (Distance > MediumDist)
    {
        return EPerf_LODLevel::Medium;
    }
    else
    {
        return EPerf_LODLevel::High;
    }
}

void APerf_LODManager::SetLODLevel(AActor* Actor, EPerf_LODLevel LODLevel)
{
    if (!Actor)
    {
        return;
    }
    
    // Update managed actor data
    for (FPerf_ActorLODData& Data : ManagedActors)
    {
        if (Data.Actor == Actor)
        {
            Data.CurrentLODLevel = LODLevel;
            Data.LastUpdateTime = GetWorld()->GetTimeSeconds();
            break;
        }
    }
    
    // Apply LOD to mesh components
    TArray<UStaticMeshComponent*> StaticMeshComps;
    Actor->GetComponents<UStaticMeshComponent>(StaticMeshComps);
    
    for (UStaticMeshComponent* MeshComp : StaticMeshComps)
    {
        ApplyLODToStaticMesh(MeshComp, LODLevel);
    }
    
    TArray<USkeletalMeshComponent*> SkeletalMeshComps;
    Actor->GetComponents<USkeletalMeshComponent>(SkeletalMeshComps);
    
    for (USkeletalMeshComponent* MeshComp : SkeletalMeshComps)
    {
        ApplyLODToSkeletalMesh(MeshComp, LODLevel);
    }
    
    // Handle visibility for culled actors
    if (LODLevel == EPerf_LODLevel::Culled)
    {
        Actor->SetActorHiddenInGame(true);
    }
    else
    {
        Actor->SetActorHiddenInGame(false);
    }
}

float APerf_LODManager::GetCurrentFPS() const
{
    if (UWorld* World = GetWorld())
    {
        return 1.0f / World->GetDeltaSeconds();
    }
    return 60.0f;
}

void APerf_LODManager::OptimizePerformance()
{
    float CurrentFPS = GetCurrentFPS();
    
    if (CurrentFPS < PerformanceThreshold)
    {
        // Aggressive LOD optimization when performance is poor
        for (FPerf_ActorLODData& Data : ManagedActors)
        {
            if (Data.Actor.IsValid() && !Data.bIsImportantActor)
            {
                EPerf_LODLevel CurrentLevel = Data.CurrentLODLevel;
                EPerf_LODLevel NewLevel = CurrentLevel;
                
                switch (CurrentLevel)
                {
                case EPerf_LODLevel::High:
                    NewLevel = EPerf_LODLevel::Medium;
                    break;
                case EPerf_LODLevel::Medium:
                    NewLevel = EPerf_LODLevel::Low;
                    break;
                case EPerf_LODLevel::Low:
                    NewLevel = EPerf_LODLevel::Culled;
                    break;
                default:
                    break;
                }
                
                if (NewLevel != CurrentLevel)
                {
                    SetLODLevel(Data.Actor.Get(), NewLevel);
                }
            }
        }
        
        UE_LOG(LogTemp, Warning, TEXT("Performance optimization applied. FPS: %.1f"), CurrentFPS);
    }
}

void APerf_LODManager::RefreshManagedActors()
{
    if (UWorld* World = GetWorld())
    {
        ManagedActors.Empty();
        
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            
            if (!Actor || Actor == this)
            {
                continue;
            }
            
            // Register actors with mesh components
            bool bHasMeshComponent = false;
            
            if (Actor->FindComponentByClass<UStaticMeshComponent>() ||
                Actor->FindComponentByClass<USkeletalMeshComponent>())
            {
                bHasMeshComponent = true;
            }
            
            if (bHasMeshComponent)
            {
                bool bIsImportant = Actor->GetName().Contains(TEXT("Dinosaur")) ||
                                   Actor->GetName().Contains(TEXT("Player")) ||
                                   Actor->GetName().Contains(TEXT("Important"));
                
                RegisterActor(Actor, bIsImportant);
            }
        }
        
        UE_LOG(LogTemp, Log, TEXT("Refreshed managed actors: %d total"), ManagedActors.Num());
    }
}

void APerf_LODManager::UpdateLODSystem()
{
    if (ManagedActors.Num() == 0)
    {
        return;
    }
    
    // Update a subset of actors each frame to spread the load
    int32 ActorsToUpdate = FMath::Min(MaxActorsPerFrame, ManagedActors.Num());
    
    for (int32 i = 0; i < ActorsToUpdate; ++i)
    {
        int32 Index = (CurrentUpdateIndex + i) % ManagedActors.Num();
        FPerf_ActorLODData& Data = ManagedActors[Index];
        
        if (Data.Actor.IsValid())
        {
            UpdateActorLOD(Data.Actor.Get());
        }
        else
        {
            // Remove invalid actors
            ManagedActors.RemoveAt(Index);
            if (Index < CurrentUpdateIndex)
            {
                CurrentUpdateIndex--;
            }
            i--; // Adjust loop counter
            ActorsToUpdate = FMath::Min(ActorsToUpdate, ManagedActors.Num());
        }
    }
    
    CurrentUpdateIndex = (CurrentUpdateIndex + ActorsToUpdate) % FMath::Max(1, ManagedActors.Num());
}

void APerf_LODManager::ApplyLODToStaticMesh(UStaticMeshComponent* MeshComp, EPerf_LODLevel LODLevel)
{
    if (!MeshComp)
    {
        return;
    }
    
    switch (LODLevel)
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

void APerf_LODManager::ApplyLODToSkeletalMesh(USkeletalMeshComponent* MeshComp, EPerf_LODLevel LODLevel)
{
    if (!MeshComp)
    {
        return;
    }
    
    switch (LODLevel)
    {
    case EPerf_LODLevel::High:
        MeshComp->SetForcedLOD(0);
        MeshComp->SetVisibility(true);
        break;
    case EPerf_LODLevel::Medium:
        MeshComp->SetForcedLOD(1);
        MeshComp->SetVisibility(true);
        break;
    case EPerf_LODLevel::Low:
        MeshComp->SetForcedLOD(2);
        MeshComp->SetVisibility(true);
        break;
    case EPerf_LODLevel::Culled:
        MeshComp->SetVisibility(false);
        break;
    }
}

float APerf_LODManager::GetDistanceToPlayer(AActor* Actor) const
{
    if (!Actor || !PlayerPawn.IsValid())
    {
        return 99999.0f;
    }
    
    return FVector::Dist(Actor->GetActorLocation(), PlayerPawn->GetActorLocation());
}

bool APerf_LODManager::IsActorInView(AActor* Actor) const
{
    if (!Actor || !PlayerPawn.IsValid())
    {
        return false;
    }
    
    // Simple frustum check - in a real implementation you'd use the camera's frustum
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    FVector PlayerForward = PlayerPawn->GetActorForwardVector();
    FVector ToActor = (Actor->GetActorLocation() - PlayerLocation).GetSafeNormal();
    
    float DotProduct = FVector::DotProduct(PlayerForward, ToActor);
    
    // Actor is in view if it's in front of the player (dot product > 0)
    // and within a reasonable field of view (dot product > -0.5 for ~120 degree FOV)
    return DotProduct > -0.5f;
}