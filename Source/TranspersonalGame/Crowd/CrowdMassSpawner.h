#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CrowdSimulationTypes.h"
#include "CrowdMassSpawner.generated.h"

UENUM(BlueprintType)
enum class ECrowd_SpawnPattern : uint8
{
    Scattered       UMETA(DisplayName = "Scattered"),
    HerdFormation   UMETA(DisplayName = "HerdFormation"),
    MigrationLine   UMETA(DisplayName = "MigrationLine"),
    WaterGathering  UMETA(DisplayName = "WaterGathering"),
    ForestEdge      UMETA(DisplayName = "ForestEdge")
};

USTRUCT(BlueprintType)
struct FCrowd_SpawnRequest
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Spawn")
    ECrowd_SpawnPattern Pattern = ECrowd_SpawnPattern::Scattered;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Spawn")
    int32 Count = 20;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Spawn")
    float SpawnRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Spawn")
    FVector SpawnCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Spawn")
    float MinSpacing = 150.0f;
};

USTRUCT(BlueprintType)
struct FCrowd_LODSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|LOD")
    float FullDetailDistance = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|LOD")
    float MediumDetailDistance = 8000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|LOD")
    float LowDetailDistance = 20000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|LOD")
    bool bUseBillboardAtMaxDistance = true;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACrowd_MassSpawner : public AActor
{
    GENERATED_BODY()

public:
    ACrowd_MassSpawner();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Crowd|Spawn")
    void SpawnHerd(const FCrowd_SpawnRequest& Request);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Spawn")
    void DespawnAllAgents();

    UFUNCTION(BlueprintCallable, Category = "Crowd|Spawn")
    int32 GetActiveAgentCount() const;

    UFUNCTION(BlueprintCallable, Category = "Crowd|LOD")
    void UpdateLODForAllAgents(const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Spawn")
    void TriggerStampedeFromLocation(const FVector& ThreatLocation, float ThreatRadius);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    FCrowd_SpawnRequest DefaultSpawnConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    FCrowd_LODSettings LODConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    int32 MaxAgentsInWorld = 500;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    bool bAutoSpawnOnBeginPlay = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float TickInterval = 0.1f;

private:
    UPROPERTY()
    TArray<AActor*> SpawnedAgents;

    float TimeSinceLastTick = 0.0f;

    FVector GetScatteredPosition(const FCrowd_SpawnRequest& Request, int32 Index) const;
    FVector GetHerdFormationPosition(const FCrowd_SpawnRequest& Request, int32 Index) const;
    FVector GetMigrationLinePosition(const FCrowd_SpawnRequest& Request, int32 Index) const;
};
