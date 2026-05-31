#include "Perf_LODManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

void UPerf_LODManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("Performance LOD Manager initialized"));
    
    // Initialize default settings
    LODSettings = FPerf_LODSettings();
    AverageLODUpdateTime = 0.0f;
    LODUpdateTimes.Reserve(60); // Store last 60 update times
    
    // Start periodic LOD updates (every 0.5 seconds)
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(LODUpdateTimer, this, &UPerf_LODManager::PeriodicLODUpdate, 0.5f, true);
    }
}

void UPerf_LODManager::Deinitialize()
{
    // Clear timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(LODUpdateTimer);
    }
    
    // Clear managed actors
    ManagedActors.Empty();
    CulledActors.Empty();
    LODUpdateTimes.Empty();
    
    UE_LOG(LogTemp, Log, TEXT("Performance LOD Manager deinitialized"));
    
    Super::Deinitialize();
}

void UPerf_LODManager::UpdateLODForActor(AActor* Actor, const FVector& PlayerLocation)
{
    if (!Actor || !LODSettings.bEnableAutomaticLOD)
    {
        return;
    }
    
    double StartTime = FPlatformTime::Seconds();
    
    // Calculate distance to player
    float Distance = FVector::Dist(Actor->GetActorLocation(), PlayerLocation);
    
    // Determine LOD level
    EPerf_LODLevel LODLevel = CalculateLODLevel(Distance);
    
    // Handle culling
    if (LODLevel == EPerf_LODLevel::Culled)
    {
        CullActor(Actor);
        return;
    }
    else if (IsActorCulled(Actor))
    {
        UnCullActor(Actor);
    }
    
    // Apply LOD to mesh components
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
    
    // Track performance
    float UpdateTime = (FPlatformTime::Seconds() - StartTime) * 1000.0f; // Convert to ms
    UpdatePerformanceStats(UpdateTime);
}

void UPerf_LODManager::UpdateAllActorLODs(const FVector& PlayerLocation)
{
    // Clean up invalid weak pointers
    ManagedActors.RemoveAll([](const TWeakObjectPtr<AActor>& WeakActor) {
        return !WeakActor.IsValid();
    });
    
    // Update LOD for all managed actors
    for (const TWeakObjectPtr<AActor>& WeakActor : ManagedActors)
    {
        if (AActor* Actor = WeakActor.Get())
        {
            UpdateLODForActor(Actor, PlayerLocation);
        }
    }
}

EPerf_LODLevel UPerf_LODManager::CalculateLODLevel(float Distance) const
{
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
        return EPerf_LODLevel::Low; // Keep visible but lowest detail
    }
    else
    {
        return EPerf_LODLevel::Culled;
    }
}

void UPerf_LODManager::SetLODSettings(const FPerf_LODSettings& NewSettings)
{
    LODSettings = NewSettings;
    UE_LOG(LogTemp, Log, TEXT("LOD Settings updated - High: %.0f, Medium: %.0f, Low: %.0f, Cull: %.0f"), 
           LODSettings.HighDetailDistance, LODSettings.MediumDetailDistance, 
           LODSettings.LowDetailDistance, LODSettings.CullDistance);
}

void UPerf_LODManager::CullActor(AActor* Actor)
{
    if (!Actor || IsActorCulled(Actor))
    {
        return;
    }
    
    // Hide the actor
    Actor->SetActorHiddenInGame(true);
    Actor->SetActorEnableCollision(false);
    
    // Add to culled list
    CulledActors.AddUnique(Actor);
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("Culled actor: %s"), *Actor->GetName());
}

void UPerf_LODManager::UnCullActor(AActor* Actor)
{
    if (!Actor || !IsActorCulled(Actor))
    {
        return;
    }
    
    // Show the actor
    Actor->SetActorHiddenInGame(false);
    Actor->SetActorEnableCollision(true);
    
    // Remove from culled list
    CulledActors.Remove(Actor);
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("Un-culled actor: %s"), *Actor->GetName());
}

bool UPerf_LODManager::IsActorCulled(AActor* Actor) const
{
    if (!Actor)
    {
        return false;
    }
    
    return CulledActors.Contains(Actor);
}

void UPerf_LODManager::ApplyLODToStaticMesh(UStaticMeshComponent* MeshComp, EPerf_LODLevel LODLevel)
{
    if (!MeshComp)
    {
        return;
    }
    
    // Set forced LOD level
    int32 ForcedLODLevel = 0;
    switch (LODLevel)
    {
        case EPerf_LODLevel::High:
            ForcedLODLevel = 0;
            break;
        case EPerf_LODLevel::Medium:
            ForcedLODLevel = 1;
            break;
        case EPerf_LODLevel::Low:
            ForcedLODLevel = 2;
            break;
        default:
            ForcedLODLevel = 0;
            break;
    }
    
    MeshComp->SetForcedLodModel(ForcedLODLevel + 1); // +1 because 0 means no forced LOD
}

void UPerf_LODManager::ApplyLODToSkeletalMesh(USkeletalMeshComponent* MeshComp, EPerf_LODLevel LODLevel)
{
    if (!MeshComp)
    {
        return;
    }
    
    // Set forced LOD level
    int32 ForcedLODLevel = 0;
    switch (LODLevel)
    {
        case EPerf_LODLevel::High:
            ForcedLODLevel = 0;
            break;
        case EPerf_LODLevel::Medium:
            ForcedLODLevel = 1;
            break;
        case EPerf_LODLevel::Low:
            ForcedLODLevel = 2;
            break;
        default:
            ForcedLODLevel = 0;
            break;
    }
    
    MeshComp->SetForcedLOD(ForcedLODLevel + 1); // +1 because 0 means no forced LOD
}

void UPerf_LODManager::RegisterActor(AActor* Actor)
{
    if (!Actor)
    {
        return;
    }
    
    ManagedActors.AddUnique(Actor);
    UE_LOG(LogTemp, VeryVerbose, TEXT("Registered actor for LOD management: %s"), *Actor->GetName());
}

void UPerf_LODManager::UnregisterActor(AActor* Actor)
{
    if (!Actor)
    {
        return;
    }
    
    ManagedActors.Remove(Actor);
    CulledActors.Remove(Actor);
    UE_LOG(LogTemp, VeryVerbose, TEXT("Unregistered actor from LOD management: %s"), *Actor->GetName());
}

void UPerf_LODManager::UpdatePerformanceStats(float UpdateTime)
{
    // Add to update times array
    LODUpdateTimes.Add(UpdateTime);
    
    // Keep only last 60 measurements
    if (LODUpdateTimes.Num() > 60)
    {
        LODUpdateTimes.RemoveAt(0);
    }
    
    // Calculate average
    if (LODUpdateTimes.Num() > 0)
    {
        float Total = 0.0f;
        for (float Time : LODUpdateTimes)
        {
            Total += Time;
        }
        AverageLODUpdateTime = Total / LODUpdateTimes.Num();
    }
}

void UPerf_LODManager::PeriodicLODUpdate()
{
    if (!LODSettings.bEnableAutomaticLOD)
    {
        return;
    }
    
    // Get player location
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    APlayerController* PlayerController = World->GetFirstPlayerController();
    if (!PlayerController || !PlayerController->GetPawn())
    {
        return;
    }
    
    FVector PlayerLocation = PlayerController->GetPawn()->GetActorLocation();
    
    // Update all actor LODs
    UpdateAllActorLODs(PlayerLocation);
}