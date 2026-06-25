#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/PostProcessVolume.h"
#include "CretaceousLightingManager.generated.h"

// ============================================================
// Cretaceous Day Phase Enum
// ============================================================
UENUM(BlueprintType)
enum class ELight_DayPhase : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Morning     UMETA(DisplayName = "Morning"),
    Midday      UMETA(DisplayName = "Midday"),
    GoldenHour  UMETA(DisplayName = "Golden Hour"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Night       UMETA(DisplayName = "Night"),
};

// ============================================================
// Lighting Preset Config Struct
// ============================================================
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_DayPhaseConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Phase")
    ELight_DayPhase Phase = ELight_DayPhase::GoldenHour;

    /** Sun pitch angle in degrees (negative = above horizon) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Sun")
    float SunPitch = -38.0f;

    /** Sun yaw angle in degrees */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Sun")
    float SunYaw = 45.0f;

    /** Sun intensity in lux */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Sun")
    float SunIntensity = 12.0f;

    /** Sun color (linear) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Sun")
    FLinearColor SunColor = FLinearColor(1.0f, 0.878f, 0.620f, 1.0f);

    /** Fog density */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Fog")
    float FogDensity = 0.02f;

    /** Fog inscattering color */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Fog")
    FLinearColor FogInscatterColor = FLinearColor(0.45f, 0.62f, 0.85f, 1.0f);

    /** Sky light intensity */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Sky")
    float SkyLightIntensity = 2.0f;

    /** Post process exposure bias */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|PostProcess")
    float ExposureBias = 0.3f;

    /** Bloom intensity */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|PostProcess")
    float BloomIntensity = 0.35f;
};

// ============================================================
// Cretaceous Lighting Manager Actor
// ============================================================
UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Cretaceous Lighting Manager"))
class TRANSPERSONALGAME_API ACretaceousLightingManager : public AActor
{
    GENERATED_BODY()

public:
    ACretaceousLightingManager();

    // --------------------------------------------------------
    // Day/Night Cycle Control
    // --------------------------------------------------------

    /** Current time of day (0.0 = midnight, 0.5 = noon, 1.0 = midnight) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|DayNight",
        meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float TimeOfDay = 0.65f; // Default: golden hour

    /** Speed of day/night cycle (1.0 = real-time, 100.0 = 100x faster) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|DayNight")
    float DayCycleSpeed = 50.0f;

    /** Whether the day/night cycle is active */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|DayNight")
    bool bDayCycleActive = false;

    // --------------------------------------------------------
    // Phase Presets
    // --------------------------------------------------------

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Presets")
    FLight_DayPhaseConfig GoldenHourPreset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Presets")
    FLight_DayPhaseConfig MorningPreset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Presets")
    FLight_DayPhaseConfig MiddayPreset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Presets")
    FLight_DayPhaseConfig NightPreset;

    // --------------------------------------------------------
    // Scene References (auto-discovered in BeginPlay)
    // --------------------------------------------------------

    UPROPERTY(BlueprintReadOnly, Category = "Lighting|References",
        meta = (AllowPrivateAccess = "true"))
    TObjectPtr<AActor> DirectionalLightActor;

    UPROPERTY(BlueprintReadOnly, Category = "Lighting|References",
        meta = (AllowPrivateAccess = "true"))
    TObjectPtr<AActor> SkyLightActor;

    UPROPERTY(BlueprintReadOnly, Category = "Lighting|References",
        meta = (AllowPrivateAccess = "true"))
    TObjectPtr<AActor> FogActor;

    UPROPERTY(BlueprintReadOnly, Category = "Lighting|References",
        meta = (AllowPrivateAccess = "true"))
    TObjectPtr<APostProcessVolume> PostProcessActor;

    // --------------------------------------------------------
    // Lumen Settings
    // --------------------------------------------------------

    /** Enable Lumen Global Illumination */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Lumen")
    bool bLumenGIEnabled = true;

    /** Enable Lumen Reflections */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Lumen")
    bool bLumenReflectionsEnabled = true;

    /** Lumen max trace distance (cm) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Lumen",
        meta = (ClampMin = "1000.0", ClampMax = "100000.0"))
    float LumenMaxTraceDistance = 20000.0f;

    // --------------------------------------------------------
    // Methods
    // --------------------------------------------------------

    /** Apply the current TimeOfDay to all lighting actors */
    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void ApplyCurrentTimeOfDay();

    /** Apply a specific preset immediately */
    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void ApplyPhasePreset(const FLight_DayPhaseConfig& Preset);

    /** Get the current day phase based on TimeOfDay */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Lighting")
    ELight_DayPhase GetCurrentDayPhase() const;

    /** Discover and cache all lighting actors in the level */
    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void DiscoverLightingActors();

    /** Apply golden hour preset (CallInEditor for quick testing) */
    UFUNCTION(CallInEditor, Category = "Lighting|Quick")
    void ApplyGoldenHour();

    /** Apply midday preset (CallInEditor for quick testing) */
    UFUNCTION(CallInEditor, Category = "Lighting|Quick")
    void ApplyMidday();

    /** Apply night preset (CallInEditor for quick testing) */
    UFUNCTION(CallInEditor, Category = "Lighting|Quick")
    void ApplyNight();

    /** Toggle day/night cycle on/off */
    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void ToggleDayCycle(bool bEnable);

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

#if WITH_EDITOR
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

private:
    /** Initialize default presets */
    void InitializeDefaultPresets();

    /** Interpolate between two phase configs */
    FLight_DayPhaseConfig InterpolatePhaseConfigs(
        const FLight_DayPhaseConfig& A,
        const FLight_DayPhaseConfig& B,
        float Alpha) const;

    /** Apply config to actual lighting components */
    void ApplyConfigToComponents(const FLight_DayPhaseConfig& Config);

    /** Compute sun pitch from time of day */
    float ComputeSunPitch(float InTimeOfDay) const;

    /** Compute sun color from time of day */
    FLinearColor ComputeSunColor(float InTimeOfDay) const;
};
