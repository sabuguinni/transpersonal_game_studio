#include "Perf_LODManager.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "DrawDebugHelpers.h"

UPerf_LODManager::UPerf_LODManager()
{
    LODSettings = FPerf_LODSettings();
    bLODSystemEnabled = true;
    UpdateFrequency = 0.1f;
    LastUpdateTime = 0.0f;
}

void UPerf_LODManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Perf_LODManager: Initialized successfully"));
    
    // Set up tick delegate
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            FTimerHandle(),
            this,
            &UPerf_LODManager::UpdateLODForAllMeshes,
            UpdateFrequency,
            true
        );
    }
}

void UPerf_LODManager::Deinitialize()
{
    RegisteredMeshes.Empty();
    Super::Deinitialize();
}

void UPerf_LODManager::Tick(float DeltaTime)
{
    if (!bLODSystemEnabled)
    {
        return;
    }

    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastUpdateTime >= UpdateFrequency)
    {
        UpdateLODForAllMeshes();
        LastUpdateTime = CurrentTime;
    }
}

bool UPerf_LODManager::ShouldCreateSubsystem(UObject* Outer) const
{
    return true;
}

void UPerf_LODManager::RegisterMeshComponent(UStaticMeshComponent* MeshComponent)
{
    if (!MeshComponent || !IsValid(MeshComponent))
    {
        return;
    }

    // Check if already registered
    for (const FPerf_MeshLODData& ExistingData : RegisteredMeshes)
    {
        if (ExistingData.MeshComponent == MeshComponent)
        {
            return; // Already registered
        }
    }

    // Create new LOD data
    FPerf_MeshLODData NewMeshData;
    NewMeshData.MeshComponent = MeshComponent;
    NewMeshData.CurrentLODLevel = EPerf_LODLevel::LOD_High;
    NewMeshData.LastUpdateTime = GetWorld()->GetTimeSeconds();
    NewMeshData.DistanceToPlayer = 0.0f;
    NewMeshData.bIsVisible = true;

    RegisteredMeshes.Add(NewMeshData);

    UE_LOG(LogTemp, Log, TEXT("Perf_LODManager: Registered mesh component. Total: %d"), RegisteredMeshes.Num());
}

void UPerf_LODManager::UnregisterMeshComponent(UStaticMeshComponent* MeshComponent)
{
    if (!MeshComponent)
    {
        return;
    }

    RegisteredMeshes.RemoveAll([MeshComponent](const FPerf_MeshLODData& Data)
    {
        return Data.MeshComponent == MeshComponent;
    });
}

void UPerf_LODManager::UpdateLODForAllMeshes()
{
    if (!bLODSystemEnabled || RegisteredMeshes.Num() == 0)
    {
        return;
    }

    // Clean up invalid meshes first
    CleanupInvalidMeshes();

    // Update LOD for each registered mesh
    for (FPerf_MeshLODData& MeshData : RegisteredMeshes)
    {
        UpdateMeshLOD(MeshData);
    }
}

EPerf_LODLevel UPerf_LODManager::CalculateLODLevel(float Distance) const
{
    if (Distance <= LODSettings.HighQualityDistance)
    {
        return EPerf_LODLevel::LOD_High;
    }
    else if (Distance <= LODSettings.MediumQualityDistance)
    {
        return EPerf_LODLevel::LOD_Medium;
    }
    else if (Distance <= LODSettings.LowQualityDistance)
    {
        return EPerf_LODLevel::LOD_Low;
    }
    else if (Distance <= LODSettings.CullDistance)
    {
        return EPerf_LODLevel::LOD_Low; // Keep visible but lowest quality
    }
    else
    {
        return EPerf_LODLevel::LOD_Culled;
    }
}

void UPerf_LODManager::SetLODSettings(const FPerf_LODSettings& NewSettings)
{
    LODSettings = NewSettings;
    
    // Force update all LODs with new settings
    ForceUpdateAllLODs();
}

void UPerf_LODManager::ForceUpdateAllLODs()
{
    for (FPerf_MeshLODData& MeshData : RegisteredMeshes)
    {
        if (UStaticMeshComponent* MeshComp = MeshData.MeshComponent.Get())
        {
            float Distance = GetDistanceToPlayer(MeshComp->GetComponentLocation());
            EPerf_LODLevel NewLODLevel = CalculateLODLevel(Distance);
            ApplyLODLevel(MeshComp, NewLODLevel);
            
            MeshData.CurrentLODLevel = NewLODLevel;
            MeshData.DistanceToPlayer = Distance;
            MeshData.LastUpdateTime = GetWorld()->GetTimeSeconds();
        }
    }
}

void UPerf_LODManager::EnableLODSystem(bool bEnabled)
{
    bLODSystemEnabled = bEnabled;
    
    if (!bEnabled)
    {
        // Reset all meshes to high quality when disabled
        for (FPerf_MeshLODData& MeshData : RegisteredMeshes)
        {
            if (UStaticMeshComponent* MeshComp = MeshData.MeshComponent.Get())
            {
                ApplyLODLevel(MeshComp, EPerf_LODLevel::LOD_High);
                MeshData.CurrentLODLevel = EPerf_LODLevel::LOD_High;
            }
        }
    }
}

void UPerf_LODManager::UpdateMeshLOD(FPerf_MeshLODData& MeshData)
{
    UStaticMeshComponent* MeshComponent = MeshData.MeshComponent.Get();
    if (!MeshComponent || !IsValid(MeshComponent))
    {
        return;
    }

    // Calculate distance to player
    float Distance = GetDistanceToPlayer(MeshComponent->GetComponentLocation());
    MeshData.DistanceToPlayer = Distance;

    // Check frustum culling if enabled
    bool bInFrustum = true;
    if (LODSettings.bEnableFrustumCulling)
    {
        bInFrustum = IsMeshInViewFrustum(MeshComponent->GetComponentLocation());
    }

    // Calculate new LOD level
    EPerf_LODLevel NewLODLevel = CalculateLODLevel(Distance);
    
    // Apply frustum culling
    if (!bInFrustum && LODSettings.bEnableFrustumCulling)
    {
        NewLODLevel = EPerf_LODLevel::LOD_Culled;
    }

    // Apply LOD level if changed
    if (NewLODLevel != MeshData.CurrentLODLevel)
    {
        ApplyLODLevel(MeshComponent, NewLODLevel);
        MeshData.CurrentLODLevel = NewLODLevel;
    }

    MeshData.LastUpdateTime = GetWorld()->GetTimeSeconds();
    MeshData.bIsVisible = (NewLODLevel != EPerf_LODLevel::LOD_Culled);
}

void UPerf_LODManager::ApplyLODLevel(UStaticMeshComponent* MeshComponent, EPerf_LODLevel LODLevel)
{
    if (!MeshComponent || !IsValid(MeshComponent))
    {
        return;
    }

    switch (LODLevel)
    {
        case EPerf_LODLevel::LOD_High:
            MeshComponent->SetVisibility(true);
            MeshComponent->SetForcedLodModel(0); // Highest quality LOD
            MeshComponent->SetCastShadow(true);
            break;

        case EPerf_LODLevel::LOD_Medium:
            MeshComponent->SetVisibility(true);
            MeshComponent->SetForcedLodModel(1); // Medium quality LOD
            MeshComponent->SetCastShadow(true);
            break;

        case EPerf_LODLevel::LOD_Low:
            MeshComponent->SetVisibility(true);
            MeshComponent->SetForcedLodModel(2); // Low quality LOD
            MeshComponent->SetCastShadow(false); // Disable shadows for performance
            break;

        case EPerf_LODLevel::LOD_Culled:
            MeshComponent->SetVisibility(false);
            break;
    }
}

float UPerf_LODManager::GetDistanceToPlayer(const FVector& MeshLocation) const
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return 0.0f;
    }

    APlayerController* PlayerController = World->GetFirstPlayerController();
    if (!PlayerController)
    {
        return 0.0f;
    }

    APawn* PlayerPawn = PlayerController->GetPawn();
    if (!PlayerPawn)
    {
        return 0.0f;
    }

    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    return FVector::Dist(PlayerLocation, MeshLocation);
}

bool UPerf_LODManager::IsMeshInViewFrustum(const FVector& MeshLocation) const
{
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

    // Get player camera location and rotation
    FVector CameraLocation;
    FRotator CameraRotation;
    PlayerController->GetPlayerViewPoint(CameraLocation, CameraRotation);

    // Simple frustum check - check if mesh is roughly in front of camera
    FVector ToMesh = (MeshLocation - CameraLocation).GetSafeNormal();
    FVector CameraForward = CameraRotation.Vector();
    
    float DotProduct = FVector::DotProduct(ToMesh, CameraForward);
    
    // Consider mesh visible if it's within a 120-degree cone in front of camera
    return DotProduct > -0.5f; // cos(120°) ≈ -0.5
}

void UPerf_LODManager::CleanupInvalidMeshes()
{
    RegisteredMeshes.RemoveAll([](const FPerf_MeshLODData& Data)
    {
        return !Data.MeshComponent.IsValid() || !IsValid(Data.MeshComponent.Get());
    });
}