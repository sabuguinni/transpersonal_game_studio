#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "World_BiomeManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_BiomeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EWorld_BiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector CenterLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Radius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Temperature;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Humidity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Elevation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    bool bHasWaterBodies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    int32 VegetationDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<FString> AvailableSpecies;

    FWorld_BiomeData()
    {
        BiomeType = EWorld_BiomeType::Savanna;
        CenterLocation = FVector::ZeroVector;
        Radius = 10000.0f;
        Temperature = 25.0f;
        Humidity = 50.0f;
        Elevation = 100.0f;
        bHasWaterBodies = true;
        VegetationDensity = 50;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_WaterBodyData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
    EWorld_WaterType WaterType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
    FVector Scale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
    float FlowSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
    float Depth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
    bool bIsDrinkable;

    FWorld_WaterBodyData()
    {
        WaterType = EWorld_WaterType::River;
        Location = FVector::ZeroVector;
        Scale = FVector(1000.0f, 200.0f, 50.0f);
        FlowSpeed = 100.0f;
        Depth = 200.0f;
        bIsDrinkable = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorld_BiomeManager : public AActor
{
    GENERATED_BODY()

public:
    AWorld_BiomeManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Core biome management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation")
    TArray<FWorld_BiomeData> BiomeRegions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation")
    TArray<FWorld_WaterBodyData> WaterBodies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation")
    float WorldSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation")
    int32 BiomeTransitionDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation")
    bool bEnableProceduralGeneration;

    // Biome detection and management
    UFUNCTION(BlueprintCallable, Category = "Biome Management")
    EWorld_BiomeType GetBiomeAtLocation(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Biome Management")
    FWorld_BiomeData GetBiomeData(EWorld_BiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Biome Management")
    float GetTemperatureAtLocation(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Biome Management")
    float GetHumidityAtLocation(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Biome Management")
    bool IsLocationNearWater(const FVector& Location, float SearchRadius = 1000.0f);

    // Procedural generation
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateWorldBiomes();

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void SpawnWaterBodies();

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void PopulateBiomeVegetation(EWorld_BiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void SpawnBiomeSpecificDinosaurs();

    // Environmental effects
    UFUNCTION(BlueprintCallable, Category = "Environment")
    void UpdateEnvironmentalEffects();

    UFUNCTION(BlueprintCallable, Category = "Environment")
    void ApplyBiomeWeatherEffects(EWorld_BiomeType BiomeType);

    // Debug and visualization
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void DebugVisualizeBiomes();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void ClearDebugVisualization();

private:
    // Internal biome calculation
    float CalculateBiomeInfluence(const FVector& Location, const FWorld_BiomeData& BiomeData);
    FVector GetNearestWaterBody(const FVector& Location);
    void InitializeDefaultBiomes();
    void ValidateBiomeConfiguration();

    // Component references
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    USceneComponent* RootSceneComponent;

    // Runtime state
    UPROPERTY()
    bool bBiomesInitialized;

    UPROPERTY()
    float LastUpdateTime;
};