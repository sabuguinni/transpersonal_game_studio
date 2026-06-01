#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Subsystems/WorldSubsystem.h"
#include "../SharedTypes.h"
#include "Perf_DayNightCycle.generated.h"

UENUM(BlueprintType)
enum class EPerf_TimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Morning     UMETA(DisplayName = "Morning"),
    Noon        UMETA(DisplayName = "Noon"),
    Afternoon   UMETA(DisplayName = "Afternoon"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Night       UMETA(DisplayName = "Night"),
    Midnight    UMETA(DisplayName = "Midnight"),
    PreDawn     UMETA(DisplayName = "Pre-Dawn")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_TimeSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    float SunAngle = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    FLinearColor SunColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time", meta = (ClampMin = "0.0", ClampMax = "20.0"))
    float SunIntensity = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    FLinearColor SkyColor = FLinearColor(0.4f, 0.5f, 0.8f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time", meta = (ClampMin = "0.0", ClampMax = "10.0"))
    float SkyIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    FLinearColor HorizonColor = FLinearColor(1.0f, 0.8f, 0.6f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float StarVisibility = 0.0f;

    FPerf_TimeSettings()
    {
        SunAngle = 0.0f;
        SunColor = FLinearColor::White;
        SunIntensity = 3.0f;
        SkyColor = FLinearColor(0.4f, 0.5f, 0.8f, 1.0f);
        SkyIntensity = 1.0f;
        HorizonColor = FLinearColor(1.0f, 0.8f, 0.6f, 1.0f);
        StarVisibility = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPerf_DayNightCycle : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UPerf_DayNightCycle();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    void SetTimeOfDay(float Hours);

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    float GetTimeOfDay() const { return CurrentTimeHours; }

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    EPerf_TimeOfDay GetCurrentTimeOfDayEnum() const;

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    void SetCycleSpeed(float NewSpeed);

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    float GetCycleSpeed() const { return CycleSpeedMultiplier; }

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    void PauseCycle(bool bPause);

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    bool IsCyclePaused() const { return bIsPaused; }

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    void UpdateLighting();

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle", CallInEditor)
    void RefreshLightingReferences();

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    FString GetTimeString() const;

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    void SetToNoon();

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    void SetToMidnight();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time", meta = (ClampMin = "0.0", ClampMax = "24.0"))
    float CurrentTimeHours = 12.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time", meta = (ClampMin = "0.1", ClampMax = "100.0"))
    float CycleSpeedMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    bool bIsPaused = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    bool bAutoUpdateLighting = true;

    // Time period settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Periods")
    FPerf_TimeSettings DawnSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Periods")
    FPerf_TimeSettings MorningSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Periods")
    FPerf_TimeSettings NoonSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Periods")
    FPerf_TimeSettings AfternoonSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Periods")
    FPerf_TimeSettings DuskSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Periods")
    FPerf_TimeSettings NightSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Periods")
    FPerf_TimeSettings MidnightSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Periods")
    FPerf_TimeSettings PreDawnSettings;

    // Lighting references
    UPROPERTY(BlueprintReadOnly, Category = "Lighting")
    TWeakObjectPtr<ADirectionalLight> SunLight;

    UPROPERTY(BlueprintReadOnly, Category = "Lighting")
    TWeakObjectPtr<ASkyLight> SkyLight;

private:
    void InitializeTimeSettings();
    void FindLightingActors();
    void TickDayNightCycle(float DeltaTime);
    void ApplyTimeSettings(const FPerf_TimeSettings& Settings);
    FPerf_TimeSettings GetInterpolatedTimeSettings() const;
    FPerf_TimeSettings GetTimeSettingsForPeriod(EPerf_TimeOfDay TimeOfDay) const;
    float GetSunAngleFromTime(float Hours) const;

    FTimerHandle CycleUpdateTimer;
};