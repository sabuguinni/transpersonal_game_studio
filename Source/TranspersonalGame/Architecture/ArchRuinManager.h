#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ArchRuinManager.generated.h"

/**
 * Arch_RuinType — categories of prehistoric ruin structures
 */
UENUM(BlueprintType)
enum class EArch_RuinType : uint8
{
    StonePillar     UMETA(DisplayName = "Stone Pillar"),
    ArchGateway     UMETA(DisplayName = "Arch Gateway"),
    WallSection     UMETA(DisplayName = "Wall Section"),
    BoulderCluster  UMETA(DisplayName = "Boulder Cluster"),
    CaveDwelling    UMETA(DisplayName = "Cave Dwelling"),
    RockOutcrop     UMETA(DisplayName = "Rock Outcrop"),
};

/**
 * FArch_RuinSpawnData — data for spawning a ruin element
 */
USTRUCT(BlueprintType)
struct FArch_RuinSpawnData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    EArch_RuinType RuinType = EArch_RuinType::StonePillar;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    FRotator Rotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    FVector Scale = FVector(1.0f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    float WeatheringLevel = 0.5f;  // 0=fresh, 1=fully eroded

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    bool bHasVegetationGrowth = true;
};

/**
 * AArchRuinManager — manages placement and state of prehistoric ruin structures
 * in the Cretaceous world. Ruins serve as environmental storytelling elements —
 * each structure documents the presence of ancient life in the landscape.
 */
UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Architecture Ruin Manager"))
class TRANSPERSONALGAME_API AArchRuinManager : public AActor
{
    GENERATED_BODY()

public:
    AArchRuinManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    /** Spawn a ruin element at the given location */
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    AActor* SpawnRuinElement(const FArch_RuinSpawnData& SpawnData);

    /** Spawn a cluster of ruins around a central point */
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void SpawnRuinCluster(FVector CenterLocation, int32 NumElements, float Radius);

    /** Clear all spawned ruin actors managed by this manager */
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void ClearAllRuins();

    /** Get the total number of active ruin actors */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Architecture")
    int32 GetActiveRuinCount() const;

    /** Apply weathering pass to all managed ruins */
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void ApplyWeatheringPass(float WeatheringStrength);

protected:
    /** All ruin actors currently managed */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Architecture")
    TArray<AActor*> ManagedRuins;

    /** Default mesh for stone pillars */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture|Meshes")
    TSoftObjectPtr<UStaticMesh> PillarMesh;

    /** Default mesh for arch gateways */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture|Meshes")
    TSoftObjectPtr<UStaticMesh> ArchMesh;

    /** Default mesh for wall sections */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture|Meshes")
    TSoftObjectPtr<UStaticMesh> WallMesh;

    /** Default mesh for boulder clusters */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture|Meshes")
    TSoftObjectPtr<UStaticMesh> BoulderMesh;

    /** Radius around biome center where ruins are concentrated */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture|Placement")
    float BiomeRuinRadius = 5000.0f;

    /** Central biome coordinates for ruin placement */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture|Placement")
    FVector BiomeCenter = FVector(50000.0f, 50000.0f, 100.0f);

    /** Maximum number of ruin elements in the world */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture|Placement")
    int32 MaxRuinElements = 50;

private:
    /** Internal counter for unique ruin labels */
    int32 RuinSpawnCounter = 0;
};
