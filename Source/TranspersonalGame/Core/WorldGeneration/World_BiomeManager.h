#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "../SharedTypes.h"
#include "World_BiomeManager.generated.h"

UENUM(BlueprintType)
enum class EWorld_BiomeType : uint8
{
    Coastal         UMETA(DisplayName = "Coastal"),
    TemperateForest UMETA(DisplayName = "Temperate Forest"),
    Grasslands      UMETA(DisplayName = "Grasslands"),
    Mountains       UMETA(DisplayName = "Mountains"),
    Wetlands        UMETA(DisplayName = "Wetlands"),
    Desert          UMETA(DisplayName = "Desert"),
    TropicalForest  UMETA(DisplayName = "Tropical Forest"),
    Tundra          UMETA(DisplayName = "Tundra")
};

USTRUCT(BlueprintType)
struct FWorld_BiomeParameters
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EWorld_BiomeType BiomeType = EWorld_BiomeType::TemperateForest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Temperature = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Humidity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Elevation = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float VegetationDensity = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FLinearColor BiomeColor = FLinearColor::Green;

    FWorld_BiomeParameters()
    {
        Temperature = 20.0f;
        Humidity = 0.5f;
        Elevation = 100.0f;
        VegetationDensity = 0.7f;
        BiomeColor = FLinearColor::Green;
    }
};

USTRUCT(BlueprintType)
struct FWorld_BiomeZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
    FVector Center = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
    float Radius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
    FWorld_BiomeParameters Parameters;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
    float BlendDistance = 1000.0f;

    FWorld_BiomeZone()
    {
        Center = FVector::ZeroVector;
        Radius = 5000.0f;
        BlendDistance = 1000.0f;
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

    // Biome Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    TArray<FWorld_BiomeZone> BiomeZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    float GlobalTemperature = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    float GlobalHumidity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    bool bEnableSeasonalChanges = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    float SeasonCycleLength = 300.0f; // seconds for full cycle

    // Core Functions
    UFUNCTION(BlueprintCallable, Category = "Biome")
    FWorld_BiomeParameters GetBiomeAtLocation(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    EWorld_BiomeType GetDominantBiomeType(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    void AddBiomeZone(const FVector& Center, float Radius, EWorld_BiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Biome")
    void RemoveBiomeZone(int32 ZoneIndex);

    UFUNCTION(BlueprintCallable, Category = "Biome")
    void UpdateSeasonalParameters(float SeasonProgress);

    UFUNCTION(BlueprintCallable, Category = "Biome")
    float CalculateBiomeInfluence(const FVector& Location, const FWorld_BiomeZone& Zone) const;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    void GenerateDefaultBiomes();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Biome")
    void RegenerateBiomes();

private:
    UPROPERTY()
    float CurrentSeasonTime = 0.0f;

    void InitializeDefaultBiomes();
    FWorld_BiomeParameters BlendBiomeParameters(const TArray<TPair<FWorld_BiomeParameters, float>>& BiomesWithWeights) const;
};