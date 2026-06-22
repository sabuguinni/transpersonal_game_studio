#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SharedTypes.h"
#include "BiomeManager.generated.h"

UENUM(BlueprintType)
enum class EEng_BiomeType : uint8
{
    Forest      UMETA(DisplayName = "Forest"),
    Plains      UMETA(DisplayName = "Plains"),
    Swamp       UMETA(DisplayName = "Swamp"),
    Volcanic    UMETA(DisplayName = "Volcanic"),
    Coastal     UMETA(DisplayName = "Coastal"),
    Desert      UMETA(DisplayName = "Desert"),
    Jungle      UMETA(DisplayName = "Jungle"),
    Tundra      UMETA(DisplayName = "Tundra"),
};

USTRUCT(BlueprintType)
struct FEng_BiomeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EEng_BiomeType BiomeType = EEng_BiomeType::Plains;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Temperature = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Humidity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float VegetationDensity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float DinosaurSpawnWeight = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FLinearColor FogColor = FLinearColor(0.5f, 0.6f, 0.7f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FString BiomeName = TEXT("Plains");
};

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API ABiomeManager : public AActor
{
    GENERATED_BODY()

public:
    ABiomeManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    /** Returns biome data at world position */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    FEng_BiomeData GetBiomeAtLocation(FVector WorldLocation) const;

    /** Returns the dominant biome type at location */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    EEng_BiomeType GetBiomeTypeAtLocation(FVector WorldLocation) const;

    /** Returns temperature modifier at location */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    float GetTemperatureAtLocation(FVector WorldLocation) const;

    /** Returns humidity at location */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    float GetHumidityAtLocation(FVector WorldLocation) const;

    /** Registers a biome zone */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    void RegisterBiomeZone(FVector Center, float Radius, FEng_BiomeData BiomeData);

    /** Returns all registered biome zones count */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    int32 GetBiomeZoneCount() const;

    /** Noise-based biome seed */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Generation")
    int32 BiomeSeed = 42;

    /** World scale for biome noise */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Generation")
    float BiomeScale = 5000.0f;

    /** Default biome when no zone matches */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Generation")
    EEng_BiomeType DefaultBiome = EEng_BiomeType::Plains;

    /** All registered biome data */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Data")
    TArray<FEng_BiomeData> BiomeDefinitions;

private:
    struct FBiomeZone
    {
        FVector Center;
        float Radius;
        FEng_BiomeData Data;
    };

    TArray<FBiomeZone> RegisteredZones;

    float SampleNoise(float X, float Y) const;
    EEng_BiomeType ClassifyBiome(float Temperature, float Humidity) const;
};
