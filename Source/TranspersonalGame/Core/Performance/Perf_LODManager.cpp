#include "Perf_LODManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"

APerf_LODManager::APerf_LODManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // 10 FPS for LOD updates

    UpdateFrequency = 0.1f;
    bEnableLODSystem = true;
    MaxMeshesToProcess = 100;
    CurrentLODUpdates = 0;
    LastUpdateTime = 0.0f;
    TimeSinceLastUpdate = 0.0f;
    CachedPlayerPawn = nullptr;

    // Initialize default LOD settings
    DefaultLODSettings = FPerf_MeshLODSettings();
}

void APerf_LODManager::BeginPlay()
{
    Super::BeginPlay();
    
    CachePlayerPawn();
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, 
            FString::Printf(TEXT("LOD Manager initialized with %d managed meshes"), ManagedMeshes.Num()));
    }
}

void APerf_LODManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bEnableLODSystem)
    {
        return;
    }

    TimeSinceLastUpdate += DeltaTime;

    if (TimeSinceLastUpdate >= UpdateFrequency)
    {
        UpdateAllMeshLODs();
        TimeSinceLastUpdate = 0.0f;
        LastUpdateTime = GetWorld()->GetTimeSeconds();
    }
}

void APerf_LODManager::RegisterMeshComponent(UStaticMeshComponent* MeshComponent)
{
    if (MeshComponent && !ManagedMeshes.Contains(MeshComponent))
    {
        ManagedMeshes.Add(MeshComponent);
        
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Blue, 
                FString::Printf(TEXT("Registered mesh: %s (Total: %d)"), 
                    *MeshComponent->GetName(), ManagedMeshes.Num()));
        }
    }
}

void APerf_LODManager::UnregisterMeshComponent(UStaticMeshComponent* MeshComponent)
{
    if (MeshComponent)
    {
        ManagedMeshes.Remove(MeshComponent);
        
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, 
                FString::Printf(TEXT("Unregistered mesh: %s (Total: %d)"), 
                    *MeshComponent->GetName(), ManagedMeshes.Num()));
        }
    }
}

void APerf_LODManager::UpdateLODForMesh(UStaticMeshComponent* MeshComponent, float DistanceToPlayer)
{
    if (!MeshComponent || !MeshComponent->GetStaticMesh())
    {
        return;
    }

    int32 OptimalLOD = GetOptimalLODLevel(DistanceToPlayer, DefaultLODSettings);
    
    // Check if mesh should be culled
    if (DefaultLODSettings.bEnableDistanceCulling && 
        DistanceToPlayer > DefaultLODSettings.MaxDrawDistance)
    {
        MeshComponent->SetVisibility(false);
        return;
    }
    else
    {
        MeshComponent->SetVisibility(true);
    }

    // Set LOD level
    MeshComponent->SetForcedLodModel(OptimalLOD + 1); // UE5 uses 1-based LOD indices
    CurrentLODUpdates++;
}

void APerf_LODManager::UpdateAllMeshLODs()
{
    if (!CachedPlayerPawn)
    {
        CachePlayerPawn();
        if (!CachedPlayerPawn)
        {
            return;
        }
    }

    CurrentLODUpdates = 0;
    FVector PlayerLocation = CachedPlayerPawn->GetActorLocation();

    // Process meshes in batches to avoid frame spikes
    int32 MeshesToProcess = FMath::Min(MaxMeshesToProcess, ManagedMeshes.Num());
    
    for (int32 i = 0; i < MeshesToProcess; ++i)
    {
        if (ManagedMeshes.IsValidIndex(i) && ManagedMeshes[i])
        {
            float Distance = GetDistanceToPlayer(ManagedMeshes[i]->GetComponentLocation());
            UpdateLODForMesh(ManagedMeshes[i], Distance);
        }
    }

    // Clean up invalid mesh references
    ManagedMeshes.RemoveAll([](const UStaticMeshComponent* Mesh) 
    {
        return !IsValid(Mesh);
    });
}

int32 APerf_LODManager::GetOptimalLODLevel(float Distance, const FPerf_MeshLODSettings& Settings)
{
    int32 LODLevel = 0;

    for (int32 i = Settings.LODLevels.Num() - 1; i >= 0; --i)
    {
        if (Distance >= Settings.LODLevels[i].Distance)
        {
            LODLevel = Settings.LODLevels[i].LODIndex;
            break;
        }
    }

    return LODLevel;
}

void APerf_LODManager::SetLODSettings(const FPerf_MeshLODSettings& NewSettings)
{
    DefaultLODSettings = NewSettings;
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Orange, 
            TEXT("LOD settings updated"));
    }
}

float APerf_LODManager::GetDistanceToPlayer(const FVector& MeshLocation)
{
    if (!CachedPlayerPawn)
    {
        return 10000.0f; // Return large distance if no player
    }

    return FVector::Dist(CachedPlayerPawn->GetActorLocation(), MeshLocation);
}

void APerf_LODManager::EnableLODSystem(bool bEnable)
{
    bEnableLODSystem = bEnable;
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Cyan, 
            FString::Printf(TEXT("LOD System %s"), bEnable ? TEXT("Enabled") : TEXT("Disabled")));
    }
}

void APerf_LODManager::SetUpdateFrequency(float NewFrequency)
{
    UpdateFrequency = FMath::Clamp(NewFrequency, 0.01f, 1.0f);
    PrimaryActorTick.TickInterval = UpdateFrequency;
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Purple, 
            FString::Printf(TEXT("LOD Update Frequency: %.2f"), UpdateFrequency));
    }
}

void APerf_LODManager::CachePlayerPawn()
{
    if (UWorld* World = GetWorld())
    {
        if (APlayerController* PC = World->GetFirstPlayerController())
        {
            CachedPlayerPawn = PC->GetPawn();
        }
    }
}

void APerf_LODManager::ProcessMeshBatch(int32 StartIndex, int32 EndIndex)
{
    if (!CachedPlayerPawn)
    {
        return;
    }

    for (int32 i = StartIndex; i < EndIndex && i < ManagedMeshes.Num(); ++i)
    {
        if (ManagedMeshes.IsValidIndex(i) && ManagedMeshes[i])
        {
            float Distance = GetDistanceToPlayer(ManagedMeshes[i]->GetComponentLocation());
            UpdateLODForMesh(ManagedMeshes[i], Distance);
        }
    }
}