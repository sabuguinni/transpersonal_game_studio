#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "LightingAtmosphereManager.generated.h"

/**
 * FLight_TimeOfDaySettings
 * Stores per-time-of-day lighting parameters for the Cretaceous world.
 * Prefix: Light_ (Rule 2 — unique type names)
 */
USTRUCT(BlueprintType)
struct FLight_TimeOfDaySettings
{
    GENERATED_BODY()

    /** Sun pitch angle in degrees. MUST be negative to illuminate terrain (memory: hugo_sun_pitch_negative_proven). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Sun")
    float SunPitch = -45.0f;

    /** Sun yaw angle in degrees. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Sun")
    float SunYaw = 45.0f;

    /** Directional light intensity in lux. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Sun")
    float SunIntensity = 10.0f;

    /** Sun colour — warm amber for golden hour. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Sun")
    FLinearColor SunColor = FLinearColor(1.0f, 0.92f, 0.78f, 1.0f);

    /** Sky light intensity for Lumen ambient fill. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Sky")
    float SkyLightIntensity = 1.5f;

    /** Volumetric fog density. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Fog")
    float FogDensity = 0.02f;

    /** Fog albedo colour — cool blue-white mist. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Fog")
    FLinearColor FogAlbedo = FLinearColor(0.78f, 0.86f, 1.0f, 1.0f);

    /** Time of day label for debugging. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Debug")
    FString TimeLabel = TEXT("GoldenHour");
};

/**
 * ELight_TimeOfDay
 * Enum for discrete time-of-day states driving lighting transitions.
 */
UENUM(BlueprintType)
enum class ELight_TimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Morning     UMETA(DisplayName = "Morning"),
    Midday      UMETA(DisplayName = "Midday"),
    Afternoon   UMETA(DisplayName = "Afternoon"),
    GoldenHour  UMETA(DisplayName = "Golden Hour"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Night       UMETA(DisplayName = "Night")
};

/**
 * ALightingAtmosphereManager
 * Manages the Cretaceous world's dynamic lighting, atmosphere, and volumetric fog.
 * Drives Lumen GI via DirectionalLight + SkyLight + ExponentialHeightFog.
 *
 * Placement: spawn one instance in MinPlayableMap. Tick drives day/night cycle.
 */
UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Lighting Atmosphere Manager"))
class TRANSPERSONALGAME_API ALightingAtmosphereManager : public AActor
{
    GENERATED_BODY()

public:
    ALightingAtmosphereManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    // ── Components ───────────────────────────────────────────────────────────

    /** Primary directional light — the Cretaceous sun. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting|Components",
              meta = (AllowPrivateAccess = "true"))
    UDirectionalLightComponent* SunLightComponent;

    /** Sky light for Lumen ambient GI. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting|Components",
              meta = (AllowPrivateAccess = "true"))
    USkyLightComponent* SkyLightComponent;

    /** Exponential height fog with volumetric fog enabled. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting|Components",
              meta = (AllowPrivateAccess = "true"))
    UExponentialHeightFogComponent* FogComponent;

    // ── Day/Night Cycle ───────────────────────────────────────────────────────

    /** Current time of day (0.0 = midnight, 0.5 = noon, 1.0 = midnight). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|DayNight")
    float TimeOfDayNormalized = 0.65f;  // Default: golden hour

    /** Speed multiplier for day/night cycle. 0 = frozen. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|DayNight")
    float DayNightSpeed = 0.0f;  // Frozen by default — set >0 to animate

    /** Current discrete time-of-day state. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting|DayNight")
    ELight_TimeOfDay CurrentTimeOfDay = ELight_TimeOfDay::GoldenHour;

    /** Lighting preset for current time of day. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|DayNight")
    FLight_TimeOfDaySettings CurrentSettings;

    // ── Weather ───────────────────────────────────────────────────────────────

    /** Fog density multiplier for weather effects (1.0 = clear, 3.0 = heavy fog). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Weather",
              meta = (ClampMin = "0.1", ClampMax = "5.0"))
    float WeatherFogMultiplier = 1.0f;

    /** Sun intensity multiplier for overcast weather (1.0 = clear, 0.3 = overcast). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Weather",
              meta = (ClampMin = "0.1", ClampMax = "1.0"))
    float WeatherSunMultiplier = 1.0f;

    // ── Methods ───────────────────────────────────────────────────────────────

    /** Apply current settings to all lighting components immediately. */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Lighting")
    void ApplyLightingSettings();

    /** Transition to a specific time of day preset. */
    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetTimeOfDay(ELight_TimeOfDay NewTimeOfDay);

    /** Get the sun pitch for a given normalized time (0-1). MUST return negative pitch. */
    UFUNCTION(BlueprintPure, Category = "Lighting")
    float GetSunPitchForTime(float NormalizedTime) const;

    /** Get the sun intensity for a given normalized time. */
    UFUNCTION(BlueprintPure, Category = "Lighting")
    float GetSunIntensityForTime(float NormalizedTime) const;

private:
    /** Update day/night cycle based on DeltaTime. */
    void TickDayNightCycle(float DeltaTime);

    /** Resolve discrete TimeOfDay enum from normalized time float. */
    ELight_TimeOfDay ResolveTimeOfDayEnum(float NormalizedTime) const;
};
