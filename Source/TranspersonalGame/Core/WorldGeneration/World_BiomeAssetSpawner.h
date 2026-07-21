#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "SharedTypes.h"
#include "World_BiomeAssetSpawner.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_BiomeAssetConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Asset")
    FString AssetName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Asset")
    FString AssetPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Asset")
    EBiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Asset")
    FVector SpawnLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Asset")
    FRotator SpawnRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Asset")
    FVector SpawnScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Asset")
    float SpawnRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Asset")
    int32 SpawnCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Asset")
    bool bRandomRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Asset")
    bool bRandomScale;

    FWorld_BiomeAssetConfig()
    {
        AssetName = TEXT("");
        AssetPath = TEXT("");
        BiomeType = EBiomeType::Savanna;
        SpawnLocation = FVector::ZeroVector;
        SpawnRotation = FRotator::ZeroRotator;
        SpawnScale = FVector::OneVector;
        SpawnRadius = 5000.0f;
        SpawnCount = 50;
        bRandomRotation = true;
        bRandomScale = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorld_BiomeAssetSpawner : public AActor
{
    GENERATED_BODY()

public:
    AWorld_BiomeAssetSpawner();

protected:
    virtual void BeginPlay() override;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Spawning")
    TArray<FWorld_BiomeAssetConfig> BiomeAssetConfigs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Spawning")
    bool bAutoSpawnOnBeginPlay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Spawning")
    bool bClearExistingAssets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Spawning")
    FString AssetTagPrefix;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Biome Spawning")
    TArray<AActor*> SpawnedAssets;

    UFUNCTION(BlueprintCallable, Category = "Biome Spawning")
    void SpawnBiomeAssets();

    UFUNCTION(BlueprintCallable, Category = "Biome Spawning")
    void SpawnAssetsForBiome(EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Biome Spawning")
    void ClearSpawnedAssets();

    UFUNCTION(BlueprintCallable, Category = "Biome Spawning")
    AActor* SpawnSingleAsset(const FWorld_BiomeAssetConfig& Config, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Biome Spawning")
    FVector GetBiomeCenter(EBiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Biome Spawning")
    TArray<FVector> GenerateSpawnLocations(const FWorld_BiomeAssetConfig& Config) const;

    UFUNCTION(BlueprintCallable, Category = "Biome Spawning")
    void SetupDefaultBiomeConfigs();

    UFUNCTION(BlueprintCallable, Category = "Biome Spawning", CallInEditor = true)
    void EditorSpawnBiomeAssets();

    UFUNCTION(BlueprintCallable, Category = "Biome Spawning", CallInEditor = true)
    void EditorClearAssets();

protected:
    UFUNCTION()
    FVector GetRandomLocationInRadius(const FVector& Center, float Radius) const;

    UFUNCTION()
    FRotator GetRandomRotation() const;

    UFUNCTION()
    FVector GetRandomScale(const FVector& BaseScale) const;

    UFUNCTION()
    bool IsValidSpawnLocation(const FVector& Location) const;
};