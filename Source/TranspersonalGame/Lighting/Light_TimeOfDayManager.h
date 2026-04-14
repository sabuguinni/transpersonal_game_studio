#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/PostProcessVolume.h"
#include "Light_TimeOfDayManager.generated.h"

UENUM(BlueprintType)
enum class ELight_TimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Morning     UMETA(DisplayName = "Morning"), 
    Midday      UMETA(DisplayName = "Midday"),
    Afternoon   UMETA(DisplayName = "Afternoon"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Night       UMETA(DisplayName = "Night")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_TimeOfDaySettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    FRotator SunRotation = FRotator(-45.0f, 0.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    float SunIntensity = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    FLinearColor SunColor = FLinearColor(1.0f, 0.95f, 0.8f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky")
    float SkyLightIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky")
    FLinearColor SkyLightColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor FogColor = FLinearColor(0.9f, 0.9f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Post Process")
    float Exposure = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Post Process")
    float Temperature = 6500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Post Process")
    float Saturation = 1.0f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ALight_TimeOfDayManager : public AActor
{
    GENERATED_BODY()

public:
    ALight_TimeOfDayManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Time of Day Control
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day", meta = (ClampMin = "0.0", ClampMax = "24.0"))
    float CurrentTime = 12.0f; // 12:00 noon

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    float DayDurationMinutes = 20.0f; // Real-time minutes for full day cycle

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    bool bAutoAdvanceTime = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    float TimeMultiplier = 1.0f;

    // Lighting References
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting References")
    TSoftObjectPtr<ADirectionalLight> SunLight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting References")
    TSoftObjectPtr<ASkyLight> SkyLight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting References")
    TSoftObjectPtr<AExponentialHeightFog> HeightFog;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting References")
    TSoftObjectPtr<APostProcessVolume> PostProcessVolume;

    // Time of Day Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Settings")
    FLight_TimeOfDaySettings DawnSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Settings")
    FLight_TimeOfDaySettings MorningSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Settings")
    FLight_TimeOfDaySettings MiddaySettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Settings")
    FLight_TimeOfDaySettings AfternoonSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Settings")
    FLight_TimeOfDaySettings DuskSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Settings")
    FLight_TimeOfDaySettings NightSettings;

    // Blueprint Functions
    UFUNCTION(BlueprintCallable, Category = "Time of Day")
    void SetTimeOfDay(float NewTime);

    UFUNCTION(BlueprintCallable, Category = "Time of Day")
    ELight_TimeOfDay GetCurrentTimeOfDayPeriod() const;

    UFUNCTION(BlueprintCallable, Category = "Time of Day")
    void SetTimeOfDayPeriod(ELight_TimeOfDay Period);

    UFUNCTION(BlueprintCallable, Category = "Time of Day")
    void UpdateLighting();

    UFUNCTION(BlueprintCallable, Category = "Time of Day")
    FLight_TimeOfDaySettings GetCurrentSettings() const;

private:
    void InitializeLightingReferences();
    void UpdateSunPosition();
    void UpdateLightingIntensity();
    void UpdateFogSettings();
    void UpdatePostProcessSettings();
    FLight_TimeOfDaySettings InterpolateSettings(const FLight_TimeOfDaySettings& A, const FLight_TimeOfDaySettings& B, float Alpha) const;
    float GetInterpolationAlpha(float Time, float StartTime, float EndTime) const;
};