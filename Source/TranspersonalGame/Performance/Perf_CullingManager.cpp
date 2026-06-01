#include "Perf_CullingManager.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/Engine.h"
#include "DrawDebugHelpers.h"

UPerf_CullingManager::UPerf_CullingManager()
{
    bCullingEnabled = true;
    CullingUpdateInterval = 0.1f; // Update culling 10 times per second
    LastCullingUpdateTime = 0.0f;
    ActorsProcessedPerFrame = 50; // Process 50 actors per frame to spread load
    CurrentProcessingIndex = 0;
}

void UPerf_CullingManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("Performance Culling Manager initialized"));
    
    // Set default culling settings
    CullingSettings = FPerf_CullingSettings();
}

void UPerf_CullingManager::Deinitialize()
{
    // Restore visibility for all registered actors
    for (FPerf_ActorCullingData& ActorData : CullingData)
    {
        if (ActorData.Actor.IsValid())
        {
            SetActorVisibility(ActorData.Actor.Get(), true);
        }
    }
    
    CullingData.Empty();
    Super::Deinitialize();
}

void UPerf_CullingManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bCullingEnabled)
    {
        UpdateCulling(DeltaTime);
    }
}

void UPerf_CullingManager::RegisterActorForCulling(AActor* Actor)
{
    if (!IsValid(Actor))
    {
        return;
    }
    
    // Check if actor is already registered
    for (const FPerf_ActorCullingData& ExistingData : CullingData)
    {
        if (ExistingData.Actor == Actor)
        {
            return; // Already registered
        }
    }
    
    // Add new culling data
    FPerf_ActorCullingData NewData;
    NewData.Actor = Actor;
    NewData.DistanceToPlayer = 0.0f;
    NewData.bIsVisible = true;
    NewData.bIsCulled = false;
    NewData.LastUpdateTime = GetWorld()->GetTimeSeconds();
    
    CullingData.Add(NewData);
    
    UE_LOG(LogTemp, Log, TEXT("Registered actor for culling: %s"), *Actor->GetName());
}

void UPerf_CullingManager::UnregisterActorFromCulling(AActor* Actor)
{
    if (!IsValid(Actor))
    {
        return;
    }
    
    for (int32 i = CullingData.Num() - 1; i >= 0; --i)
    {
        if (CullingData[i].Actor == Actor)
        {
            // Restore visibility before removing
            SetActorVisibility(Actor, true);
            CullingData.RemoveAt(i);
            UE_LOG(LogTemp, Log, TEXT("Unregistered actor from culling: %s"), *Actor->GetName());
            break;
        }
    }
}

void UPerf_CullingManager::SetCullingSettings(const FPerf_CullingSettings& NewSettings)
{
    CullingSettings = NewSettings;
    UE_LOG(LogTemp, Log, TEXT("Culling settings updated"));
}

void UPerf_CullingManager::EnableCulling(bool bEnable)
{
    bCullingEnabled = bEnable;
    
    if (!bEnable)
    {
        // Restore visibility for all actors when disabling culling
        for (FPerf_ActorCullingData& ActorData : CullingData)
        {
            if (ActorData.Actor.IsValid())
            {
                SetActorVisibility(ActorData.Actor.Get(), true);
                ActorData.bIsCulled = false;
            }
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Culling %s"), bEnable ? TEXT("enabled") : TEXT("disabled"));
}

int32 UPerf_CullingManager::GetVisibleActorCount() const
{
    int32 Count = 0;
    for (const FPerf_ActorCullingData& ActorData : CullingData)
    {
        if (ActorData.Actor.IsValid() && ActorData.bIsVisible && !ActorData.bIsCulled)
        {
            Count++;
        }
    }
    return Count;
}

int32 UPerf_CullingManager::GetCulledActorCount() const
{
    int32 Count = 0;
    for (const FPerf_ActorCullingData& ActorData : CullingData)
    {
        if (ActorData.Actor.IsValid() && ActorData.bIsCulled)
        {
            Count++;
        }
    }
    return Count;
}

void UPerf_CullingManager::ForceUpdateCulling()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    for (FPerf_ActorCullingData& ActorData : CullingData)
    {
        if (ActorData.Actor.IsValid())
        {
            UpdateActorCulling(ActorData);
            ActorData.LastUpdateTime = CurrentTime;
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Forced culling update completed"));
}

void UPerf_CullingManager::LogCullingStats()
{
    int32 TotalActors = CullingData.Num();
    int32 VisibleActors = GetVisibleActorCount();
    int32 CulledActors = GetCulledActorCount();
    
    UE_LOG(LogTemp, Warning, TEXT("=== CULLING STATS ==="));
    UE_LOG(LogTemp, Warning, TEXT("Total registered actors: %d"), TotalActors);
    UE_LOG(LogTemp, Warning, TEXT("Visible actors: %d"), VisibleActors);
    UE_LOG(LogTemp, Warning, TEXT("Culled actors: %d"), CulledActors);
    UE_LOG(LogTemp, Warning, TEXT("Culling enabled: %s"), bCullingEnabled ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Warning, TEXT("Max draw distance: %.2f"), CullingSettings.MaxDrawDistance);
    UE_LOG(LogTemp, Warning, TEXT("Max visible actors: %d"), CullingSettings.MaxVisibleActors);
}

void UPerf_CullingManager::UpdateCulling(float DeltaTime)
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    if (CurrentTime - LastCullingUpdateTime < CullingUpdateInterval)
    {
        return;
    }
    
    LastCullingUpdateTime = CurrentTime;
    
    // Process a subset of actors each frame to spread the load
    int32 ActorsToProcess = FMath::Min(ActorsProcessedPerFrame, CullingData.Num());
    
    for (int32 i = 0; i < ActorsToProcess && CullingData.IsValidIndex(CurrentProcessingIndex); ++i)
    {
        FPerf_ActorCullingData& ActorData = CullingData[CurrentProcessingIndex];
        
        if (ActorData.Actor.IsValid())
        {
            UpdateActorCulling(ActorData);
            ActorData.LastUpdateTime = CurrentTime;
        }
        else
        {
            // Remove invalid actors
            CullingData.RemoveAt(CurrentProcessingIndex);
            CurrentProcessingIndex = FMath::Max(0, CurrentProcessingIndex - 1);
        }
        
        CurrentProcessingIndex = (CurrentProcessingIndex + 1) % FMath::Max(1, CullingData.Num());
    }
}

void UPerf_CullingManager::UpdateActorCulling(FPerf_ActorCullingData& ActorData)
{
    AActor* Actor = ActorData.Actor.Get();
    if (!IsValid(Actor))
    {
        return;
    }
    
    FVector PlayerLocation = GetPlayerLocation();
    FVector ActorLocation = Actor->GetActorLocation();
    float Distance = FVector::Dist(PlayerLocation, ActorLocation);
    
    ActorData.DistanceToPlayer = Distance;
    
    bool bShouldCull = ShouldCullActor(Actor, Distance);
    
    if (bShouldCull != ActorData.bIsCulled)
    {
        ActorData.bIsCulled = bShouldCull;
        SetActorVisibility(Actor, !bShouldCull);
    }
}

bool UPerf_CullingManager::ShouldCullActor(AActor* Actor, float DistanceToPlayer)
{
    if (!CullingSettings.bEnableDistanceCulling)
    {
        return false;
    }
    
    // Distance culling
    if (DistanceToPlayer > CullingSettings.MaxDrawDistance)
    {
        return true;
    }
    
    // Check if we're over the visible actor limit
    int32 VisibleCount = GetVisibleActorCount();
    if (VisibleCount >= CullingSettings.MaxVisibleActors)
    {
        // Cull actors that are further away when we hit the limit
        return DistanceToPlayer > CullingSettings.FrustumCullingDistance;
    }
    
    return false;
}

void UPerf_CullingManager::SetActorVisibility(AActor* Actor, bool bVisible)
{
    if (!IsValid(Actor))
    {
        return;
    }
    
    Actor->SetActorHiddenInGame(!bVisible);
    
    // Also set visibility on all primitive components
    TArray<UPrimitiveComponent*> PrimitiveComponents;
    Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
    
    for (UPrimitiveComponent* Component : PrimitiveComponents)
    {
        if (IsValid(Component))
        {
            Component->SetVisibility(bVisible);
        }
    }
}

FVector UPerf_CullingManager::GetPlayerLocation()
{
    if (UWorld* World = GetWorld())
    {
        if (APlayerController* PlayerController = World->GetFirstPlayerController())
        {
            if (APawn* PlayerPawn = PlayerController->GetPawn())
            {
                return PlayerPawn->GetActorLocation();
            }
        }
    }
    
    return FVector::ZeroVector;
}