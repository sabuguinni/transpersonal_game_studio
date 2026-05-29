#include "Perf_LODManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/StaticMesh.h"

APerf_LODManager::APerf_LODManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.5f; // Update every 0.5 seconds
    
    UpdateInterval = 0.5f;
    bEnableLODManagement = true;
    
    // Set default LOD settings
    LODSettings = FPerf_LODSettings();
    
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
}

void APerf_LODManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("LOD Manager initialized - Auto LOD: %s"), 
           LODSettings.bEnableAutomaticLOD ? TEXT("Enabled") : TEXT("Disabled"));
    
    // Register all existing actors for LOD management
    if (UWorld* World = GetWorld())
    {
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (Actor && Actor != this)
            {
                // Check if actor has mesh components
                TArray<UStaticMeshComponent*> StaticMeshes;
                TArray<USkeletalMeshComponent*> SkeletalMeshes;
                Actor->GetComponents<UStaticMeshComponent>(StaticMeshes);
                Actor->GetComponents<USkeletalMeshComponent>(SkeletalMeshes);
                
                if (StaticMeshes.Num() > 0 || SkeletalMeshes.Num() > 0)
                {
                    RegisterActorForLOD(Actor);
                }
            }
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("LOD Manager registered %d actors for management"), ManagedActors.Num());
}

void APerf_LODManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (!bEnableLODManagement) return;
    
    LastUpdateTime += DeltaTime;
    if (LastUpdateTime >= UpdateInterval)
    {
        UpdateLODForAllActors();
        LastUpdateTime = 0.0f;
    }
}

void APerf_LODManager::UpdateLODForAllActors()
{
    if (!LODSettings.bEnableAutomaticLOD) return;
    
    FVector ViewerLocation = GetViewerLocation();
    
    // Skip update if viewer hasn't moved significantly
    float MovementThreshold = 100.0f; // 1 meter
    if (FVector::Dist(ViewerLocation, LastViewerLocation) < MovementThreshold)
    {
        return;
    }
    
    LastViewerLocation = ViewerLocation;
    
    int32 UpdatedActors = 0;
    for (AActor* Actor : ManagedActors)
    {
        if (IsValid(Actor))
        {
            EPerf_LODLevel LODLevel = CalculateLODLevel(Actor, ViewerLocation);
            SetLODLevel(Actor, LODLevel);
            UpdatedActors++;
        }
    }
    
    // Clean up invalid actors
    ManagedActors.RemoveAll([](AActor* Actor) { return !IsValid(Actor); });
    
    if (UpdatedActors > 0)
    {
        UE_LOG(LogTemp, Verbose, TEXT("LOD Manager updated %d actors"), UpdatedActors);
    }
}

void APerf_LODManager::SetLODLevel(AActor* Actor, EPerf_LODLevel LODLevel)
{
    if (!IsValid(Actor)) return;
    
    // Update static mesh components
    TArray<UStaticMeshComponent*> StaticMeshes;
    Actor->GetComponents<UStaticMeshComponent>(StaticMeshes);
    for (UStaticMeshComponent* MeshComp : StaticMeshes)
    {
        UpdateStaticMeshLOD(MeshComp, LODLevel);
    }
    
    // Update skeletal mesh components
    TArray<USkeletalMeshComponent*> SkeletalMeshes;
    Actor->GetComponents<USkeletalMeshComponent>(SkeletalMeshes);
    for (USkeletalMeshComponent* MeshComp : SkeletalMeshes)
    {
        UpdateSkeletalMeshLOD(MeshComp, LODLevel);
    }
    
    // Handle culling for very distant objects
    bool bShouldCull = (LODLevel == EPerf_LODLevel::VeryLow);
    CullActor(Actor, bShouldCull);
}

EPerf_LODLevel APerf_LODManager::CalculateLODLevel(AActor* Actor, const FVector& ViewerLocation)
{
    if (!IsValid(Actor)) return EPerf_LODLevel::VeryLow;
    
    float Distance = FVector::Dist(ViewerLocation, Actor->GetActorLocation());
    
    if (Distance <= LODSettings.HighQualityDistance)
    {
        return EPerf_LODLevel::High;
    }
    else if (Distance <= LODSettings.MediumQualityDistance)
    {
        return EPerf_LODLevel::Medium;
    }
    else if (Distance <= LODSettings.LowQualityDistance)
    {
        return EPerf_LODLevel::Low;
    }
    else
    {
        return EPerf_LODLevel::VeryLow;
    }
}

void APerf_LODManager::RegisterActorForLOD(AActor* Actor)
{
    if (IsValid(Actor) && !ManagedActors.Contains(Actor))
    {
        ManagedActors.Add(Actor);
        UE_LOG(LogTemp, Verbose, TEXT("Registered actor for LOD management: %s"), *Actor->GetName());
    }
}

void APerf_LODManager::UnregisterActorFromLOD(AActor* Actor)
{
    if (ManagedActors.Contains(Actor))
    {
        ManagedActors.Remove(Actor);
        UE_LOG(LogTemp, Verbose, TEXT("Unregistered actor from LOD management: %s"), 
               IsValid(Actor) ? *Actor->GetName() : TEXT("Invalid Actor"));
    }
}

void APerf_LODManager::SetLODSettings(const FPerf_LODSettings& NewSettings)
{
    LODSettings = NewSettings;
    UE_LOG(LogTemp, Log, TEXT("LOD settings updated - High: %.0f, Medium: %.0f, Low: %.0f, Cull: %.0f"),
           LODSettings.HighQualityDistance, LODSettings.MediumQualityDistance,
           LODSettings.LowQualityDistance, LODSettings.CullingDistance);
}

FPerf_LODSettings APerf_LODManager::GetLODSettings() const
{
    return LODSettings;
}

int32 APerf_LODManager::GetManagedActorCount() const
{
    return ManagedActors.Num();
}

void APerf_LODManager::UpdateStaticMeshLOD(UStaticMeshComponent* MeshComp, EPerf_LODLevel LODLevel)
{
    if (!IsValid(MeshComp)) return;
    
    int32 ForcedLOD = -1; // -1 means automatic LOD
    
    switch (LODLevel)
    {
        case EPerf_LODLevel::High:
            ForcedLOD = 0;
            break;
        case EPerf_LODLevel::Medium:
            ForcedLOD = 1;
            break;
        case EPerf_LODLevel::Low:
            ForcedLOD = 2;
            break;
        case EPerf_LODLevel::VeryLow:
            ForcedLOD = 3;
            break;
    }
    
    MeshComp->SetForcedLodModel(ForcedLOD);
}

void APerf_LODManager::UpdateSkeletalMeshLOD(USkeletalMeshComponent* MeshComp, EPerf_LODLevel LODLevel)
{
    if (!IsValid(MeshComp)) return;
    
    int32 ForcedLOD = -1; // -1 means automatic LOD
    
    switch (LODLevel)
    {
        case EPerf_LODLevel::High:
            ForcedLOD = 0;
            break;
        case EPerf_LODLevel::Medium:
            ForcedLOD = 1;
            break;
        case EPerf_LODLevel::Low:
            ForcedLOD = 2;
            break;
        case EPerf_LODLevel::VeryLow:
            ForcedLOD = 3;
            break;
    }
    
    MeshComp->SetForcedLOD(ForcedLOD);
}

void APerf_LODManager::CullActor(AActor* Actor, bool bShouldCull)
{
    if (!IsValid(Actor) || !LODSettings.bEnableDistanceCulling) return;
    
    Actor->SetActorHiddenInGame(bShouldCull);
    Actor->SetActorEnableCollision(!bShouldCull);
    Actor->SetActorTickEnabled(!bShouldCull);
}

FVector APerf_LODManager::GetViewerLocation() const
{
    if (UWorld* World = GetWorld())
    {
        if (APlayerController* PC = World->GetFirstPlayerController())
        {
            if (APawn* PlayerPawn = PC->GetPawn())
            {
                return PlayerPawn->GetActorLocation();
            }
        }
    }
    
    return FVector::ZeroVector;
}