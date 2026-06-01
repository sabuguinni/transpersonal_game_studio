#include "Perf_CullingManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/Pawn.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/LocalPlayer.h"

APerf_CullingManager::APerf_CullingManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // Update every 0.1 seconds
    
    LastOcclusionCheck = 0.0f;
    CullingUpdateCounter = 0;
    
    // Set default culling settings
    CullingSettings = FPerf_CullingSettings();
}

void APerf_CullingManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Perf_CullingManager: Started culling management"));
}

void APerf_CullingManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (RegisteredActors.Num() == 0)
    {
        return;
    }
    
    CullingUpdateCounter++;
    
    // Update distance and frustum culling every tick
    if (CullingSettings.bEnableDistanceCulling)
    {
        UpdateDistanceCulling();
    }
    
    if (CullingSettings.bEnableFrustumCulling)
    {
        UpdateFrustumCulling();
    }
    
    // Update occlusion culling less frequently
    if (CullingSettings.bEnableOcclusionCulling)
    {
        LastOcclusionCheck += DeltaTime;
        if (LastOcclusionCheck >= CullingSettings.OcclusionCheckFrequency)
        {
            UpdateOcclusionCulling();
            LastOcclusionCheck = 0.0f;
        }
    }
}

void APerf_CullingManager::RegisterActorForCulling(AActor* Actor, float CullingDistance)
{
    if (!Actor || RegisteredActors.Contains(Actor))
    {
        return;
    }
    
    RegisteredActors.Add(Actor);
    ActorCullingDistances.Add(Actor, CullingDistance);
    
    UE_LOG(LogTemp, Log, TEXT("Perf_CullingManager: Registered actor %s for culling"), *Actor->GetName());
}

void APerf_CullingManager::UnregisterActorFromCulling(AActor* Actor)
{
    if (!Actor)
    {
        return;
    }
    
    RegisteredActors.Remove(Actor);
    ActorCullingDistances.Remove(Actor);
    CulledActors.Remove(Actor);
    
    // Ensure actor is visible when unregistered
    if (IsValid(Actor))
    {
        Actor->SetActorHiddenInGame(false);
    }
}

void APerf_CullingManager::SetCullingSettings(const FPerf_CullingSettings& NewSettings)
{
    CullingSettings = NewSettings;
    ForceUpdateCulling();
}

void APerf_CullingManager::ForceUpdateCulling()
{
    // Reset all actors to visible
    for (AActor* Actor : RegisteredActors)
    {
        if (IsValid(Actor))
        {
            Actor->SetActorHiddenInGame(false);
        }
    }
    
    CulledActors.Empty();
    
    // Reapply culling
    if (CullingSettings.bEnableDistanceCulling)
    {
        UpdateDistanceCulling();
    }
    
    if (CullingSettings.bEnableFrustumCulling)
    {
        UpdateFrustumCulling();
    }
    
    if (CullingSettings.bEnableOcclusionCulling)
    {
        UpdateOcclusionCulling();
    }
}

void APerf_CullingManager::UpdateDistanceCulling()
{
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
    
    for (AActor* Actor : RegisteredActors)
    {
        if (!IsValid(Actor))
        {
            continue;
        }
        
        float* CullingDistance = ActorCullingDistances.Find(Actor);
        float MaxDistance = CullingDistance ? *CullingDistance : CullingSettings.MaxRenderDistance;
        
        float Distance = FVector::Dist(PlayerLocation, Actor->GetActorLocation());
        bool bShouldCull = Distance > MaxDistance;
        
        if (bShouldCull && !CulledActors.Contains(Actor))
        {
            Actor->SetActorHiddenInGame(true);
            CulledActors.Add(Actor);
        }
        else if (!bShouldCull && CulledActors.Contains(Actor))
        {
            Actor->SetActorHiddenInGame(false);
            CulledActors.Remove(Actor);
        }
    }
}

void APerf_CullingManager::UpdateFrustumCulling()
{
    for (AActor* Actor : RegisteredActors)
    {
        if (!IsValid(Actor) || CulledActors.Contains(Actor))
        {
            continue;
        }
        
        bool bInFrustum = IsActorInCameraFrustum(Actor);
        
        if (!bInFrustum && !CulledActors.Contains(Actor))
        {
            Actor->SetActorHiddenInGame(true);
            CulledActors.Add(Actor);
        }
        else if (bInFrustum && CulledActors.Contains(Actor))
        {
            Actor->SetActorHiddenInGame(false);
            CulledActors.Remove(Actor);
        }
    }
}

void APerf_CullingManager::UpdateOcclusionCulling()
{
    for (AActor* Actor : RegisteredActors)
    {
        if (!IsValid(Actor) || CulledActors.Contains(Actor))
        {
            continue;
        }
        
        bool bIsOccluded = IsActorOccluded(Actor);
        
        if (bIsOccluded && !CulledActors.Contains(Actor))
        {
            Actor->SetActorHiddenInGame(true);
            CulledActors.Add(Actor);
        }
        else if (!bIsOccluded && CulledActors.Contains(Actor))
        {
            Actor->SetActorHiddenInGame(false);
            CulledActors.Remove(Actor);
        }
    }
}

bool APerf_CullingManager::IsActorInCameraFrustum(AActor* Actor) const
{
    if (!Actor)
    {
        return false;
    }
    
    UWorld* World = GetWorld();
    if (!World)
    {
        return true; // Default to visible if we can't check
    }
    
    APlayerController* PlayerController = World->GetFirstPlayerController();
    if (!PlayerController)
    {
        return true;
    }
    
    // Simple frustum check - in a real implementation you'd use the camera's frustum
    FVector CameraLocation;
    FRotator CameraRotation;
    PlayerController->GetPlayerViewPoint(CameraLocation, CameraRotation);
    
    FVector ToActor = Actor->GetActorLocation() - CameraLocation;
    ToActor.Normalize();
    
    FVector CameraForward = CameraRotation.Vector();
    float DotProduct = FVector::DotProduct(CameraForward, ToActor);
    
    // Actor is in front of camera (roughly)
    return DotProduct > -0.5f;
}

bool APerf_CullingManager::IsActorOccluded(AActor* Actor) const
{
    if (!Actor)
    {
        return false;
    }
    
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }
    
    APlayerController* PlayerController = World->GetFirstPlayerController();
    if (!PlayerController)
    {
        return false;
    }
    
    FVector CameraLocation;
    FRotator CameraRotation;
    PlayerController->GetPlayerViewPoint(CameraLocation, CameraRotation);
    
    FVector ActorLocation = Actor->GetActorLocation();
    
    // Perform line trace to check occlusion
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(Actor);
    QueryParams.AddIgnoredActor(PlayerController->GetPawn());
    
    bool bHit = World->LineTraceSingleByChannel(
        HitResult,
        CameraLocation,
        ActorLocation,
        ECollisionChannel::ECC_Visibility,
        QueryParams
    );
    
    return bHit; // If we hit something, the actor is occluded
}