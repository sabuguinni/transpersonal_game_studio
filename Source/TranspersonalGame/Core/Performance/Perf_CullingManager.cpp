#include "Perf_CullingManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"
#include "Camera/CameraComponent.h"
#include "Components/PrimitiveComponent.h"
#include "DrawDebugHelpers.h"

UPerf_CullingManager::UPerf_CullingManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;
}

void UPerf_CullingManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Cache all actors in the world for performance
    if (UWorld* World = GetWorld())
    {
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (Actor && Actor != GetOwner())
            {
                CachedActors.Add(Actor);
            }
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("CullingManager: Cached %d actors for culling"), CachedActors.Num());
}

void UPerf_CullingManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    LastUpdateTime += DeltaTime;
    if (LastUpdateTime >= CullingSettings.CullingUpdateFrequency)
    {
        UpdateCulling();
        LastUpdateTime = 0.0f;
    }
}

void UPerf_CullingManager::UpdateCulling()
{
    if (!GetWorld())
        return;

    CurrentVisibleActors = 0;
    CurrentCulledActors = 0;

    switch (CullingSettings.CullingMethod)
    {
        case EPerf_CullingMethod::Distance:
            PerformDistanceCulling();
            break;
        case EPerf_CullingMethod::Frustum:
            PerformFrustumCulling();
            break;
        case EPerf_CullingMethod::Occlusion:
            PerformOcclusionCulling();
            break;
        case EPerf_CullingMethod::LOD:
            PerformDistanceCulling(); // LOD is handled by distance for now
            break;
        case EPerf_CullingMethod::Hybrid:
        default:
            PerformHybridCulling();
            break;
    }

    // Enforce maximum visible actors limit
    if (CurrentVisibleActors > CullingSettings.MaxVisibleActors)
    {
        int32 ActorsToCull = CurrentVisibleActors - CullingSettings.MaxVisibleActors;
        for (int32 i = VisibleActors.Num() - 1; i >= 0 && ActorsToCull > 0; i--)
        {
            if (VisibleActors[i])
            {
                UpdateActorVisibility(VisibleActors[i], false);
                ActorsToCull--;
                CurrentCulledActors++;
                CurrentVisibleActors--;
            }
        }
    }
}

void UPerf_CullingManager::PerformDistanceCulling()
{
    VisibleActors.Empty();
    
    for (AActor* Actor : CachedActors)
    {
        if (!Actor || !IsValid(Actor))
            continue;

        float Distance = GetDistanceToActor(Actor);
        bool bShouldBeVisible = Distance <= CullingSettings.MaxDrawDistance;
        
        UpdateActorVisibility(Actor, bShouldBeVisible);
        
        if (bShouldBeVisible)
        {
            VisibleActors.Add(Actor);
            CurrentVisibleActors++;
        }
        else
        {
            CurrentCulledActors++;
        }
    }
}

void UPerf_CullingManager::PerformFrustumCulling()
{
    VisibleActors.Empty();
    
    for (AActor* Actor : CachedActors)
    {
        if (!Actor || !IsValid(Actor))
            continue;

        bool bInFrustum = IsActorInFrustum(Actor);
        bool bWithinDistance = GetDistanceToActor(Actor) <= CullingSettings.MaxDrawDistance;
        bool bShouldBeVisible = bInFrustum && bWithinDistance;
        
        UpdateActorVisibility(Actor, bShouldBeVisible);
        
        if (bShouldBeVisible)
        {
            VisibleActors.Add(Actor);
            CurrentVisibleActors++;
        }
        else
        {
            CurrentCulledActors++;
        }
    }
}

void UPerf_CullingManager::PerformOcclusionCulling()
{
    // For now, use distance culling as base and add simple occlusion check
    PerformDistanceCulling();
    
    // TODO: Implement proper occlusion culling using line traces
    // This would require more complex geometry analysis
}

void UPerf_CullingManager::PerformHybridCulling()
{
    VisibleActors.Empty();
    
    for (AActor* Actor : CachedActors)
    {
        if (!Actor || !IsValid(Actor))
            continue;

        float Distance = GetDistanceToActor(Actor);
        bool bWithinDistance = Distance <= CullingSettings.MaxDrawDistance;
        bool bInFrustum = CullingSettings.bEnableFrustumCulling ? IsActorInFrustum(Actor) : true;
        bool bShouldBeVisible = bWithinDistance && bInFrustum;
        
        UpdateActorVisibility(Actor, bShouldBeVisible);
        
        if (bShouldBeVisible)
        {
            VisibleActors.Add(Actor);
            CurrentVisibleActors++;
        }
        else
        {
            CurrentCulledActors++;
        }
    }
}

void UPerf_CullingManager::UpdateActorVisibility(AActor* Actor, bool bShouldBeVisible)
{
    if (!Actor)
        return;

    // Hide/show all primitive components
    TArray<UPrimitiveComponent*> PrimitiveComponents;
    Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
    
    for (UPrimitiveComponent* Component : PrimitiveComponents)
    {
        if (Component)
        {
            Component->SetVisibility(bShouldBeVisible);
        }
    }
}

bool UPerf_CullingManager::IsActorInFrustum(AActor* Actor) const
{
    if (!Actor || !GetWorld())
        return false;

    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (!PC || !PC->PlayerCameraManager)
        return true; // If no camera, assume visible

    FVector CameraLocation = PC->PlayerCameraManager->GetCameraLocation();
    FRotator CameraRotation = PC->PlayerCameraManager->GetCameraRotation();
    FVector ActorLocation = Actor->GetActorLocation();
    
    // Simple frustum check using dot product
    FVector ToActor = (ActorLocation - CameraLocation).GetSafeNormal();
    FVector CameraForward = CameraRotation.Vector();
    
    float DotProduct = FVector::DotProduct(CameraForward, ToActor);
    
    // Actor is in frustum if it's in front of camera (dot > 0) and within FOV
    return DotProduct > -0.5f; // Rough FOV check
}

float UPerf_CullingManager::GetDistanceToActor(AActor* Actor) const
{
    if (!Actor || !GetOwner())
        return FLT_MAX;

    return FVector::Dist(GetOwner()->GetActorLocation(), Actor->GetActorLocation());
}

void UPerf_CullingManager::SetCullingMethod(EPerf_CullingMethod NewMethod)
{
    CullingSettings.CullingMethod = NewMethod;
    UpdateCulling();
}

void UPerf_CullingManager::SetMaxDrawDistance(float NewDistance)
{
    CullingSettings.MaxDrawDistance = FMath::Max(0.0f, NewDistance);
    UpdateCulling();
}

float UPerf_CullingManager::GetCullingEfficiency() const
{
    int32 TotalActors = CurrentVisibleActors + CurrentCulledActors;
    if (TotalActors == 0)
        return 0.0f;
    
    return (float)CurrentCulledActors / (float)TotalActors * 100.0f;
}

void UPerf_CullingManager::OptimizeSceneCulling()
{
    // Refresh actor cache
    CachedActors.Empty();
    if (UWorld* World = GetWorld())
    {
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (Actor && Actor != GetOwner())
            {
                CachedActors.Add(Actor);
            }
        }
    }
    
    // Perform immediate culling update
    UpdateCulling();
    
    UE_LOG(LogTemp, Warning, TEXT("CullingManager: Scene optimized - %d visible, %d culled, %.1f%% efficiency"), 
           CurrentVisibleActors, CurrentCulledActors, GetCullingEfficiency());
}