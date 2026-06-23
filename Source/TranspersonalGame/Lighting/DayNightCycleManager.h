#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyLightComponent.h"
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
struct FLight_TimeOfDaySettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunPitch = -45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunYaw = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor FogColor = FLinearColor(0.5f, 0.6f, 0.8f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SkyLightIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float ExposureBias = 1.0f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ADayNightCycleManager : public AActor
{
    GENERATED_BODY()

public:
    ADayNightCycleManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Current time of day in hours (0-24)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle")
    float CurrentTimeOfDay = 8.0f;

    // Speed multiplier for time progression (1.0 = real time, 60.0 = 1 min = 1 hour)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle")
    float TimeSpeed = 60.0f;

    // Whether the cycle is running
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle")
    bool bCycleActive = true;

    // Reference to the directional light (Sun)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle")
    TObjectPtr<ADirectionalLight> SunLight;

    // Reference to the sky light
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle")
    TObjectPtr<ASkyLight> SkyLightActor;

    // Reference to the fog actor
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle")
    TObjectPtr<AExponentialHeightFog> FogActor;

    // Cretaceous golden hour settings (default)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|Presets")
    FLight_TimeOfDaySettings GoldenHourSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|Presets")
    FLight_TimeOfDaySettings DawnSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|Presets")
    FLight_TimeOfDaySettings NightSettings;

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    ELight_TimeOfDay GetCurrentTimeOfDayEnum() const;

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    void SetTimeOfDay(float NewTime);

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    float GetNormalizedDayProgress() const;

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Day Night Cycle")
    void ApplyGoldenHourPreset();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Day Night Cycle")
    void ApplyDawnPreset();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Day Night Cycle")
    void ApplyNightPreset();

protected:
    void UpdateLighting(float DeltaTime);
    void ApplyTimeOfDaySettings(const FLight_TimeOfDaySettings& Settings);
    FLight_TimeOfDaySettings InterpolateSettings(const FLight_TimeOfDaySettings& A, const FLight_TimeOfDaySettings& B, float Alpha) const;
    FLight_TimeOfDaySettings GetSettingsForTime(float TimeHours) const;

private:
    float AccumulatedTime = 0.0f;
};
