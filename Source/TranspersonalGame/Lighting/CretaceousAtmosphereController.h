#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "Components/SkyLightComponent.h"
#include "Engine/World.h"
#include "TranspersonalGame/TranspersonalGame.h"
#include "CretaceousAtmosphereController.generated.h"

UENUM(BlueprintType)
enum class ELight_TimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Morning     UMETA(DisplayName = "Morning"),
    Midday      UMETA(DisplayName = "Midday"),
    Afternoon   UMETA(DisplayName = "Afternoon"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Night       UMETA(DisplayName = "Night")
};

USTRUCT(BlueprintType)
struct FLight_AtmosphereSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    float SunIntensity = 15.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    FLinearColor SunColor = FLinearColor(1.0f, 0.95f, 0.85f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    float SunAngle = -45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float RayleighScattering = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float MieScattering = 0.003996f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor FogColor = FLinearColor(0.5f, 0.6f, 0.8f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky")
    float SkyLightIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky")
    FLinearColor SkyLightColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);
};

/**
 * Controller for realistic Cretaceous period atmospheric lighting
 * Manages day/night cycle, weather, and tropical environment lighting
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACretaceousAtmosphereController : public AActor
{
    GENERATED_BODY()

public:
    ACretaceousAtmosphereController();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // === ATMOSPHERIC COMPONENTS ===
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Atmosphere Settings")
    FLight_AtmosphereSettings CurrentSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Cycle")
    bool bEnableDayNightCycle = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Cycle", meta = (ClampMin = "0.1", ClampMax = "10.0"))
    float DayCycleDuration = 1200.0f; // 20 minutes in seconds

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Cycle", meta = (ClampMin = "0.0", ClampMax = "24.0"))
    float CurrentTimeOfDay = 12.0f; // Start at noon

    UPROPERTY(BlueprintReadOnly, Category = "Day/Night Cycle")
    ELight_TimeOfDay CurrentPeriod = ELight_TimeOfDay::Midday;

    // === LIGHTING REFERENCES ===

    UPROPERTY(BlueprintReadOnly, Category = "Lighting Components")
    class ADirectionalLight* SunActor;

    UPROPERTY(BlueprintReadOnly, Category = "Lighting Components")
    class ASkyAtmosphere* AtmosphereActor;

    UPROPERTY(BlueprintReadOnly, Category = "Lighting Components")
    class AExponentialHeightFog* FogActor;

    UPROPERTY(BlueprintReadOnly, Category = "Lighting Components")
    class AVolumetricCloud* CloudActor;

    UPROPERTY(BlueprintReadOnly, Category = "Lighting Components")
    class ASkyLight* SkyLightActor;

public:
    // === BLUEPRINT FUNCTIONS ===

    UFUNCTION(BlueprintCallable, Category = "Atmosphere Control")
    void SetTimeOfDay(float TimeInHours);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere Control")
    void SetAtmosphereSettings(const FLight_AtmosphereSettings& NewSettings);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere Control")
    void ApplyCretaceousPreset();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere Control")
    void FixApocalypticAtmosphere();

    UFUNCTION(BlueprintPure, Category = "Atmosphere Control")
    float GetCurrentTimeOfDay() const { return CurrentTimeOfDay; }

    UFUNCTION(BlueprintPure, Category = "Atmosphere Control")
    ELight_TimeOfDay GetCurrentPeriod() const { return CurrentPeriod; }

protected:
    // === INTERNAL FUNCTIONS ===

    void FindAtmosphericActors();
    void UpdateDayNightCycle(float DeltaTime);
    void UpdateSunPosition();
    void UpdateAtmosphereColors();
    void UpdateFogSettings();
    ELight_TimeOfDay CalculateTimeOfDayPeriod(float TimeHours) const;
    FLight_AtmosphereSettings InterpolateSettingsForTime(float TimeHours) const;

private:
    // === PRESET SETTINGS ===
    FLight_AtmosphereSettings DawnSettings;
    FLight_AtmosphereSettings MiddaySettings;
    FLight_AtmosphereSettings DuskSettings;
    FLight_AtmosphereSettings NightSettings;

    void InitializePresetSettings();
};