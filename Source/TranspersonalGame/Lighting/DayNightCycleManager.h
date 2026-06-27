#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "DayNightCycleManager.generated.h"

UENUM(BlueprintType)
enum class ELight_TimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Morning     UMETA(DisplayName = "Morning"),
    Midday      UMETA(DisplayName = "Midday"),
    Afternoon   UMETA(DisplayName = "Afternoon"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Night       UMETA(DisplayName = "Night"),
    Midnight    UMETA(DisplayName = "Midnight")
};

USTRUCT(BlueprintType)
struct FLight_TimeOfDaySettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunPitch = -45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity = 8.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor = FLinearColor(1.0f, 0.78f, 0.55f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SkyLightIntensity = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor FogColor = FLinearColor(0.6f, 0.75f, 0.9f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float AmbientTemperature = 22.0f;
};

UCLASS(ClassGroup = (Lighting), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API ADayNightCycleManager : public AActor
{
    GENERATED_BODY()

public:
    ADayNightCycleManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Current time of day (0-24 hours)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle")
    float CurrentTimeOfDay = 12.0f;

    // Speed multiplier for time progression (1.0 = real-time, 60.0 = 1 min per second)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle")
    float TimeSpeed = 60.0f;

    // Whether the cycle is active
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle")
    bool bCycleActive = true;

    // Reference to the directional light (sun)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle")
    TObjectPtr<ADirectionalLight> SunLight;

    // Reference to the sky light
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle")
    TObjectPtr<ASkyLight> SkyLightActor;

    // Reference to the height fog
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle")
    TObjectPtr<AExponentialHeightFog> HeightFogActor;

    // Dawn settings (5:00 - 7:00)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Settings")
    FLight_TimeOfDaySettings DawnSettings;

    // Midday settings (11:00 - 13:00)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Settings")
    FLight_TimeOfDaySettings MiddaySettings;

    // Dusk settings (17:00 - 19:00)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Settings")
    FLight_TimeOfDaySettings DuskSettings;

    // Night settings (21:00 - 4:00)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Settings")
    FLight_TimeOfDaySettings NightSettings;

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    ELight_TimeOfDay GetCurrentTimeOfDayEnum() const;

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    void SetTimeOfDay(float NewTime);

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    FLight_TimeOfDaySettings GetCurrentSettings() const;

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    float GetSunPitchForTime(float TimeHours) const;

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    FLinearColor GetSunColorForTime(float TimeHours) const;

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    float GetFogDensityForTime(float TimeHours) const;

private:
    void UpdateLighting(float TimeHours);
    void ApplySettingsToActors(const FLight_TimeOfDaySettings& Settings);
    FLight_TimeOfDaySettings LerpSettings(const FLight_TimeOfDaySettings& A, const FLight_TimeOfDaySettings& B, float Alpha) const;
};
