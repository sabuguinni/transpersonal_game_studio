#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "Engine/World.h"
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

USTRUCT(BlueprintType)
struct FEnvArt_LightingSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor = FLinearColor(1.0f, 0.8f, 0.6f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity = 8.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FRotator SunRotation = FRotator(-15.0f, 45.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SkyLightColor = FLinearColor(0.4f, 0.6f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SkyLightIntensity = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FLinearColor FogColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);
};

/**
 * Manages atmospheric lighting, weather effects, and time of day transitions
 * Creates immersive prehistoric environment atmosphere
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEnvArt_AtmosphereManager : public AActor
{
    GENERATED_BODY()

public:
    AEnvArt_AtmosphereManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time System")
    EEnvArt_TimeOfDay CurrentTimeOfDay = EEnvArt_TimeOfDay::Morning;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time System")
    float DayDurationMinutes = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time System")
    bool bAutoAdvanceTime = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Presets")
    TMap<EEnvArt_TimeOfDay, FEnvArt_LightingSettings> LightingPresets;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime")
    float CurrentTimeNormalized = 0.5f;

    // References to scene lighting actors
    UPROPERTY(BlueprintReadOnly, Category = "Scene References")
    class ADirectionalLight* SunActor;

    UPROPERTY(BlueprintReadOnly, Category = "Scene References")
    class ASkyLight* SkyLightActor;

    UPROPERTY(BlueprintReadOnly, Category = "Scene References")
    class AVolumetricCloud* FogActor;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Atmosphere Control")
    void SetTimeOfDay(EEnvArt_TimeOfDay NewTimeOfDay, bool bInstant = false);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere Control")
    void SetTimeNormalized(float TimeValue, bool bInstant = false);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere Control")
    void ApplyLightingSettings(const FEnvArt_LightingSettings& Settings, bool bInstant = false);

    UFUNCTION(BlueprintCallable, Category = "Weather")
    void SetWeatherIntensity(float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere Control")
    void CreateAtmosphericParticles(const TArray<FVector>& Locations);

    UFUNCTION(BlueprintPure, Category = "Time System")
    EEnvArt_TimeOfDay GetCurrentTimeOfDay() const { return CurrentTimeOfDay; }

    UFUNCTION(BlueprintPure, Category = "Time System")
    float GetCurrentTimeNormalized() const { return CurrentTimeNormalized; }

protected:
    UFUNCTION(BlueprintImplementableEvent, Category = "Events")
    void OnTimeOfDayChanged(EEnvArt_TimeOfDay NewTimeOfDay);

    UFUNCTION(BlueprintImplementableEvent, Category = "Events")
    void OnWeatherChanged(float WeatherIntensity);

private:
    void InitializeLightingPresets();
    void FindSceneLightingActors();
    void UpdateLighting(float DeltaTime);
    void InterpolateLightingSettings(const FEnvArt_LightingSettings& From, const FEnvArt_LightingSettings& To, float Alpha);

    float TransitionTimer = 0.0f;
    float TransitionDuration = 5.0f;
    bool bIsTransitioning = false;
    FEnvArt_LightingSettings StartSettings;
    FEnvArt_LightingSettings TargetSettings;
};