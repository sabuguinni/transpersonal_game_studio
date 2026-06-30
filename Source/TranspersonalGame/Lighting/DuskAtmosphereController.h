#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyLightComponent.h"
#include "DuskAtmosphereController.generated.h"

/**
 * FLight_DuskPaletteSettings
 * Dusk/sunset lighting palette configuration for the Cretaceous prehistoric world.
 * Controls sun angle, color temperature, fog density, and volumetric atmosphere.
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_DuskPaletteSettings
{
    GENERATED_BODY()

    /** Sun pitch angle for dusk (low horizon, -10 to -20 degrees) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Dusk")
    float SunPitchAngle = -12.0f;

    /** Sun yaw for directional variation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Dusk")
    float SunYawAngle = 45.0f;

    /** Sun intensity at dusk (warm, slightly dimmer than noon) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Dusk")
    float SunIntensity = 2.5f;

    /** Sun color — warm orange at dusk */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Dusk")
    FLinearColor SunColor = FLinearColor(1.0f, 0.55f, 0.18f, 1.0f);

    /** Color temperature in Kelvin (3200K = warm sunset) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Dusk")
    float ColorTemperatureKelvin = 3200.0f;

    /** Fog density — moderate amber haze */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Dusk|Fog")
    float FogDensity = 0.06f;

    /** Fog inscattering color — warm amber */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Dusk|Fog")
    FLinearColor FogInscatteringColor = FLinearColor(0.9f, 0.45f, 0.12f, 1.0f);

    /** Fog max opacity */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Dusk|Fog")
    float FogMaxOpacity = 0.85f;

    /** Sky light intensity at dusk */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Dusk|Sky")
    float SkyLightIntensity = 1.2f;

    /** Sky light tint — warm amber sky */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Dusk|Sky")
    FLinearColor SkyLightColor = FLinearColor(0.95f, 0.75f, 0.55f, 1.0f);

    /** Enable volumetric fog for god rays */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Dusk|Volumetric")
    bool bEnableVolumetricFog = true;

    /** Volumetric fog scattering distribution */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Dusk|Volumetric")
    float VolumetricFogScatteringDistribution = 0.4f;

    /** Volumetric fog albedo — warm amber particles */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Dusk|Volumetric")
    FLinearColor VolumetricFogAlbedo = FLinearColor(0.85f, 0.65f, 0.35f, 1.0f);

    /** Volumetric fog extinction scale */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Dusk|Volumetric")
    float VolumetricFogExtinctionScale = 1.2f;
};

/**
 * ELight_TimeOfDay
 * Enumeration of the main time-of-day lighting states for the Cretaceous world.
 */
UENUM(BlueprintType)
enum class ELight_TimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Morning     UMETA(DisplayName = "Morning"),
    Noon        UMETA(DisplayName = "Noon"),
    Afternoon   UMETA(DisplayName = "Afternoon"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Night       UMETA(DisplayName = "Night"),
    Storm       UMETA(DisplayName = "Storm"),
    Overcast    UMETA(DisplayName = "Overcast")
};

/**
 * ADuskAtmosphereController
 * Runtime lighting controller for the Cretaceous prehistoric world.
 * Manages dusk/sunset palette, day-night cycle transitions, and volumetric atmosphere.
 * Designed for Unreal Engine 5 Lumen global illumination pipeline.
 *
 * Placement: Place one instance in the persistent level.
 * The controller auto-discovers DirectionalLight, SkyLight, and ExponentialHeightFog actors.
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup = "TranspersonalGame|Lighting")
class TRANSPERSONALGAME_API ADuskAtmosphereController : public AActor
{
    GENERATED_BODY()

public:
    ADuskAtmosphereController();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // ─── Time of Day ───────────────────────────────────────────────────────────

    /** Current time of day (0.0 = midnight, 0.5 = noon, 1.0 = midnight) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|TimeOfDay",
        meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float TimeOfDayNormalized = 0.75f;  // Default: dusk

    /** Speed of the day-night cycle (full cycle per real-world second) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|TimeOfDay")
    float DayCycleSpeed = 0.0f;  // 0 = static, >0 = animated

    /** Whether to animate the day-night cycle in real-time */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|TimeOfDay")
    bool bAnimateDayCycle = false;

    /** Current time of day enum state */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting|TimeOfDay")
    ELight_TimeOfDay CurrentTimeOfDay = ELight_TimeOfDay::Dusk;

    // ─── Dusk Palette ──────────────────────────────────────────────────────────

    /** Dusk/sunset lighting palette settings */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palettes")
    FLight_DuskPaletteSettings DuskPalette;

    // ─── Scene References (auto-discovered) ───────────────────────────────────

    /** Reference to the scene's directional light (sun/moon) */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting|References")
    TObjectPtr<ADirectionalLight> SceneSun;

    /** Reference to the scene's sky light */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting|References")
    TObjectPtr<ASkyLight> SceneSkyLight;

    /** Reference to the scene's exponential height fog */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting|References")
    TObjectPtr<AExponentialHeightFog> SceneFog;

    // ─── Functions ─────────────────────────────────────────────────────────────

    /** Apply the dusk palette immediately */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Lighting|Palettes")
    void ApplyDuskPalette();

    /** Apply a specific time of day palette */
    UFUNCTION(BlueprintCallable, Category = "Lighting|TimeOfDay")
    void SetTimeOfDay(ELight_TimeOfDay NewTimeOfDay);

    /** Get the current sun direction vector */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Lighting|TimeOfDay")
    FVector GetSunDirection() const;

    /** Get normalized sun elevation (0=horizon, 1=zenith, -1=nadir) */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Lighting|TimeOfDay")
    float GetSunElevationNormalized() const;

    /** Auto-discover scene lighting actors */
    UFUNCTION(BlueprintCallable, Category = "Lighting|Setup")
    void DiscoverSceneLightingActors();

    /** Enable/disable volumetric fog god rays */
    UFUNCTION(BlueprintCallable, Category = "Lighting|Volumetric")
    void SetVolumetricFogEnabled(bool bEnabled);

protected:
    virtual void OnConstruction(const FTransform& Transform) override;

private:
    /** Internal: apply sun rotation and color for current time */
    void ApplySunSettings(float PitchDeg, float YawDeg, float Intensity, FLinearColor Color, float TempKelvin);

    /** Internal: apply fog settings */
    void ApplyFogSettings(float Density, FLinearColor InscatterColor, float MaxOpacity, bool bVolumetric);

    /** Internal: apply sky light settings */
    void ApplySkyLightSettings(float Intensity, FLinearColor Color);

    /** Accumulated time for day cycle animation */
    float DayCycleAccumulator = 0.0f;
};
