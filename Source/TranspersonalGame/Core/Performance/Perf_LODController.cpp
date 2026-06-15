#include "Perf_LODController.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/PlayerController.h"

UPerf_LODController::UPerf_LODController()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second
    
    LODSettings = FPerf_LODSettings();
    CurrentLODLevel = EPerf_LODLevel::LOD0;
    DistanceToPlayer = 0.0f;
    bIsEnabled = true;
    UpdateFrequency = 0.1f;
    LastUpdateTime = 0.0f;
    bWasCulled = false;
}

void UPerf_LODController::BeginPlay()
{
    Super::BeginPlay();
    
    // Initial LOD calculation
    UpdateLOD();
}

void UPerf_LODController::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!bIsEnabled)
    {
        return;
    }

    // Update at specified frequency
    LastUpdateTime += DeltaTime;
    if (LastUpdateTime >= UpdateFrequency)
    {
        UpdateLOD();
        LastUpdateTime = 0.0f;
    }
}

void UPerf_LODController::UpdateLOD()
{
    if (!bIsEnabled)
    {
        return;
    }

    CalculateDistanceToPlayer();

    EPerf_LODLevel NewLODLevel = CurrentLODLevel;

    // Determine LOD level based on distance
    if (DistanceToPlayer <= LODSettings.LOD0Distance)
    {
        NewLODLevel = EPerf_LODLevel::LOD0;
    }
    else if (DistanceToPlayer <= LODSettings.LOD1Distance)
    {
        NewLODLevel = EPerf_LODLevel::LOD1;
    }
    else if (DistanceToPlayer <= LODSettings.LOD2Distance)
    {
        NewLODLevel = EPerf_LODLevel::LOD2;
    }
    else if (DistanceToPlayer >= LODSettings.CullDistance && LODSettings.bEnableDistanceCulling)
    {
        NewLODLevel = EPerf_LODLevel::Culled;
    }

    // Apply frustum culling if enabled
    if (LODSettings.bEnableFrustumCulling && NewLODLevel != EPerf_LODLevel::Culled)
    {
        if (!IsInPlayerView())
        {
            NewLODLevel = EPerf_LODLevel::Culled;
        }
    }

    // Apply the new LOD level if it changed
    if (NewLODLevel != CurrentLODLevel)
    {
        ApplyLODLevel(NewLODLevel);
    }
}

void UPerf_LODController::CalculateDistanceToPlayer()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0);
    if (!PlayerPawn)
    {
        DistanceToPlayer = 99999.0f; // Very far if no player
        return;
    }

    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return;
    }

    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    FVector OwnerLocation = Owner->GetActorLocation();
    
    DistanceToPlayer = FVector::Dist(PlayerLocation, OwnerLocation);
}

void UPerf_LODController::ApplyLODLevel(EPerf_LODLevel NewLODLevel)
{
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return;
    }

    CurrentLODLevel = NewLODLevel;

    // Handle culling
    if (NewLODLevel == EPerf_LODLevel::Culled)
    {
        if (!bWasCulled)
        {
            Owner->SetActorHiddenInGame(true);
            Owner->SetActorEnableCollision(false);
            bWasCulled = true;
        }
        return;
    }
    else if (bWasCulled)
    {
        // Un-cull the actor
        Owner->SetActorHiddenInGame(false);
        Owner->SetActorEnableCollision(true);
        bWasCulled = false;
    }

    // Apply LOD to static mesh components
    TArray<UStaticMeshComponent*> MeshComponents;
    Owner->GetComponents<UStaticMeshComponent>(MeshComponents);

    for (UStaticMeshComponent* MeshComp : MeshComponents)
    {
        if (!MeshComp)
        {
            continue;
        }

        int32 LODIndex = 0;
        switch (NewLODLevel)
        {
            case EPerf_LODLevel::LOD0:
                LODIndex = 0;
                break;
            case EPerf_LODLevel::LOD1:
                LODIndex = 1;
                break;
            case EPerf_LODLevel::LOD2:
                LODIndex = 2;
                break;
            default:
                LODIndex = 0;
                break;
        }

        // Force LOD level
        MeshComp->SetForcedLodModel(LODIndex + 1); // +1 because 0 means auto
    }
}

bool UPerf_LODController::IsInPlayerView() const
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }

    APlayerController* PlayerController = UGameplayStatics::GetPlayerController(World, 0);
    if (!PlayerController)
    {
        return false;
    }

    APawn* PlayerPawn = PlayerController->GetPawn();
    if (!PlayerPawn)
    {
        return false;
    }

    // Get player camera location and rotation
    FVector CameraLocation;
    FRotator CameraRotation;
    PlayerController->GetPlayerViewPoint(CameraLocation, CameraRotation);

    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return false;
    }

    // Calculate direction to object
    FVector ToObject = Owner->GetActorLocation() - CameraLocation;
    ToObject.Normalize();

    // Get camera forward vector
    FVector CameraForward = CameraRotation.Vector();

    // Check if object is within view cone (90 degree FOV)
    float DotProduct = FVector::DotProduct(CameraForward, ToObject);
    return DotProduct > 0.0f; // Object is in front of camera
}

void UPerf_LODController::SetLODSettings(const FPerf_LODSettings& NewSettings)
{
    LODSettings = NewSettings;
    UpdateLOD(); // Immediately apply new settings
}

void UPerf_LODController::SetEnabled(bool bEnabled)
{
    bIsEnabled = bEnabled;
    
    if (!bIsEnabled && bWasCulled)
    {
        // Un-cull if disabling LOD system
        AActor* Owner = GetOwner();
        if (Owner)
        {
            Owner->SetActorHiddenInGame(false);
            Owner->SetActorEnableCollision(true);
            bWasCulled = false;
        }
    }
}