#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "DawnAtmosphereController.generated.h"

/**
 * Lighting palette presets for the prehistoric world's day/night cycle.
 * Each preset defines a distinct emotional and visual moment in the day.
 */
UENUM(BlueprintType)
enum class ELight_DayPalette : uint8
{
    Dawn        UMETA(DisplayName = "Dawn — Cool Mist"),
    Morning     UMETA(DisplayName = "Morning — Warm Rise"),
    Midday      UMETA(DisplayName = "Midday — Harsh Overhead"),
    Afternoon   UMETA(DisplayName = "Afternoon — Golden Hour"),
    Dusk        UMETA(DisplayName = "Dusk — Deep Orange"),
    Night       UMETA(DisplayName = "Night — Moonlit Dark"),
    Overcast    UMETA(DisplayName = "Overcast — Storm Brewing"),
    StormActive UMETA(DisplayName = "Storm — Active Lightning")
};

/**
 * Data container for a single lighting palette configuration.
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_PaletteConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    float SunPitch = -25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    float SunYaw = 180.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    float SunIntensity = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    FLinearColor SunColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky")
    float SkyIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky")
    FLinearColor SkyTint = FLinearColor(0.5f, 0.6f, 0.9f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor FogColor = FLinearColor(0.5f, 0.6f, 0.8f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    bool bVolumetricFog = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float VolumetricFogExtinction = 1.0f;

    FLight_PaletteConfig() {}
};

/**
 * ADawnAtmosphereController
 *
 * Manages the full lighting and atmosphere pipeline for the prehistoric world.
 * Drives day/night cycle, weather transitions, and emotional lighting palettes.
 * Designed to work with UE5 Lumen GI and Sky Atmosphere system.
 *
 * Director of Photography philosophy: light serves emotion, not just illumination.
 * The player never notices correct lighting — only wrong lighting breaks immersion.
 */
UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Dawn Atmosphere Controller"))
class TRANSPERSONALGAME_API ADawnAtmosphereController : public AActor
{
    GENERATED_BODY()

public:
    ADawnAtmosphereController();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // === PALETTE CONTROL ===

    /** Apply a named lighting palette immediately */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Lighting|Palette")
    void ApplyPalette(ELight_DayPalette Palette);

    /** Smoothly transition to a new palette over TransitionDuration seconds */
    UFUNCTION(BlueprintCallable, Category = "Lighting|Palette")
    void TransitionToPalette(ELight_DayPalette TargetPalette, float TransitionDuration = 5.0f);

    /** Apply the dawn palette (cool mist, pink-orange horizon) */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Lighting|Palette")
    void ApplyDawnPalette();

    /** Apply the midday palette (harsh overhead, bleached) */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Lighting|Palette")
    void ApplyMiddayPalette();

    /** Apply the dusk palette (deep orange-red, long shadows) */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Lighting|Palette")
    void ApplyDuskPalette();

    /** Apply the night palette (moonlit, cool blue-black) */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Lighting|Palette")
    void ApplyNightPalette();

    /** Apply the storm palette (dark overcast, desaturated) */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Lighting|Palette")
    void ApplyStormPalette();

    // === DAY/NIGHT CYCLE ===

    /** Enable automatic day/night cycle progression */
    UFUNCTION(BlueprintCallable, Category = "Lighting|Cycle")
    void StartDayNightCycle();

    /** Pause the automatic day/night cycle */
    UFUNCTION(BlueprintCallable, Category = "Lighting|Cycle")
    void PauseDayNightCycle();

    /** Set the current time of day (0.0 = midnight, 0.25 = dawn, 0.5 = noon, 0.75 = dusk) */
    UFUNCTION(BlueprintCallable, Category = "Lighting|Cycle")
    void SetTimeOfDay(float NormalizedTime);

    /** Get current normalized time of day */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Lighting|Cycle")
    float GetTimeOfDay() const { return CurrentTimeOfDay; }

    /** Get current palette name as string */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Lighting|Palette")
    FString GetCurrentPaletteName() const;

    // === REFERENCES ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|References")
    ADirectionalLight* SunLight = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|References")
    ASkyLight* SkyLightActor = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|References")
    AActor* FogActor = nullptr;

    // === CYCLE SETTINGS ===

    /** Full day duration in real seconds (default: 1200s = 20 minutes) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Cycle", meta = (ClampMin = "60.0", ClampMax = "7200.0"))
    float DayDurationSeconds = 1200.0f;

    /** Whether the day/night cycle is currently running */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Cycle")
    bool bCycleActive = false;

    /** Starting time of day when level loads */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Cycle", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float StartingTimeOfDay = 0.25f;

    // === PALETTE CONFIGS ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palettes")
    FLight_PaletteConfig DawnConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palettes")
    FLight_PaletteConfig MiddayConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palettes")
    FLight_PaletteConfig DuskConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palettes")
    FLight_PaletteConfig NightConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palettes")
    FLight_PaletteConfig StormConfig;

private:
    /** Current normalized time of day (0.0-1.0) */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting|State", meta = (AllowPrivateAccess = "true"))
    float CurrentTimeOfDay = 0.25f;

    /** Current active palette */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting|State", meta = (AllowPrivateAccess = "true"))
    ELight_DayPalette CurrentPalette = ELight_DayPalette::Dawn;

    /** Whether a palette transition is in progress */
    bool bTransitioning = false;
    float TransitionProgress = 0.0f;
    float TransitionDuration = 5.0f;
    ELight_DayPalette TransitionTarget = ELight_DayPalette::Dawn;

    /** Initialize default palette configurations */
    void InitializeDefaultPalettes();

    /** Apply a raw palette config to the scene lights */
    void ApplyPaletteConfig(const FLight_PaletteConfig& Config);

    /** Lerp between two palette configs */
    FLight_PaletteConfig LerpPalettes(const FLight_PaletteConfig& A, const FLight_PaletteConfig& B, float Alpha) const;

    /** Get palette config for a given palette enum */
    const FLight_PaletteConfig& GetPaletteConfig(ELight_DayPalette Palette) const;

    /** Update day/night cycle based on elapsed time */
    void UpdateDayNightCycle(float DeltaTime);

    /** Determine which palette should be active at a given time of day */
    ELight_DayPalette GetPaletteForTime(float NormalizedTime) const;

    /** Auto-find scene lights if references not set */
    void AutoFindSceneLights();
};
