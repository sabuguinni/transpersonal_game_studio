#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "Components/NiagaraComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/VolumetricCloud.h"
#include "NiagaraSystem.h"
#include "AtmosphericManager.generated.h"

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
    Fog         UMETA(DisplayName = "Fog"),
    Rain        UMETA(DisplayName = "Rain"),
    Storm       UMETA(DisplayName = "Storm")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_LightingSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunElevation = -30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunAzimuth = 225.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float LightIntensity = 3.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor LightColor = FLinearColor(1.0f, 0.85f, 0.6f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float VolumetricScattering = 2.0f;

    FEnvArt_LightingSettings()
    {
        SunElevation = -30.0f;
        SunAzimuth = 225.0f;
        LightIntensity = 3.5f;
        LightColor = FLinearColor(1.0f, 0.85f, 0.6f, 1.0f);
        VolumetricScattering = 2.0f;
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
    FLinearColor FogColor = FLinearColor(0.7f, 0.8f, 0.9f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    bool bVolumetricFog = true;

    FEnvArt_FogSettings()
    {
        FogDensity = 0.02f;
        FogHeightFalloff = 0.2f;
        FogColor = FLinearColor(0.7f, 0.8f, 0.9f, 1.0f);
        bVolumetricFog = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAtmosphericManager : public AActor
{
    GENERATED_BODY()

public:
    AAtmosphericManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Core atmospheric components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Atmosphere", meta = (AllowPrivateAccess = "true"))
    class USceneComponent* RootSceneComponent;

    // Time of day system
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    EEnvArt_TimeOfDay CurrentTimeOfDay = EEnvArt_TimeOfDay::Afternoon;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    float TimeOfDaySpeed = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    bool bAutoAdvanceTime = false;

    // Weather system
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    EEnvArt_WeatherType CurrentWeather = EEnvArt_WeatherType::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WeatherTransitionSpeed = 1.0f;

    // Lighting settings for each time of day
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    TMap<EEnvArt_TimeOfDay, FEnvArt_LightingSettings> TimeOfDayLighting;

    // Fog settings for each weather type
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    TMap<EEnvArt_WeatherType, FEnvArt_FogSettings> WeatherFogSettings;

    // Particle effects
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particles")
    TSoftObjectPtr<UNiagaraSystem> DustParticleSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particles")
    TSoftObjectPtr<UNiagaraSystem> PollenParticleSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particles")
    TSoftObjectPtr<UNiagaraSystem> RainParticleSystem;

    // Active particle components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Particles")
    TArray<UNiagaraComponent*> ActiveParticleComponents;

    // Blueprint callable functions
    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SetTimeOfDay(EEnvArt_TimeOfDay NewTimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SetWeather(EEnvArt_WeatherType NewWeather);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void UpdateLighting();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void UpdateFog();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void UpdateParticleEffects();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void CreateAtmosphericParticles(FVector Location, float Radius = 1000.0f);

    // Editor functions
    UFUNCTION(CallInEditor, Category = "Atmosphere")
    void ApplyGoldenHourLighting();

    UFUNCTION(CallInEditor, Category = "Atmosphere")
    void CreateFogSystem();

    UFUNCTION(CallInEditor, Category = "Atmosphere")
    void SpawnParticleEffects();

private:
    // Internal time tracking
    float CurrentTimeValue = 0.0f;
    float WeatherTransitionProgress = 0.0f;

    // References to scene actors
    UPROPERTY()
    ADirectionalLight* MainDirectionalLight;

    UPROPERTY()
    AExponentialHeightFog* HeightFogActor;

    UPROPERTY()
    AVolumetricCloud* VolumetricCloudActor;

    // Helper functions
    void FindSceneActors();
    void InitializeTimeOfDaySettings();
    void InitializeWeatherSettings();
    FEnvArt_LightingSettings InterpolateLightingSettings(const FEnvArt_LightingSettings& A, const FEnvArt_LightingSettings& B, float Alpha);
    FEnvArt_FogSettings InterpolateFogSettings(const FEnvArt_FogSettings& A, const FEnvArt_FogSettings& B, float Alpha);
};