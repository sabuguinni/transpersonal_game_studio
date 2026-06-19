#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyLightComponent.h"
#include "Engine/DirectionalLight.h"
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
    float SunIntensity = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor = FLinearColor(1.0f, 0.92f, 0.78f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunPitch = -45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SkyLightIntensity = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor FogColor = FLinearColor(0.4f, 0.5f, 0.7f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float AmbientTemperature = 22.0f;
};

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent))
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

    // Speed multiplier for time progression (1.0 = real time, 60.0 = 1 min per second)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle")
    float TimeSpeedMultiplier = 60.0f;

    // Whether the cycle is actively running
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle")
    bool bCycleActive = true;

    // Reference to the directional light (sun)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle")
    ADirectionalLight* SunLight = nullptr;

    // Reference to the sky light
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle")
    ASkyLight* SkyLightActor = nullptr;

    // Presets for each time of day
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle")
    TMap<ELight_TimeOfDay, FLight_TimeOfDaySettings> TimeOfDayPresets;

    // Current time of day enum
    UPROPERTY(BlueprintReadOnly, Category = "Day Night Cycle", meta = (AllowPrivateAccess = "true"))
    ELight_TimeOfDay CurrentTimeOfDay = ELight_TimeOfDay::Midday;

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    void SetTimeOfDay(float NewTimeHours);

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    float GetCurrentTimeHours() const { return CurrentTimeHours; }

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    ELight_TimeOfDay GetCurrentTimeOfDayEnum() const { return CurrentTimeOfDay; }

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    void ApplyLightingPreset(const FLight_TimeOfDaySettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    FLight_TimeOfDaySettings GetSettingsForTime(float TimeHours) const;

private:
    void UpdateSunPosition(float TimeHours);
    void UpdateLightColors(float TimeHours);
    void UpdateFog(float TimeHours);
    ELight_TimeOfDay TimeHoursToEnum(float TimeHours) const;
    void InitializeDefaultPresets();
};
