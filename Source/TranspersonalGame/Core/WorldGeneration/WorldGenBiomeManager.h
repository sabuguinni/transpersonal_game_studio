#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "../SharedTypes.h"
#include "WorldGenBiomeManager.generated.h"

UENUM(BlueprintType)
enum class EWorld_BiomeType : uint8
{
    Forest      UMETA(DisplayName = "Forest"),
    Plains      UMETA(DisplayName = "Plains"), 
    Mountains   UMETA(DisplayName = "Mountains"),
    Desert      UMETA(DisplayName = "Desert"),
    Coastal     UMETA(DisplayName = "Coastal"),
    Swamp       UMETA(DisplayName = "Swamp"),
    RiverValley UMETA(DisplayName = "River Valley"),
    Tundra      UMETA(DisplayName = "Tundra")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_BiomeParameters
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EWorld_BiomeType BiomeType = EWorld_BiomeType::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float ElevationMin = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float ElevationMax = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climate")
    float Temperature = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climate")
    float Humidity = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    float VegetationDensity = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    FString AmbientSoundProfile = TEXT("Forest_Ambient");

    FWorld_BiomeParameters()
    {
        BiomeType = EWorld_BiomeType::Forest;
        ElevationMin = 0.0f;
        ElevationMax = 1000.0f;
        Temperature = 20.0f;
        Humidity = 50.0f;
        VegetationDensity = 0.7f;
        AmbientSoundProfile = TEXT("Forest_Ambient");
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_AudioZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float Radius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    EWorld_BiomeType AssociatedBiome = EWorld_BiomeType::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TArray<FString> SoundLayers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float VolumeMultiplier = 1.0f;

    FWorld_AudioZone()
    {
        Location = FVector::ZeroVector;
        Radius = 1000.0f;
        AssociatedBiome = EWorld_BiomeType::Forest;
        VolumeMultiplier = 1.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorldGenBiomeManager : public AActor
{
    GENERATED_BODY()

public:
    AWorldGenBiomeManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Biome Management
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void InitializeBiomes();

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    EWorld_BiomeType GetBiomeAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    FWorld_BiomeParameters GetBiomeParameters(EWorld_BiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void SetBiomeParameters(EWorld_BiomeType BiomeType, const FWorld_BiomeParameters& Parameters);

    // Audio Zone Management
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void CreateAudioZone(const FVector& Location, float Radius, EWorld_BiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    FWorld_AudioZone GetAudioZoneAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void UpdateAudioForLocation(const FVector& PlayerLocation);

    // Procedural Generation
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateBiomeTransitions();

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void PlaceEnvironmentalAudio();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void DebugVisualizeBiomes();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    TMap<EWorld_BiomeType, FWorld_BiomeParameters> BiomeParametersMap;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TArray<FWorld_AudioZone> AudioZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation")
    float BiomeTransitionDistance = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation")
    int32 WorldSizeKm = 10;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float AudioUpdateFrequency = 0.5f;

private:
    float LastAudioUpdateTime = 0.0f;
    FVector LastPlayerLocation = FVector::ZeroVector;

    // Internal biome calculation methods
    float CalculateElevationInfluence(const FVector& Location) const;
    float CalculateDistanceFromWater(const FVector& Location) const;
    float CalculateTemperatureGradient(const FVector& Location) const;
};