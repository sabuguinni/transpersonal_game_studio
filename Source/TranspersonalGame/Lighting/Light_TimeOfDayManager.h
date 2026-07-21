#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
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
struct FLight_TimeSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor = FLinearColor(1.0f, 0.95f, 0.9f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FRotator SunRotation = FRotator(-30.0f, 45.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FLinearColor FogColor = FLinearColor(0.9f, 0.95f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Post Process")
    float ExposureBias = 0.5f;

    FLight_TimeSettings()
    {
        SunIntensity = 5.0f;
        SunColor = FLinearColor(1.0f, 0.95f, 0.9f, 1.0f);
        SunRotation = FRotator(-30.0f, 45.0f, 0.0f);
        FogDensity = 0.02f;
        FogColor = FLinearColor(0.9f, 0.95f, 1.0f, 1.0f);
        ExposureBias = 0.5f;
    }
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
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    float CurrentTimeOfDay = 12.0f; // 0-24 hours

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    float DayDuration = 1200.0f; // Real seconds for full day cycle

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    bool bAutoAdvanceTime = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    float TimeScale = 1.0f;

    // Lighting References
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting References")
    class ADirectionalLight* SunLight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting References")
    class AExponentialHeightFog* AtmosphericFog;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting References")
    class APostProcessVolume* PostProcessVolume;

    // Time Period Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Settings")
    FLight_TimeSettings DawnSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Settings")
    FLight_TimeSettings MorningSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Settings")
    FLight_TimeSettings MiddaySettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Settings")
    FLight_TimeSettings AfternoonSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Settings")
    FLight_TimeSettings DuskSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Settings")
    FLight_TimeSettings NightSettings;

    // Functions
    UFUNCTION(BlueprintCallable, Category = "Time of Day")
    void SetTimeOfDay(float NewTime);

    UFUNCTION(BlueprintCallable, Category = "Time of Day")
    ELight_TimeOfDay GetCurrentTimePeriod() const;

    UFUNCTION(BlueprintCallable, Category = "Time of Day")
    void SetTimePeriod(ELight_TimeOfDay TimePeriod);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void UpdateLighting();

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void FindLightingActors();

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    FLight_TimeSettings GetInterpolatedSettings() const;

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void ApplyLightingSettings(const FLight_TimeSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void ConfigureCretaceousAtmosphere();

    UFUNCTION(BlueprintCallable, Category = "Lumen")
    void ConfigureLumenSettings();

private:
    void InitializeTimeSettings();
    FLight_TimeSettings InterpolateSettings(const FLight_TimeSettings& A, const FLight_TimeSettings& B, float Alpha) const;
    float GetTimeAlpha(float StartHour, float EndHour, float CurrentHour) const;
};

#include "Light_TimeOfDayManager.generated.h"