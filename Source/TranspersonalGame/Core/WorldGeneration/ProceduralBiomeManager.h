#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "../SharedTypes.h"
#include "ProceduralBiomeManager.generated.h"

UENUM(BlueprintType)
enum class EWorld_BiomeType : uint8
{
    Forest      UMETA(DisplayName = "Forest"),
    Plains      UMETA(DisplayName = "Plains"), 
    Mountains   UMETA(DisplayName = "Mountains"),
    River       UMETA(DisplayName = "River"),
    Wetlands    UMETA(DisplayName = "Wetlands"),
    Desert      UMETA(DisplayName = "Desert"),
    Tundra      UMETA(DisplayName = "Tundra")
};

USTRUCT(BlueprintType)
struct FWorld_BiomeParameters
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EWorld_BiomeType BiomeType = EWorld_BiomeType::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Temperature = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Humidity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Elevation = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float FoliageDensity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float RockDensity = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FLinearColor BiomeColor = FLinearColor::Green;

    FWorld_BiomeParameters()
    {
        BiomeType = EWorld_BiomeType::Forest;
        Temperature = 20.0f;
        Humidity = 0.5f;
        Elevation = 0.0f;
        FoliageDensity = 0.5f;
        RockDensity = 0.3f;
        BiomeColor = FLinearColor::Green;
    }
};

USTRUCT(BlueprintType)
struct FWorld_AudioZoneData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundCue> AmbientSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float Volume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float Pitch = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float FadeInTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float FadeOutTime = 2.0f;

    FWorld_AudioZoneData()
    {
        Volume = 1.0f;
        Pitch = 1.0f;
        FadeInTime = 2.0f;
        FadeOutTime = 2.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AProceduralBiomeManager : public AActor
{
    GENERATED_BODY()

public:
    AProceduralBiomeManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Core biome management
    UFUNCTION(BlueprintCallable, Category = "Biome Management")
    void InitializeBiomes();

    UFUNCTION(BlueprintCallable, Category = "Biome Management")
    void GenerateBiomeAtLocation(const FVector& Location, EWorld_BiomeType BiomeType, float Radius = 5000.0f);

    UFUNCTION(BlueprintCallable, Category = "Biome Management")
    EWorld_BiomeType GetBiomeAtLocation(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Biome Management")
    FWorld_BiomeParameters GetBiomeParameters(EWorld_BiomeType BiomeType) const;

    // Environmental audio management
    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void SetupAudioZones();

    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void UpdateAmbientAudioForLocation(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void SetBiomeAmbientSound(EWorld_BiomeType BiomeType, USoundCue* SoundCue);

    // Weather integration
    UFUNCTION(BlueprintCallable, Category = "Weather")
    void UpdateWeatherEffects(float Temperature, float Humidity, float WindStrength);

    UFUNCTION(BlueprintCallable, Category = "Weather")
    void ApplyWeatherToBiome(EWorld_BiomeType BiomeType, const FWeatherState& WeatherState);

    // PCG integration
    UFUNCTION(BlueprintCallable, Category = "PCG Integration")
    void TriggerPCGGeneration();

    UFUNCTION(BlueprintCallable, Category = "PCG Integration")
    void RefreshBiomeGeneration(EWorld_BiomeType BiomeType);

protected:
    // Biome configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Configuration")
    TMap<EWorld_BiomeType, FWorld_BiomeParameters> BiomeParametersMap;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Configuration")
    TMap<EWorld_BiomeType, FWorld_AudioZoneData> BiomeAudioMap;

    // Audio components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio")
    TMap<EWorld_BiomeType, UAudioComponent*> BiomeAudioComponents;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio")
    UAudioComponent* CurrentAmbientAudio;

    // World state
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "World State")
    EWorld_BiomeType CurrentPlayerBiome;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "World State")
    FVector LastPlayerLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World State")
    float BiomeTransitionDistance = 1000.0f;

    // Performance settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float UpdateFrequency = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MaxGenerationDistance = 10000.0f;

private:
    // Internal methods
    void SetupDefaultBiomeParameters();
    void SetupDefaultAudioZones();
    float CalculateBiomeInfluence(const FVector& Location, const FVector& BiomeCenter, float BiomeRadius) const;
    void TransitionAmbientAudio(EWorld_BiomeType NewBiome);

    // Timers
    float LastUpdateTime;
    float AudioTransitionTimer;
    bool bIsTransitioningAudio;
};