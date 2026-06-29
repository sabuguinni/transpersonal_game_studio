#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "DayNightCycleManager.generated.h"

// ============================================================
// ELight_TimeOfDay — Named time phases for lighting transitions
// ============================================================
UENUM(BlueprintType)
enum class ELight_TimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Morning     UMETA(DisplayName = "Morning"),
    Midday      UMETA(DisplayName = "Midday"),
    Afternoon   UMETA(DisplayName = "Afternoon"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Twilight    UMETA(DisplayName = "Twilight"),
    Night       UMETA(DisplayName = "Night"),
    Midnight    UMETA(DisplayName = "Midnight")
};

// ============================================================
// FLight_SkyPalette — Color/intensity data for one time phase
// ============================================================
USTRUCT(BlueprintType)
struct FLight_SkyPalette
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palette")
    FLinearColor SunColor = FLinearColor(1.f, 0.95f, 0.8f, 1.f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palette")
    float SunIntensity = 10.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palette")
    float SunPitchDegrees = 45.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palette")
    FLinearColor FogInscatterColor = FLinearColor(0.5f, 0.6f, 0.8f, 1.f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palette")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palette")
    FLinearColor SkyLightColor = FLinearColor(0.4f, 0.5f, 0.7f, 1.f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palette")
    float SkyLightIntensity = 1.f;
};

// ============================================================
// ADayNightCycleManager — Drives full 24h lighting cycle
// ============================================================
UCLASS(ClassGroup = "TranspersonalGame|Lighting", meta = (DisplayName = "Day Night Cycle Manager"))
class TRANSPERSONALGAME_API ADayNightCycleManager : public AActor
{
    GENERATED_BODY()

public:
    ADayNightCycleManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // ---- References (set in editor or auto-found) ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|References")
    ADirectionalLight* SunLight = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|References")
    ASkyLight* SkyLightActor = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|References")
    AActor* FogActor = nullptr;

    // ---- Time Settings ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Time", meta = (ClampMin = "0.0", ClampMax = "24.0"))
    float CurrentHour = 6.5f;  // Default: Dawn

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Time", meta = (ClampMin = "0.01", ClampMax = "100.0"))
    float DayDurationMinutes = 20.f;  // Real minutes per full game day

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Time")
    bool bPauseDayNightCycle = false;

    // ---- Palette Definitions ----
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

    // ---- Current State (read-only) ----
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting|State", meta = (AllowPrivateAccess = "true"))
    ELight_TimeOfDay CurrentTimeOfDay = ELight_TimeOfDay::Dawn;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting|State", meta = (AllowPrivateAccess = "true"))
    float CurrentSunPitch = 8.f;

    // ---- Blueprint Events ----
    UFUNCTION(BlueprintImplementableEvent, Category = "Lighting|Events")
    void OnTimeOfDayChanged(ELight_TimeOfDay NewTimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Lighting|Control")
    void SetHour(float NewHour);

    UFUNCTION(BlueprintCallable, Category = "Lighting|Control")
    void AdvanceTimeByHours(float Hours);

    UFUNCTION(BlueprintCallable, Category = "Lighting|Debug", CallInEditor)
    void ApplyCurrentPaletteNow();

private:
    void TickDayNightCycle(float DeltaTime);
    ELight_TimeOfDay GetTimeOfDayFromHour(float Hour) const;
    FLight_SkyPalette GetPaletteForTime(ELight_TimeOfDay TimeOfDay) const;
    FLight_SkyPalette LerpPalettes(const FLight_SkyPalette& A, const FLight_SkyPalette& B, float Alpha) const;
    void ApplyPaletteToWorld(const FLight_SkyPalette& Palette);
    void AutoFindLightingActors();

    float TimeAccumulator = 0.f;
    ELight_TimeOfDay PreviousTimeOfDay = ELight_TimeOfDay::Dawn;
};
