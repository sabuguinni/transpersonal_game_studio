#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/AudioComponent.h"
#include "Engine/TriggerBox.h"
#include "../SharedTypes.h"
#include "World_BiomeSystem.generated.h"

UENUM(BlueprintType)
enum class EWorld_BiomeType : uint8
{
    Forest      UMETA(DisplayName = "Forest"),
    Plains      UMETA(DisplayName = "Plains"),
    Rocky       UMETA(DisplayName = "Rocky"),
    Wetland     UMETA(DisplayName = "Wetland"),
    Water       UMETA(DisplayName = "Water"),
    Transition  UMETA(DisplayName = "Transition")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_BiomeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EWorld_BiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector Scale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float InfluenceRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float AmbientVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TArray<FString> AudioTags;

    FWorld_BiomeData()
    {
        BiomeType = EWorld_BiomeType::Forest;
        Location = FVector::ZeroVector;
        Scale = FVector::OneVector;
        InfluenceRadius = 1000.0f;
        AmbientVolume = 0.5f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_EnvironmentalAudio
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    FString AudioName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    EWorld_BiomeType AssociatedBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    FVector AudioLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float Volume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float AttenuationRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    bool bIsWeatherDependent;

    FWorld_EnvironmentalAudio()
    {
        AudioName = TEXT("DefaultAmbient");
        AssociatedBiome = EWorld_BiomeType::Forest;
        AudioLocation = FVector::ZeroVector;
        Volume = 0.5f;
        AttenuationRadius = 1000.0f;
        bIsWeatherDependent = false;
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
    UFUNCTION(BlueprintCallable, Category = "Biome System")
    void InitializeBiomes();

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    void CreateBiomeZone(const FWorld_BiomeData& BiomeData);

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    EWorld_BiomeType GetBiomeAtLocation(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    float GetBiomeInfluenceAtLocation(const FVector& Location, EWorld_BiomeType BiomeType) const;

    // Environmental audio
    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void SetupEnvironmentalAudio();

    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void CreateAudioSource(const FWorld_EnvironmentalAudio& AudioData);

    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void UpdateAudioForLocation(const FVector& PlayerLocation);

    // Weather integration
    UFUNCTION(BlueprintCallable, Category = "Weather")
    void SetWeatherIntensity(float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Weather")
    void UpdateWeatherAudio(float WeatherIntensity);

    // Terrain generation
    UFUNCTION(BlueprintCallable, Category = "Terrain")
    void GenerateTerrainForBiome(const FWorld_BiomeData& BiomeData);

    UFUNCTION(BlueprintCallable, Category = "Terrain")
    void CreateWaterBodies();

    UFUNCTION(BlueprintCallable, Category = "Terrain")
    void PlaceVegetation();

protected:
    // Biome data storage
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome System")
    TArray<FWorld_BiomeData> BiomeZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Audio")
    TArray<FWorld_EnvironmentalAudio> AudioSources;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Audio")
    TArray<AActor*> SpawnedAudioActors;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float BiomeTransitionDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float AudioUpdateFrequency;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bAutoGenerateOnBeginPlay;

    // Weather state
    UPROPERTY(BlueprintReadOnly, Category = "Weather")
    float CurrentWeatherIntensity;

    UPROPERTY(BlueprintReadOnly, Category = "Weather")
    float LastAudioUpdateTime;

private:
    // Internal helper methods
    void CreateBiomeTerrain(const FWorld_BiomeData& BiomeData);
    void SetupBiomeAudio(const FWorld_BiomeData& BiomeData);
    void CreateTransitionZones();
    float CalculateDistanceInfluence(const FVector& Location, const FVector& BiomeCenter, float Radius) const;
};