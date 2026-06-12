#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/AtmosphericFogComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/PostProcessComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/AtmosphericFog.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/PostProcessVolume.h"
#include "EnvArt_AtmosphericManager.generated.h"

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
enum class EEnvArt_WeatherState : uint8
{
    Clear       UMETA(DisplayName = "Clear"),
    Overcast    UMETA(DisplayName = "Overcast"),
    Foggy       UMETA(DisplayName = "Foggy"),
    Stormy      UMETA(DisplayName = "Stormy"),
    Dusty       UMETA(DisplayName = "Dusty")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_AtmosphericSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity = 3.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FRotator SunRotation = FRotator(-15.0f, 225.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogHeightFalloff = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor FogInscatteringColor = FLinearColor(0.7f, 0.8f, 0.9f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float AtmosphericFogMultiplier = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float AtmosphericDensityMultiplier = 0.6f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEnvArt_AtmosphericManager : public AActor
{
    GENERATED_BODY()

public:
    AEnvArt_AtmosphericManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // === ATMOSPHERIC CONTROL ===
    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SetTimeOfDay(EEnvArt_TimeOfDay NewTimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SetWeatherState(EEnvArt_WeatherState NewWeatherState);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void ApplyAtmosphericSettings(const FEnvArt_AtmosphericSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void TransitionToTimeOfDay(EEnvArt_TimeOfDay TargetTime, float TransitionDuration = 5.0f);

    // === LIGHTING CONTROL ===
    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetSunAngle(float Pitch, float Yaw);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetSunIntensity(float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetSunColor(FLinearColor Color);

    // === FOG CONTROL ===
    UFUNCTION(BlueprintCallable, Category = "Fog")
    void SetFogDensity(float Density);

    UFUNCTION(BlueprintCallable, Category = "Fog")
    void SetFogColor(FLinearColor Color);

    UFUNCTION(BlueprintCallable, Category = "Fog")
    void EnableVolumetricFog(bool bEnable);

    // === PRESET ATMOSPHERES ===
    UFUNCTION(BlueprintCallable, Category = "Presets")
    void ApplyGoldenHourPreset();

    UFUNCTION(BlueprintCallable, Category = "Presets")
    void ApplyMorningMistPreset();

    UFUNCTION(BlueprintCallable, Category = "Presets")
    void ApplyStormyPreset();

    UFUNCTION(BlueprintCallable, Category = "Presets")
    void ApplyClearDayPreset();

protected:
    // === COMPONENT REFERENCES ===
    UPROPERTY(BlueprintReadOnly, Category = "Components")
    TObjectPtr<ADirectionalLight> DirectionalLightActor;

    UPROPERTY(BlueprintReadOnly, Category = "Components")
    TObjectPtr<AAtmosphericFog> AtmosphericFogActor;

    UPROPERTY(BlueprintReadOnly, Category = "Components")
    TObjectPtr<AExponentialHeightFog> HeightFogActor;

    UPROPERTY(BlueprintReadOnly, Category = "Components")
    TObjectPtr<APostProcessVolume> PostProcessVolumeActor;

    // === ATMOSPHERIC STATE ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
    EEnvArt_TimeOfDay CurrentTimeOfDay = EEnvArt_TimeOfDay::Afternoon;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
    EEnvArt_WeatherState CurrentWeatherState = EEnvArt_WeatherState::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    FEnvArt_AtmosphericSettings CurrentSettings;

    // === TRANSITION SYSTEM ===
    UPROPERTY(BlueprintReadOnly, Category = "Transition")
    bool bIsTransitioning = false;

    UPROPERTY(BlueprintReadOnly, Category = "Transition")
    float TransitionProgress = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Transition")
    float TransitionDuration = 5.0f;

    FEnvArt_AtmosphericSettings TransitionStartSettings;
    FEnvArt_AtmosphericSettings TransitionTargetSettings;

private:
    // === INTERNAL METHODS ===
    void FindAtmosphericActors();
    void UpdateLighting();
    void UpdateFog();
    void UpdatePostProcess();
    void ProcessTransition(float DeltaTime);
    FEnvArt_AtmosphericSettings GetPresetSettings(EEnvArt_TimeOfDay TimeOfDay, EEnvArt_WeatherState WeatherState);
};