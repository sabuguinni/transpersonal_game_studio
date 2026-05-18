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
    EBiomeType BiomeType = EBiomeType::Savana;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector CenterLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float BiomeRadius = 10000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    int32 RockCount = 15;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    int32 VegetationCount = 25;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float MinScale = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float MaxScale = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assets")
    TArray<FString> RockAssetPaths;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assets")
    TArray<FString> VegetationAssetPaths;
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
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    TArray<FWorld_BiomeTerrainConfig> BiomeConfigs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    bool bAutoGenerateOnBeginPlay = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    int32 RandomSeed = 12345;

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateAllBiomes();

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateBiome(const FWorld_BiomeTerrainConfig& Config);

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void ClearGeneratedTerrain();

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    int32 GetGeneratedActorCount() const;

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Editor Tools")
    void EditorGenerateAllBiomes();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Editor Tools")
    void EditorClearTerrain();

protected:
    UFUNCTION(BlueprintCallable, Category = "Generation")
    AStaticMeshActor* SpawnTerrainActor(const FString& AssetPath, const FVector& Location, const FRotator& Rotation, const FVector& Scale);

    UFUNCTION(BlueprintCallable, Category = "Generation")
    FVector GenerateRandomLocationInBiome(const FWorld_BiomeTerrainConfig& Config) const;

    UFUNCTION(BlueprintCallable, Category = "Generation")
    void InitializeDefaultBiomeConfigs();

private:
    UPROPERTY()
    TArray<AStaticMeshActor*> GeneratedTerrainActors;

    void SetupBiomeConfiguration(EBiomeType BiomeType, const FVector& Location);
    bool LoadAndValidateAssets(const FWorld_BiomeTerrainConfig& Config);
};