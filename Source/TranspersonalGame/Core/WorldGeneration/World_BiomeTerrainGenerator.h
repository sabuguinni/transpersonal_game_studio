#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "World_BiomeTerrainGenerator.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_BiomeTerrainConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EBiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector CenterLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float BiomeRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    int32 TerrainFeatureCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float MinTerrainScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float MaxTerrainScale;

    FWorld_BiomeTerrainConfig()
    {
        BiomeType = EBiomeType::Savana;
        CenterLocation = FVector::ZeroVector;
        BiomeRadius = 10000.0f;
        TerrainFeatureCount = 50;
        MinTerrainScale = 0.5f;
        MaxTerrainScale = 3.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorld_BiomeTerrainGenerator : public AActor
{
    GENERATED_BODY()

public:
    AWorld_BiomeTerrainGenerator();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation")
    TArray<FWorld_BiomeTerrainConfig> BiomeConfigs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation")
    bool bAutoGenerateOnBeginPlay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation")
    bool bUseProceduralPlacement;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation")
    int32 RandomSeed;

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateAllBiomeTerrain();

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateBiomeTerrain(EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void ClearBiomeTerrain(EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void ClearAllTerrain();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "World Generation")
    void EditorGenerateTerrain();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "World Generation")
    void EditorClearTerrain();

protected:
    UFUNCTION()
    void SpawnTerrainFeature(const FVector& Location, EBiomeType BiomeType, int32 FeatureIndex);

    UFUNCTION()
    FVector GetRandomLocationInBiome(const FWorld_BiomeTerrainConfig& Config);

    UFUNCTION()
    float GetTerrainScaleForBiome(EBiomeType BiomeType);

    UFUNCTION()
    FRotator GetRandomRotationForTerrain();

private:
    UPROPERTY()
    TArray<AActor*> SpawnedTerrainActors;

    UPROPERTY()
    class UStaticMesh* DefaultTerrainMesh;

    void InitializeDefaultAssets();
    void SetupBiomeConfigs();
};