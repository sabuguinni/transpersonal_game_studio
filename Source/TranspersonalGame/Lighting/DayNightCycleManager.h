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
    Dawn        UMETA(DisplayName = "Dawn"),
    Morning     UMETA(DisplayName = "Morning"),
    Midday      UMETA(DisplayName = "Midday"),
    Afternoon   UMETA(DisplayName = "Afternoon"),
    GoldenHour  UMETA(DisplayName = "Golden Hour"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Night       UMETA(DisplayName = "Night"),
    Midnight    UMETA(DisplayName = "Midnight")
};

USTRUCT(BlueprintType)
struct FLight_TimePreset
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    ELight_TimeOfDay TimeOfDay = ELight_TimeOfDay::Midday;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunPitchDegrees = -45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunYawDegrees = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor = FLinearColor(1.0f, 0.95f, 0.8f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor FogInscatteringColor = FLinearColor(0.4f, 0.5f, 0.6f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float AmbientBrightness = 1.0f;
};

UCLASS(ClassGroup = (Lighting), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API ADayNightCycleManager : public AActor
{
    GENERATED_BODY()

public:
    ADayNightCycleManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    // Current time of day in hours (0-24)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle")
    float CurrentTimeHours = 12.0f;

    // Speed multiplier for time progression (1.0 = real time, 60.0 = 1 min = 1 hour)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle")
    float TimeSpeedMultiplier = 60.0f;

    // Whether the cycle is actively running
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle")
    bool bCycleActive = true;

    // Reference to the directional light (sun)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|References")
    ADirectionalLight* SunLight = nullptr;

    // Reference to the sky light
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|References")
    ASkyLight* SkyLightActor = nullptr;

    // Reference to the exponential height fog
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|References")
    AExponentialHeightFog* HeightFog = nullptr;

    // Presets for different times of day
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|Presets")
    TArray<FLight_TimePreset> TimePresets;

    // Current time of day enum
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Day Night Cycle")
    ELight_TimeOfDay CurrentTimeOfDayEnum = ELight_TimeOfDay::Midday;

    // Get current time as 0-1 normalized value
    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    float GetNormalizedTime() const;

    // Get current time of day enum
    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    ELight_TimeOfDay GetCurrentTimeOfDay() const;

    // Set time directly (0-24 hours)
    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    void SetTimeOfDay(float NewTimeHours);

    // Jump to a preset time
    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    void SetTimePreset(ELight_TimeOfDay TargetTime);

    // Auto-find scene references
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Day Night Cycle")
    void AutoFindSceneReferences();

    // Apply current lighting based on time
    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    void ApplyLightingForCurrentTime();

private:
    void UpdateSunPosition(float TimeHours);
    void UpdateFogSettings(float TimeHours);
    void UpdateSkyLight(float TimeHours);
    FLinearColor LerpColor(const FLinearColor& A, const FLinearColor& B, float Alpha) const;
    void InitializeDefaultPresets();
    float TimeAccumulator = 0.0f;
};
