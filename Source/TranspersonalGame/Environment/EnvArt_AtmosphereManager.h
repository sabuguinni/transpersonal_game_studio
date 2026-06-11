#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/AtmosphericFogComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/AudioComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/AtmosphericFog.h"
#include "Engine/ExponentialHeightFog.h"
#include "Sound/AmbientSound.h"
#include "Particles/Emitter.h"
#include "EnvArt_AtmosphereManager.generated.h"

UENUM(BlueprintType)
enum class EEnvArt_TimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Morning     UMETA(DisplayName = "Morning"),
    Midday      UMETA(DisplayName = "Midday"),
    Afternoon   UMETA(DisplayName = "Afternoon"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Night       UMETA(DisplayName = "Night")
};

UENUM(BlueprintType)
enum class EEnvArt_WeatherType : uint8
{
    Clear       UMETA(DisplayName = "Clear"),
    Overcast    UMETA(DisplayName = "Overcast"),
    Foggy       UMETA(DisplayName = "Foggy"),
    Stormy      UMETA(DisplayName = "Stormy"),
    Misty       UMETA(DisplayName = "Misty")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_AtmosphereSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor = FLinearColor(1.0f, 0.8f, 0.6f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity = 3.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FRotator SunRotation = FRotator(-15.0f, 45.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogHeightFalloff = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor FogInscatteringColor = FLinearColor(0.7f, 0.8f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float SunMultiplier = 1.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float AtmosphereFogMultiplier = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float DensityMultiplier = 0.6f;

    FEnvArt_AtmosphereSettings()
    {
        // Default constructor with preset values
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_AmbientSoundZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
    float Radius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
    FString SoundType = TEXT("Forest");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
    float Volume = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
    bool bIsActive = true;

    FEnvArt_AmbientSoundZone()
    {
        // Default constructor
    }
};

/**
 * Environment Artist Atmosphere Manager
 * Manages dynamic lighting, fog, weather, and ambient audio for the Cretaceous environment
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEnvArt_AtmosphereManager : public AActor
{
    GENERATED_BODY()

public:
    AEnvArt_AtmosphereManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // === LIGHTING SYSTEM ===
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    EEnvArt_TimeOfDay CurrentTimeOfDay = EEnvArt_TimeOfDay::Afternoon;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    float DayDurationMinutes = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    bool bEnableDynamicTimeOfDay = true;

    // === WEATHER SYSTEM ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    EEnvArt_WeatherType CurrentWeather = EEnvArt_WeatherType::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WeatherTransitionSpeed = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    bool bEnableDynamicWeather = false;

    // === ATMOSPHERE SETTINGS ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FEnvArt_AtmosphereSettings DawnSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FEnvArt_AtmosphereSettings DaySettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FEnvArt_AtmosphereSettings DuskSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FEnvArt_AtmosphereSettings NightSettings;

    // === AMBIENT AUDIO ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ambient Audio")
    TArray<FEnvArt_AmbientSoundZone> AmbientSoundZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ambient Audio")
    float MasterAmbientVolume = 1.0f;

    // === PARTICLE EFFECTS ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particles")
    bool bEnableAtmosphericParticles = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particles")
    float ParticleIntensity = 1.0f;

private:
    // Internal references to world lighting actors
    UPROPERTY()
    class ADirectionalLight* DirectionalLight;

    UPROPERTY()
    class AAtmosphericFog* AtmosphericFog;

    UPROPERTY()
    class AExponentialHeightFog* ExponentialHeightFog;

    UPROPERTY()
    TArray<class AAmbientSound*> AmbientSounds;

    UPROPERTY()
    TArray<class AEmitter*> ParticleEmitters;

    // Time tracking
    float CurrentTimeOfDayFloat = 0.5f; // 0.0 = midnight, 0.5 = noon, 1.0 = midnight
    float WeatherTransitionTimer = 0.0f;

public:
    // === PUBLIC INTERFACE ===
    
    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SetTimeOfDay(EEnvArt_TimeOfDay NewTimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SetWeather(EEnvArt_WeatherType NewWeather);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void ApplyAtmosphereSettings(const FEnvArt_AtmosphereSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    FEnvArt_AtmosphereSettings GetCurrentAtmosphereSettings() const;

    UFUNCTION(BlueprintCallable, Category = "Ambient Audio")
    void AddAmbientSoundZone(const FEnvArt_AmbientSoundZone& SoundZone);

    UFUNCTION(BlueprintCallable, Category = "Ambient Audio")
    void RemoveAmbientSoundZone(int32 ZoneIndex);

    UFUNCTION(BlueprintCallable, Category = "Ambient Audio")
    void SetMasterAmbientVolume(float NewVolume);

    UFUNCTION(BlueprintCallable, Category = "Particles")
    void SetParticleIntensity(float NewIntensity);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void RefreshAtmosphere();

protected:
    // === INTERNAL METHODS ===
    
    UFUNCTION()
    void FindWorldLightingActors();

    UFUNCTION()
    void UpdateTimeOfDay(float DeltaTime);

    UFUNCTION()
    void UpdateWeather(float DeltaTime);

    UFUNCTION()
    void UpdateLighting();

    UFUNCTION()
    void UpdateFog();

    UFUNCTION()
    void UpdateAmbientAudio();

    UFUNCTION()
    void UpdateParticleEffects();

    UFUNCTION()
    FEnvArt_AtmosphereSettings InterpolateAtmosphereSettings(
        const FEnvArt_AtmosphereSettings& SettingsA,
        const FEnvArt_AtmosphereSettings& SettingsB,
        float Alpha
    ) const;

    UFUNCTION()
    EEnvArt_TimeOfDay GetTimeOfDayFromFloat(float TimeFloat) const;
};