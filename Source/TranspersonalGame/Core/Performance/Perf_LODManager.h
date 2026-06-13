#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "SharedTypes.h"
#include "Perf_LODManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_LODLevel
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float Distance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    int32 LODIndex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float CullingDistance;

    FPerf_LODLevel()
    {
        Distance = 1000.0f;
        LODIndex = 0;
        CullingDistance = 5000.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_MeshLODSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    TArray<FPerf_LODLevel> LODLevels;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    bool bEnableDistanceCulling;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float MaxDrawDistance;

    FPerf_MeshLODSettings()
    {
        bEnableDistanceCulling = true;
        MaxDrawDistance = 10000.0f;
        
        // Default LOD levels
        FPerf_LODLevel LOD0;
        LOD0.Distance = 0.0f;
        LOD0.LODIndex = 0;
        LOD0.CullingDistance = 2000.0f;
        LODLevels.Add(LOD0);

        FPerf_LODLevel LOD1;
        LOD1.Distance = 1500.0f;
        LOD1.LODIndex = 1;
        LOD1.CullingDistance = 4000.0f;
        LODLevels.Add(LOD1);

        FPerf_LODLevel LOD2;
        LOD2.Distance = 3000.0f;
        LOD2.LODIndex = 2;
        LOD2.CullingDistance = 8000.0f;
        LODLevels.Add(LOD2);
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API APerf_LODManager : public AActor
{
    GENERATED_BODY()

public:
    APerf_LODManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    FPerf_MeshLODSettings DefaultLODSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float UpdateFrequency;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableLODSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxMeshesToProcess;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    TArray<UStaticMeshComponent*> ManagedMeshes;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 CurrentLODUpdates;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float LastUpdateTime;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void RegisterMeshComponent(UStaticMeshComponent* MeshComponent);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UnregisterMeshComponent(UStaticMeshComponent* MeshComponent);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdateLODForMesh(UStaticMeshComponent* MeshComponent, float DistanceToPlayer);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdateAllMeshLODs();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    int32 GetOptimalLODLevel(float Distance, const FPerf_MeshLODSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetLODSettings(const FPerf_MeshLODSettings& NewSettings);

    UFUNCTION(BlueprintPure, Category = "Performance")
    float GetDistanceToPlayer(const FVector& MeshLocation);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void EnableLODSystem(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetUpdateFrequency(float NewFrequency);

private:
    float TimeSinceLastUpdate;
    APawn* CachedPlayerPawn;

    void CachePlayerPawn();
    void ProcessMeshBatch(int32 StartIndex, int32 EndIndex);
};