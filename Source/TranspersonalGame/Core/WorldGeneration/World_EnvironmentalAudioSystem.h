#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "World_EnvironmentalAudioSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_BiomeAudioData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    FString BiomeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    FVector Center;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    float Radius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    FString AudioAssetPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    float Volume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    float FadeDistance;

    FWorld_BiomeAudioData()
    {
        BiomeName = TEXT("Default");
        Center = FVector::ZeroVector;
        Radius = 1000.0f;
        AudioAssetPath = TEXT("");
        Volume = 1.0f;
        FadeDistance = 500.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_WeatherAudioData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Audio")
    FString WeatherType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Audio")
    FVector ZoneCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Audio")
    float ZoneRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Audio")
    FString AudioAssetPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Audio")
    float Intensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Audio")
    bool bIsActive;

    FWorld_WeatherAudioData()
    {
        WeatherType = TEXT("Clear");
        ZoneCenter = FVector::ZeroVector;
        ZoneRadius = 800.0f;
        AudioAssetPath = TEXT("");
        Intensity = 1.0f;
        bIsActive = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorld_EnvironmentalAudioSystem : public AActor
{
    GENERATED_BODY()

public:
    AWorld_EnvironmentalAudioSystem();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* RootMeshComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UAudioComponent* MasterAudioComponent;

    // Biome Audio System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    TArray<FWorld_BiomeAudioData> BiomeAudioZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    TArray<UAudioComponent*> BiomeAudioComponents;

    // Weather Audio System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Audio")
    TArray<FWorld_WeatherAudioData> WeatherAudioZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Audio")
    TArray<UAudioComponent*> WeatherAudioComponents;

    // Environmental Audio Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Audio")
    float GlobalAudioVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Audio")
    float AudioUpdateFrequency;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Audio")
    float MaxAudioDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Audio")
    bool bEnableDynamicAudioMixing;

    // Day/Night Cycle Audio
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Audio")
    float DayAudioVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Audio")
    float NightAudioVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Audio")
    float CurrentTimeOfDay;

    // Elevation-based Audio
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Elevation Audio")
    float LowlandAudioVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Elevation Audio")
    float MidlandAudioVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Elevation Audio")
    float HighlandAudioVolume;

public:
    // Biome Audio Management
    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void InitializeBiomeAudio();

    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void UpdateBiomeAudio(const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    FWorld_BiomeAudioData GetClosestBiome(const FVector& Location);

    // Weather Audio Management
    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void InitializeWeatherAudio();

    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void UpdateWeatherAudio(const FString& WeatherType, float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void ActivateWeatherZone(const FString& WeatherType, const FVector& Location);

    // Dynamic Audio Mixing
    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void UpdateAudioMixing(const FVector& PlayerLocation, float PlayerElevation);

    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void SetDayNightAudioMix(float TimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void SetElevationAudioMix(float Elevation);

    // Audio Zone Management
    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void CreateBiomeAudioZone(const FString& BiomeName, const FVector& Center, float Radius, const FString& AudioPath);

    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void CreateWeatherAudioZone(const FString& WeatherType, const FVector& Center, float Radius, const FString& AudioPath);

    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void RemoveAudioZone(const FString& ZoneName);

    // Audio Transition System
    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void StartAudioTransition(const FString& FromZone, const FString& ToZone, float TransitionTime);

    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void UpdateAudioTransitions(float DeltaTime);

    // Master Audio Control
    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void SetMasterEnvironmentalVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void EnableEnvironmentalAudio(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void ResetAllAudioZones();

private:
    float AudioUpdateTimer;
    TMap<FString, float> AudioTransitionStates;
    bool bIsInitialized;
};