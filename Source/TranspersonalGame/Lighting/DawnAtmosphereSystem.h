#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyLightComponent.h"
#include "Engine/DirectionalLight.h"
#include "DawnAtmosphereSystem.generated.h"

// ============================================================
// Dawn/Sunrise Lighting Palette — Cretaceous Era
// Agent #08 — Lighting & Atmosphere | Cycle AUTO_20260630_006
// ============================================================

UENUM(BlueprintType)
enum class ELight_TimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn (Sunrise)"),
    Midday      UMETA(DisplayName = "Midday"),
    Dusk        UMETA(DisplayName = "Dusk (Sunset)"),
    Night       UMETA(DisplayName = "Night (Moonlight)"),
    Overcast    UMETA(DisplayName = "Overcast Storm")
};

USTRUCT(BlueprintType)
struct FLight_DawnPalette
{
    GENERATED_BODY()

    /** Sun pitch angle in degrees (negative = above horizon) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dawn|Sun")
    float SunPitchDegrees = -18.0f;

    /** Sun yaw angle for directional shadow casting */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dawn|Sun")
    float SunYawDegrees = 45.0f;

    /** Directional light intensity (lux) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dawn|Sun")
    float SunIntensity = 3.5f;

    /** Warm dawn orange-pink sun color */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dawn|Sun")
    FLinearColor SunColor = FLinearColor(1.0f, 0.78f, 0.55f, 1.0f);

    /** Fog density for morning mist */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dawn|Fog")
    float FogDensity = 0.035f;

    /** Fog height falloff */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dawn|Fog")
    float FogHeightFalloff = 0.2f;

    /** Dawn mist inscattering color — orange-amber */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dawn|Fog")
    FLinearColor FogInscatteringColor = FLinearColor(0.9f, 0.6f, 0.4f, 1.0f);

    /** Skylight ambient intensity */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dawn|Sky")
    float SkylightIntensity = 1.2f;

    /** Enable volumetric fog for god rays */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dawn|Fog")
    bool bVolumetricFog = true;

    /** Volumetric fog scattering distribution */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dawn|Fog")
    float VolumetricFogScatteringDistribution = 0.2f;
};

USTRUCT(BlueprintType)
struct FLight_AtmosphereState
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Atmosphere")
    ELight_TimeOfDay CurrentTimeOfDay = ELight_TimeOfDay::Dawn;

    UPROPERTY(BlueprintReadOnly, Category = "Atmosphere")
    float CurrentSunAngle = -18.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Atmosphere")
    float CurrentFogDensity = 0.035f;

    UPROPERTY(BlueprintReadOnly, Category = "Atmosphere")
    bool bIsTransitioning = false;

    UPROPERTY(BlueprintReadOnly, Category = "Atmosphere")
    float TransitionProgress = 0.0f;
};

/**
 * ADawnAtmosphereSystem
 * 
 * Controls the Cretaceous era dawn/sunrise lighting atmosphere.
 * Manages directional light, exponential height fog, sky atmosphere,
 * and Lumen settings for cinematic prehistoric dawn mood.
 * 
 * Designed for MinPlayableMap — place one instance in the level.
 * References existing DirectionalLight, SkyLight, and Fog actors.
 */
UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Dawn Atmosphere System"))
class TRANSPERSONALGAME_API ADawnAtmosphereSystem : public AActor
{
    GENERATED_BODY()

public:
    ADawnAtmosphereSystem();

    // === DAWN PALETTE SETTINGS ===

    /** Current dawn lighting palette — edit in Details panel */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Dawn Palette")
    FLight_DawnPalette DawnPalette;

    /** Current atmosphere state (read-only runtime info) */
    UPROPERTY(BlueprintReadOnly, Category = "Lighting|State")
    FLight_AtmosphereState AtmosphereState;

    /** Target time of day for this atmosphere controller */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Time of Day")
    ELight_TimeOfDay TargetTimeOfDay = ELight_TimeOfDay::Dawn;

    // === ACTOR REFERENCES ===

    /** Reference to the scene's DirectionalLight actor (auto-found if null) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|References")
    TObjectPtr<ADirectionalLight> DirectionalLightActor;

    /** Reference to the scene's SkyLight actor (auto-found if null) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|References")
    TObjectPtr<AActor> SkyLightActor;

    /** Reference to the scene's ExponentialHeightFog actor (auto-found if null) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|References")
    TObjectPtr<AActor> FogActor;

    // === LUMEN SETTINGS ===

    /** Enable Lumen Global Illumination */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Lumen")
    bool bEnableLumenGI = true;

    /** Enable Lumen Reflections */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Lumen")
    bool bEnableLumenReflections = true;

    /** Enable Volumetric Clouds */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Atmosphere")
    bool bEnableVolumetricClouds = true;

    // === TRANSITION SETTINGS ===

    /** Duration in seconds for time-of-day transitions */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Transitions", meta = (ClampMin = "1.0", ClampMax = "300.0"))
    float TransitionDuration = 30.0f;

    /** Whether to auto-cycle through time of day */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Transitions")
    bool bAutoCycleDayNight = false;

    /** Full day cycle duration in seconds (real-time) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Transitions", meta = (ClampMin = "60.0", ClampMax = "3600.0"))
    float FullDayCycleDuration = 600.0f;

    // === BLUEPRINT CALLABLE FUNCTIONS ===

    /** Apply the dawn palette immediately to all referenced light actors */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Lighting|Dawn")
    void ApplyDawnPalette();

    /** Transition to a new time of day over TransitionDuration seconds */
    UFUNCTION(BlueprintCallable, Category = "Lighting|Transitions")
    void TransitionToTimeOfDay(ELight_TimeOfDay NewTimeOfDay);

    /** Get the current atmosphere state */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Lighting|State")
    FLight_AtmosphereState GetAtmosphereState() const { return AtmosphereState; }

    /** Auto-find and cache light actor references in the level */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Lighting|Setup")
    void AutoFindLightActors();

    /** Apply Lumen console variable settings */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Lighting|Lumen")
    void ApplyLumenSettings();

    /** Get sun color for current time of day */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Lighting|Dawn")
    FLinearColor GetSunColorForTimeOfDay(ELight_TimeOfDay TimeOfDay) const;

    /** Get fog density for current time of day */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Lighting|Dawn")
    float GetFogDensityForTimeOfDay(ELight_TimeOfDay TimeOfDay) const;

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

#if WITH_EDITOR
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

private:
    /** Apply directional light properties from palette */
    void ApplyDirectionalLight(const FLight_DawnPalette& Palette);

    /** Apply fog properties from palette */
    void ApplyFog(const FLight_DawnPalette& Palette);

    /** Apply skylight properties from palette */
    void ApplySkyLight(const FLight_DawnPalette& Palette);

    /** Internal transition state */
    bool bIsTransitioning = false;
    float TransitionElapsed = 0.0f;
    ELight_TimeOfDay TransitionFrom = ELight_TimeOfDay::Dawn;
    ELight_TimeOfDay TransitionTo = ELight_TimeOfDay::Dawn;
    FLight_DawnPalette PaletteFrom;
    FLight_DawnPalette PaletteTo;

    /** Build palette for a given time of day */
    FLight_DawnPalette BuildPaletteForTimeOfDay(ELight_TimeOfDay TimeOfDay) const;

    /** Lerp between two palettes */
    FLight_DawnPalette LerpPalettes(const FLight_DawnPalette& A, const FLight_DawnPalette& B, float Alpha) const;

    /** Day cycle elapsed time */
    float DayCycleElapsed = 0.0f;
};
