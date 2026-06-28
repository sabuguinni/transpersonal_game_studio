#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/SkyLight.h"
#include "DayNightCycleManager.generated.h"

UENUM(BlueprintType)
enum class ELight_TimeOfDay : uint8
{
    Dawn       UMETA(DisplayName = "Dawn"),
    Morning    UMETA(DisplayName = "Morning"),
    Midday     UMETA(DisplayName = "Midday"),
    Afternoon  UMETA(DisplayName = "Afternoon"),
    Dusk       UMETA(DisplayName = "Dusk"),
    Night      UMETA(DisplayName = "Night"),
    Midnight   UMETA(DisplayName = "Midnight")
};

USTRUCT(BlueprintType)
struct FLight_TimeOfDaySettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor = FLinearColor(1.0f, 0.86f, 0.63f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity = 8.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunPitch = -45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor FogColor = FLinearColor(0.6f, 0.75f, 0.9f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float FogDensity = 0.025f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SkyTint = FLinearColor(0.4f, 0.6f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SkyLightIntensity = 1.2f;
};

/**
 * ADayNightCycleManager
 * Drives the full 24-hour lighting cycle for the Cretaceous world.
 * Controls sun position, color temperature, fog density, and sky atmosphere.
 * Designed for UE5 Lumen — all changes are real-time and physically based.
 */
UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Day Night Cycle Manager"))
class TRANSPERSONALGAME_API ADayNightCycleManager : public AActor
{
    GENERATED_BODY()

public:
    ADayNightCycleManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // --- Configuration ---

    /** Total real-world seconds for one full day (default: 600s = 10 minutes) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle")
    float DayDurationSeconds = 600.0f;

    /** Current time of day in hours (0.0 = midnight, 6.0 = dawn, 12.0 = noon, 18.0 = dusk) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle", meta = (ClampMin = "0.0", ClampMax = "24.0"))
    float CurrentHour = 8.0f;

    /** Whether the cycle is actively running */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle")
    bool bCycleActive = true;

    /** Speed multiplier for time progression */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle")
    float TimeScale = 1.0f;

    // --- Lighting References ---

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting References")
    ADirectionalLight* SunLight = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting References")
    AExponentialHeightFog* HeightFog = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting References")
    ASkyLight* SkyLightActor = nullptr;

    // --- Time of Day Settings ---

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Settings")
    FLight_TimeOfDaySettings DawnSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Settings")
    FLight_TimeOfDaySettings MiddaySettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Settings")
    FLight_TimeOfDaySettings DuskSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Settings")
    FLight_TimeOfDaySettings NightSettings;

    // --- Queries ---

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Day Night Cycle")
    ELight_TimeOfDay GetCurrentTimeOfDay() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Day Night Cycle")
    float GetCurrentHour() const { return CurrentHour; }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Day Night Cycle")
    bool IsNight() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Day Night Cycle")
    float GetNormalizedDayProgress() const;

    // --- Control ---

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    void SetTimeOfDay(float Hour);

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    void PauseCycle();

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    void ResumeCycle();

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    void AutoFindLightingActors();

private:
    void AdvanceTime(float DeltaTime);
    void ApplyLightingForHour(float Hour);
    FLight_TimeOfDaySettings InterpolateSettings(const FLight_TimeOfDaySettings& A, const FLight_TimeOfDaySettings& B, float Alpha) const;
    void ApplySettingsToScene(const FLight_TimeOfDaySettings& Settings);
    void InitDefaultSettings();

    float HoursPerSecond = 0.04f; // 24 hours / 600 seconds
};
