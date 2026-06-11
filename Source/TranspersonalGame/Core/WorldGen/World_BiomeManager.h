#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Landscape/Classes/Landscape.h"
#include "SharedTypes.h"
#include "World_BiomeManager.generated.h"

UENUM(BlueprintType)
enum class EWorld_BiomeType : uint8
{
    DenseForest     UMETA(DisplayName = "Dense Forest"),
    OpenPlains      UMETA(DisplayName = "Open Plains"), 
    RockyHighlands  UMETA(DisplayName = "Rocky Highlands"),
    SwampWetlands   UMETA(DisplayName = "Swamp Wetlands"),
    VolcanicRegion  UMETA(DisplayName = "Volcanic Region")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_BiomeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EWorld_BiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector Center;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Radius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float TreeDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    bool bHasUndergrowth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FLinearColor AmbientLighting;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Temperature;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Humidity;

    FWorld_BiomeData()
    {
        BiomeType = EWorld_BiomeType::DenseForest;
        Center = FVector::ZeroVector;
        Radius = 1000.0f;
        TreeDensity = 0.5f;
        bHasUndergrowth = true;
        AmbientLighting = FLinearColor::White;
        Temperature = 25.0f;
        Humidity = 0.6f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_LandmarkData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landmark")
    FString Name;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landmark")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landmark")
    FVector Scale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landmark")
    EWorld_BiomeType AssociatedBiome;

    FWorld_LandmarkData()
    {
        Name = TEXT("Unknown Landmark");
        Location = FVector::ZeroVector;
        Scale = FVector::OneVector;
        AssociatedBiome = EWorld_BiomeType::DenseForest;
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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome System")
    TArray<FWorld_BiomeData> BiomeZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome System")
    TArray<FWorld_LandmarkData> PrehistoricLandmarks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome System")
    float BiomeTransitionDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome System")
    bool bEnableDynamicWeather;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome System")
    float WeatherUpdateInterval;

    // Biome detection and management
    UFUNCTION(BlueprintCallable, Category = "Biome System")
    EWorld_BiomeType GetBiomeAtLocation(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    FWorld_BiomeData GetBiomeData(EWorld_BiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    float GetDistanceToBiomeCenter(const FVector& Location, EWorld_BiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    TArray<EWorld_BiomeType> GetNearbyBiomes(const FVector& Location, float SearchRadius) const;

    // Procedural generation
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Biome System")
    void GenerateAllBiomes();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Biome System")
    void GenerateBiome(EWorld_BiomeType BiomeType);

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Biome System")
    void ClearAllBiomes();

    // Landmark management
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Biome System")
    void GenerateLandmarks();

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    FWorld_LandmarkData GetNearestLandmark(const FVector& Location) const;

    // Environmental effects
    UFUNCTION(BlueprintCallable, Category = "Biome System")
    FLinearColor GetBiomeAmbientColor(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    float GetBiomeTemperature(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    float GetBiomeHumidity(const FVector& Location) const;

private:
    void InitializeDefaultBiomes();
    void InitializeDefaultLandmarks();
    void SpawnBiomeVegetation(const FWorld_BiomeData& BiomeData);
    void SpawnBiomeLighting(const FWorld_BiomeData& BiomeData);
    void SpawnBiomeTerrain(const FWorld_BiomeData& BiomeData);

    FTimerHandle WeatherUpdateTimer;
    void UpdateDynamicWeather();
};