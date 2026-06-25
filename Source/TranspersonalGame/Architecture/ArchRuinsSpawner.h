#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ArchRuinsSpawner.generated.h"

USTRUCT(BlueprintType)
struct FArch_RuinPillarConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture|Ruins")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture|Ruins")
    FVector Scale = FVector(0.5f, 0.5f, 4.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture|Ruins")
    float TiltAngleDegrees = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture|Ruins")
    bool bIsBroken = false;
};

/**
 * AArchRuinsSpawner — Agent #7 Architecture
 * Spawns Cretaceous-era stone ruin props (pillars, walls, archways)
 * at configurable world locations. Designed for biome coords (50000, 50000).
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AArchRuinsSpawner : public AActor
{
    GENERATED_BODY()

public:
    AArchRuinsSpawner();

    virtual void BeginPlay() override;

    /** Array of pillar configurations to spawn on BeginPlay */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture|Ruins")
    TArray<FArch_RuinPillarConfig> PillarConfigs;

    /** Static mesh to use for pillars (assign in Blueprint) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture|Ruins")
    UStaticMesh* PillarMesh;

    /** Radius around this actor's location to scatter ruins */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture|Ruins")
    float ScatterRadius = 1000.0f;

    /** Number of pillars to auto-generate if PillarConfigs is empty */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture|Ruins")
    int32 AutoPillarCount = 5;

    /** Spawn all configured pillars */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Architecture|Ruins")
    void SpawnRuins();

    /** Clear all spawned ruin actors owned by this spawner */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Architecture|Ruins")
    void ClearRuins();

private:
    UPROPERTY()
    TArray<AActor*> SpawnedRuinActors;

    void GenerateDefaultPillarConfigs();
};
