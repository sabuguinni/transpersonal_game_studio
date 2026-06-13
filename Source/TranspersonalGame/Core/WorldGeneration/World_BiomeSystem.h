#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "SharedTypes.h"
#include "World_BiomeSystem.generated.h"

UENUM(BlueprintType)
enum class EWorld_BiomeType : uint8
{
    TropicalForest   UMETA(DisplayName = "Tropical Forest"),
    DesertCanyon     UMETA(DisplayName = "Desert Canyon"),
    Swampland        UMETA(DisplayName = "Swampland"),
    RockyPlains      UMETA(DisplayName = "Rocky Plains"),
    VolcanicRegion   UMETA(DisplayName = "Volcanic Region"),
    CoastalBeach     UMETA(DisplayName = "Coastal Beach")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_BiomeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EWorld_BiomeType BiomeType = EWorld_BiomeType::TropicalForest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FString BiomeName = "Unknown Biome";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FLinearColor FogColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Temperature = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Humidity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<TSoftObjectPtr<UStaticMesh>> VegetationMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<TSoftObjectPtr<UMaterialInterface>> TerrainMaterials;

    FWorld_BiomeData()
    {
        BiomeType = EWorld_BiomeType::TropicalForest;
        BiomeName = "Default Biome";
        FogColor = FLinearColor(0.7f, 0.8f, 0.9f, 1.0f);
        FogDensity = 0.02f;
        Temperature = 25.0f;
        Humidity = 0.5f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UWorld_BiomeSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UWorld_BiomeSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome System")
    TArray<FWorld_BiomeData> BiomeDefinitions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome System")
    float BiomeTransitionRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome System")
    float WeatherUpdateInterval = 30.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Biome System")
    EWorld_BiomeType CurrentBiome = EWorld_BiomeType::TropicalForest;

    UPROPERTY(BlueprintReadOnly, Category = "Biome System")
    float CurrentTemperature = 25.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Biome System")
    float CurrentHumidity = 0.5f;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    EWorld_BiomeType GetBiomeAtLocation(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    FWorld_BiomeData GetBiomeData(EWorld_BiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    void UpdateWeatherForBiome(EWorld_BiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    void TransitionToBiome(EWorld_BiomeType NewBiome, float TransitionTime = 5.0f);

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    TArray<FVector> GetBiomeCenters() const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    void SpawnBiomeVegetation(const FVector& Location, float Radius, EWorld_BiomeType BiomeType);

private:
    UPROPERTY()
    float WeatherTimer = 0.0f;

    UPROPERTY()
    TMap<EWorld_BiomeType, FVector> BiomeCenterLocations;

    void InitializeBiomeLocations();
    void UpdateCurrentBiome();
    FVector GetPlayerLocation() const;
};