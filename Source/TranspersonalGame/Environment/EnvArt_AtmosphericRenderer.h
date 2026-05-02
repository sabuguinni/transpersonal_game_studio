#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DirectionalLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/AudioComponent.h"
#include "Engine/World.h"
#include "EnvArt_AtmosphericRenderer.generated.h"

UENUM(BlueprintType)
enum class EEnvArt_TimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Morning     UMETA(DisplayName = "Morning"),
    Noon        UMETA(DisplayName = "Noon"),
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
struct TRANSPERSONALGAME_API FEnvArt_LightingSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FRotator SunRotation = FRotator(-30.0f, 45.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    bool bCastVolumetricShadows = true;

    FEnvArt_LightingSettings()
    {
        SunIntensity = 3.0f;
        SunColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);
        SunRotation = FRotator(-30.0f, 45.0f, 0.0f);
        bCastVolumetricShadows = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_FogSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogHeightFalloff = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor FogColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    bool bVolumetricFog = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float VolumetricFogScattering = 0.2f;

    FEnvArt_FogSettings()
    {
        FogDensity = 0.02f;
        FogHeightFalloff = 0.2f;
        FogColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);
        bVolumetricFog = true;
        VolumetricFogScattering = 0.2f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UEnvArt_AtmosphericRenderer : public UActorComponent
{
    GENERATED_BODY()

public:
    UEnvArt_AtmosphericRenderer();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Time and weather control
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    EEnvArt_TimeOfDay CurrentTimeOfDay = EEnvArt_TimeOfDay::Afternoon;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    EEnvArt_WeatherType CurrentWeather = EEnvArt_WeatherType::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float TimeTransitionSpeed = 1.0f;

    // Lighting settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FEnvArt_LightingSettings LightingSettings;

    // Fog settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FEnvArt_FogSettings FogSettings;

    // Particle effects
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particles")
    bool bEnableAtmosphericParticles = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particles")
    float ParticleIntensity = 1.0f;

    // Audio
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    bool bEnableAmbientAudio = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float AmbientVolume = 0.5f;

    // Blueprint callable functions
    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SetTimeOfDay(EEnvArt_TimeOfDay NewTimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SetWeatherType(EEnvArt_WeatherType NewWeather);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void UpdateLighting();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void UpdateFog();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void UpdateParticleEffects();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void UpdateAmbientAudio();

    // Editor functions
    UFUNCTION(CallInEditor, Category = "Atmosphere")
    void ApplyAtmosphericSettings();

    UFUNCTION(CallInEditor, Category = "Atmosphere")
    void ResetToDefaults();

private:
    // Internal functions
    void FindSceneActors();
    void ConfigureDirectionalLight();
    void ConfigureHeightFog();
    void SpawnParticleEffects();
    void SetupAmbientAudio();

    // Cached references
    UPROPERTY()
    ADirectionalLight* CachedSunLight;

    UPROPERTY()
    class AExponentialHeightFog* CachedHeightFog;

    UPROPERTY()
    TArray<class AParticleSystemActor*> ParticleEmitters;

    UPROPERTY()
    TArray<class AAmbientSound*> AmbientSounds;

    // Internal state
    float CurrentTimeProgress = 0.5f;
    bool bInitialized = false;
};