#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Engine/World.h"
#include "../SharedTypes.h"
#include "World_BiomeGenerator.generated.h"

UENUM(BlueprintType)
enum class EWorld_BiomeType : uint8
{
    Swamp       UMETA(DisplayName = "Swamp"),
    Forest      UMETA(DisplayName = "Forest"),
    Savanna     UMETA(DisplayName = "Savanna"),
    Desert      UMETA(DisplayName = "Desert"),
    SnowMountain UMETA(DisplayName = "Snow Mountain")
};

USTRUCT(BlueprintType)
struct FWorld_BiomeConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EWorld_BiomeType BiomeType = EWorld_BiomeType::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector CenterLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Radius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    int32 VegetationDensity = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FLinearColor GroundColor = FLinearColor::Green;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float TemperatureRange = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float HumidityLevel = 0.5f;
};

USTRUCT(BlueprintType)
struct FWorld_VegetationSpawn
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    FRotator Rotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    FVector Scale = FVector::OneVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    EWorld_BiomeType SourceBiome = EWorld_BiomeType::Forest;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorld_BiomeGenerator : public AActor
{
    GENERATED_BODY()

public:
    AWorld_BiomeGenerator();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    TArray<FWorld_BiomeConfig> BiomeConfigurations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    bool bAutoGenerateOnPlay = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    int32 RandomSeed = 12345;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime")
    TArray<FWorld_VegetationSpawn> GeneratedVegetation;

public:
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "World Generation")
    void GenerateAllBiomes();

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateBiome(const FWorld_BiomeConfig& BiomeConfig);

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void ClearAllGeneration();

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    FVector GetBiomeCenter(EWorld_BiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    EWorld_BiomeType GetBiomeAtLocation(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void SpawnVegetationInBiome(const FWorld_BiomeConfig& BiomeConfig);

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void CreateWaterFeatures();

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void SetupBiomeTransitions();

private:
    void InitializeDefaultBiomes();
    FVector GenerateRandomLocationInRadius(const FVector& Center, float Radius) const;
    bool IsLocationValidForVegetation(const FVector& Location) const;
    void SpawnVegetationActor(const FWorld_VegetationSpawn& SpawnData);
};