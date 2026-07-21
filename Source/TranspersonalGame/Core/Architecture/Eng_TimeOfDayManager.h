#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/DirectionalLight.h"
#include "Components/SkyLightComponent.h"
#include "SharedTypes.h"
#include "Eng_TimeOfDayManager.generated.h"

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UEng_TimeOfDayManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual void Tick(float DeltaTime) override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override { return true; }

    // Time Control
    UFUNCTION(BlueprintCallable, Category = "Time System")
    void SetTimeOfDay(float Hours);

    UFUNCTION(BlueprintCallable, Category = "Time System")
    void SetTimeScale(float Scale);

    UFUNCTION(BlueprintCallable, Category = "Time System")
    void PauseTime(bool bPause);

    UFUNCTION(BlueprintCallable, Category = "Time System")
    void SetDayDuration(float Minutes);

    // Time State
    UFUNCTION(BlueprintPure, Category = "Time System")
    float GetTimeOfDay() const { return CurrentTimeOfDay; }

    UFUNCTION(BlueprintPure, Category = "Time System")
    int32 GetHours() const { return FMath::FloorToInt(CurrentTimeOfDay); }

    UFUNCTION(BlueprintPure, Category = "Time System")
    int32 GetMinutes() const { return FMath::FloorToInt((CurrentTimeOfDay - GetHours()) * 60.0f); }

    UFUNCTION(BlueprintPure, Category = "Time System")
    EEng_TimeOfDay GetTimePhase() const;

    UFUNCTION(BlueprintPure, Category = "Time System")
    float GetSunAngle() const;

    UFUNCTION(BlueprintPure, Category = "Time System")
    bool IsNight() const;

    UFUNCTION(BlueprintPure, Category = "Time System")
    bool IsDay() const;

    UFUNCTION(BlueprintPure, Category = "Time System")
    float GetTimeScale() const { return TimeScale; }

    // Lighting Control
    UFUNCTION(BlueprintCallable, Category = "Time System")
    void UpdateSunPosition();

    UFUNCTION(BlueprintCallable, Category = "Time System")
    void UpdateSkyLighting();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Time State")
    float CurrentTimeOfDay; // 0-24 hours

    UPROPERTY(BlueprintReadOnly, Category = "Time State")
    float TimeScale; // Speed multiplier

    UPROPERTY(BlueprintReadOnly, Category = "Time State")
    bool bTimePaused;

    UPROPERTY(BlueprintReadOnly, Category = "Time State")
    float DayDurationMinutes; // Real-time minutes for full day

    // Sun Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun Config")
    float SunriseTime; // Hours (e.g., 6.0)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun Config")
    float SunsetTime; // Hours (e.g., 18.0)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun Config")
    float MaxSunIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun Config")
    float MinSunIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun Config")
    FLinearColor DayLightColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun Config")
    FLinearColor NightLightColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun Config")
    FLinearColor SunriseColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun Config")
    FLinearColor SunsetColor;

private:
    void UpdateTimeOfDay(float DeltaTime);
    void UpdateLighting();
    FLinearColor GetCurrentLightColor() const;
    float GetCurrentLightIntensity() const;
    
    TWeakObjectPtr<ADirectionalLight> SunLight;
    TWeakObjectPtr<USkyLightComponent> SkyLight;
    
    float LastLightingUpdate;
    const float LightingUpdateInterval = 0.5f; // Update lighting every 0.5 seconds
};