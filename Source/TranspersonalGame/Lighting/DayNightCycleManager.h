#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyLightComponent.h"
#include "DayNightCycleManager.generated.h"

// ============================================================
//  ELight_TimeOfDay — named time phases for the day/night cycle
// ============================================================
UENUM(BlueprintType)
enum class ELight_TimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Morning     UMETA(DisplayName = "Morning"),
    Midday      UMETA(DisplayName = "Midday"),
    Afternoon   UMETA(DisplayName = "Afternoon"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Night       UMETA(DisplayName = "Night"),
    DeepNight   UMETA(DisplayName = "DeepNight")
};

// ============================================================
//  FLight_SkyPalette — one snapshot of sky/light parameters
// ============================================================
USTRUCT(BlueprintType)
struct FLight_SkyPalette
{
    GENERATED_BODY()

    /** Sun/Moon pitch angle in degrees (negative = above horizon) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palette")
    float SunPitchDeg = -45.0f;

    /** Sun/Moon yaw angle in degrees */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palette")
    float SunYawDeg = 180.0f;

    /** Directional light intensity (lux) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palette")
    float SunIntensity = 10.0f;

    /** Directional light color (linear) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palette")
    FLinearColor SunColor = FLinearColor(1.0f, 0.95f, 0.8f, 1.0f);

    /** Fog density */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palette")
    float FogDensity = 0.02f;

    /** Fog inscattering color (linear) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palette")
    FLinearColor FogColor = FLinearColor(0.4f, 0.5f, 0.7f, 1.0f);

    /** Rayleigh scattering scale for SkyAtmosphere */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palette")
    float RayleighScale = 0.0331f;

    /** Mie scattering scale for SkyAtmosphere */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palette")
    float MieScale = 0.003f;

    /** Color temperature in Kelvin */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palette")
    float ColorTemperatureK = 6500.0f;
};

// ============================================================
//  ADayNightCycleManager — drives the full 24h lighting cycle
// ============================================================
UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Day Night Cycle Manager"))
class TRANSPERSONALGAME_API ADayNightCycleManager : public AActor
{
    GENERATED_BODY()

public:
    ADayNightCycleManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // ---- References (set in level or auto-found) ----

    /** The directional light acting as sun/moon */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|References")
    TObjectPtr<ADirectionalLight> SunMoonLight;

    /** The exponential height fog actor */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|References")
    TObjectPtr<AExponentialHeightFog> HeightFog;

    /** The sky light actor */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|References")
    TObjectPtr<ASkyLight> SkyLightActor;

    // ---- Cycle settings ----

    /** Total real-time seconds for one full in-game day */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Cycle", meta = (ClampMin = "60.0", ClampMax = "3600.0"))
    float DayDurationSeconds = 600.0f;

    /** Current in-game time of day (0.0 = midnight, 0.5 = noon, 1.0 = midnight again) */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting|Cycle")
    float NormalizedTimeOfDay = 0.25f;  // Start at dawn

    /** Whether the cycle is running */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Cycle")
    bool bCycleActive = true;

    /** Current time phase */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting|Cycle")
    ELight_TimeOfDay CurrentPhase = ELight_TimeOfDay::Dawn;

    // ---- Palettes ----

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palettes")
    FLight_SkyPalette DawnPalette;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palettes")
    FLight_SkyPalette MorningPalette;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palettes")
    FLight_SkyPalette MiddayPalette;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palettes")
    FLight_SkyPalette AfternoonPalette;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palettes")
    FLight_SkyPalette DuskPalette;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palettes")
    FLight_SkyPalette NightPalette;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palettes")
    FLight_SkyPalette DeepNightPalette;

    // ---- Blueprint events ----

    UFUNCTION(BlueprintImplementableEvent, Category = "Lighting|Events")
    void OnPhaseChanged(ELight_TimeOfDay NewPhase);

    UFUNCTION(BlueprintCallable, Category = "Lighting|Control")
    void SetTimeOfDay(float NormalizedTime);

    UFUNCTION(BlueprintCallable, Category = "Lighting|Control")
    void SetPhaseImmediate(ELight_TimeOfDay Phase);

    UFUNCTION(BlueprintPure, Category = "Lighting|Info")
    float GetInGameHour() const;

    UFUNCTION(BlueprintPure, Category = "Lighting|Info")
    ELight_TimeOfDay GetCurrentPhase() const { return CurrentPhase; }

private:
    void AutoFindLightActors();
    void AdvanceCycle(float DeltaTime);
    ELight_TimeOfDay TimeToPhase(float NormalizedTime) const;
    FLight_SkyPalette GetPaletteForPhase(ELight_TimeOfDay Phase) const;
    FLight_SkyPalette LerpPalettes(const FLight_SkyPalette& A, const FLight_SkyPalette& B, float Alpha) const;
    void ApplyPalette(const FLight_SkyPalette& Palette);

    ELight_TimeOfDay LastPhase = ELight_TimeOfDay::Dawn;
};
