#include "Perf_LODManager.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/Engine.h"
#include "TimerManager.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"

UPerf_LODManager::UPerf_LODManager()
{
    LastUpdateTime = 0.0f;
    UpdateFrequency = 0.1f;
}

void UPerf_LODManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("Perf_LODManager: Initialized"));
    
    // Start the LOD update timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            LODUpdateTimer,
            this,
            &UPerf_LODManager::PerformLODUpdate,
            UpdateFrequency,
            true
        );
    }
}

void UPerf_LODManager::Deinitialize()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(LODUpdateTimer);
    }
    
    RegisteredActors.Empty();
    
    Super::Deinitialize();
    UE_LOG(LogTemp, Log, TEXT("Perf_LODManager: Deinitialized"));
}

void UPerf_LODManager::RegisterActor(AActor* Actor)
{
    if (!Actor)
    {
        return;
    }

    // Check if already registered
    for (const TWeakObjectPtr<AActor>& WeakActor : RegisteredActors)
    {
        if (WeakActor.Get() == Actor)
        {
            return; // Already registered
        }
    }

    RegisteredActors.Add(Actor);
    UE_LOG(LogTemp, Log, TEXT("Perf_LODManager: Registered actor %s"), *Actor->GetName());
}

void UPerf_LODManager::UnregisterActor(AActor* Actor)
{
    if (!Actor)
    {
        return;
    }

    RegisteredActors.RemoveAll([Actor](const TWeakObjectPtr<AActor>& WeakActor)
    {
        return WeakActor.Get() == Actor;
    });
    
    UE_LOG(LogTemp, Log, TEXT("Perf_LODManager: Unregistered actor %s"), *Actor->GetName());
}

void UPerf_LODManager::UpdateLODForActor(AActor* Actor, APawn* ViewerPawn)
{
    if (!Actor || !ViewerPawn)
    {
        return;
    }

    float Distance = GetDistanceToViewer(Actor, ViewerPawn);
    EPerf_LODLevel LODLevel = GetLODLevelForDistance(Distance);

    // Apply frustum culling if enabled
    if (LODSettings.bEnableFrustumCulling && !IsActorInViewFrustum(Actor, ViewerPawn))
    {
        LODLevel = EPerf_LODLevel::LOD_Culled;
    }

    // Apply LOD to all mesh components
    TArray<UStaticMeshComponent*> StaticMeshes;
    Actor->GetComponents<UStaticMeshComponent>(StaticMeshes);
    for (UStaticMeshComponent* MeshComp : StaticMeshes)
    {
        ApplyLODToStaticMesh(MeshComp, LODLevel);
    }

    TArray<USkeletalMeshComponent*> SkeletalMeshes;
    Actor->GetComponents<USkeletalMeshComponent>(SkeletalMeshes);
    for (USkeletalMeshComponent* MeshComp : SkeletalMeshes)
    {
        ApplyLODToSkeletalMesh(MeshComp, LODLevel);
    }
}

void UPerf_LODManager::UpdateAllLODs()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Get the first player pawn as viewer
    APawn* ViewerPawn = nullptr;
    if (APlayerController* PC = World->GetFirstPlayerController())
    {
        ViewerPawn = PC->GetPawn();
    }

    if (!ViewerPawn)
    {
        return;
    }

    // Clean up invalid weak pointers
    RegisteredActors.RemoveAll([](const TWeakObjectPtr<AActor>& WeakActor)
    {
        return !WeakActor.IsValid();
    });

    // Update LOD for all registered actors
    for (const TWeakObjectPtr<AActor>& WeakActor : RegisteredActors)
    {
        if (AActor* Actor = WeakActor.Get())
        {
            UpdateLODForActor(Actor, ViewerPawn);
        }
    }
}

EPerf_LODLevel UPerf_LODManager::GetLODLevelForDistance(float Distance) const
{
    if (!LODSettings.bEnableDistanceCulling)
    {
        return EPerf_LODLevel::LOD_High;
    }

    if (Distance > LODSettings.CullDistance)
    {
        return EPerf_LODLevel::LOD_Culled;
    }
    else if (Distance > LODSettings.LowQualityDistance)
    {
        return EPerf_LODLevel::LOD_Low;
    }
    else if (Distance > LODSettings.MediumQualityDistance)
    {
        return EPerf_LODLevel::LOD_Medium;
    }
    else
    {
        return EPerf_LODLevel::LOD_High;
    }
}

void UPerf_LODManager::SetLODSettings(const FPerf_LODSettings& NewSettings)
{
    LODSettings = NewSettings;
    UE_LOG(LogTemp, Log, TEXT("Perf_LODManager: LOD settings updated"));
}

void UPerf_LODManager::ApplyLODToStaticMesh(UStaticMeshComponent* MeshComp, EPerf_LODLevel LODLevel)
{
    if (!MeshComp)
    {
        return;
    }

    switch (LODLevel)
    {
        case EPerf_LODLevel::LOD_High:
            MeshComp->SetForcedLodModel(0);
            MeshComp->SetVisibility(true);
            MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            break;
        case EPerf_LODLevel::LOD_Medium:
            MeshComp->SetForcedLodModel(1);
            MeshComp->SetVisibility(true);
            MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
            break;
        case EPerf_LODLevel::LOD_Low:
            MeshComp->SetForcedLodModel(2);
            MeshComp->SetVisibility(true);
            MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            break;
        case EPerf_LODLevel::LOD_Culled:
            MeshComp->SetVisibility(false);
            MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            break;
    }
}

void UPerf_LODManager::ApplyLODToSkeletalMesh(USkeletalMeshComponent* MeshComp, EPerf_LODLevel LODLevel)
{
    if (!MeshComp)
    {
        return;
    }

    switch (LODLevel)
    {
        case EPerf_LODLevel::LOD_High:
            MeshComp->SetForcedLOD(0);
            MeshComp->SetVisibility(true);
            MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            break;
        case EPerf_LODLevel::LOD_Medium:
            MeshComp->SetForcedLOD(1);
            MeshComp->SetVisibility(true);
            MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
            break;
        case EPerf_LODLevel::LOD_Low:
            MeshComp->SetForcedLOD(2);
            MeshComp->SetVisibility(true);
            MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            break;
        case EPerf_LODLevel::LOD_Culled:
            MeshComp->SetVisibility(false);
            MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            break;
    }
}

bool UPerf_LODManager::IsActorInViewFrustum(AActor* Actor, APawn* ViewerPawn) const
{
    if (!Actor || !ViewerPawn)
    {
        return false;
    }

    // Simple frustum check - in a real implementation, you'd use the camera's frustum
    // For now, just return true as a placeholder
    return true;
}

float UPerf_LODManager::GetDistanceToViewer(AActor* Actor, APawn* ViewerPawn) const
{
    if (!Actor || !ViewerPawn)
    {
        return 0.0f;
    }

    return FVector::Dist(Actor->GetActorLocation(), ViewerPawn->GetActorLocation());
}

void UPerf_LODManager::PerformLODUpdate()
{
    UpdateAllLODs();
}