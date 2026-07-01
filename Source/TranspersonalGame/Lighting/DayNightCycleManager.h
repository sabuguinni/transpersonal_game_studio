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
    Twilight    UMETA(DisplayName = "Twilight"),
    Night       UMETA(DisplayName = "Night"),
    Midnight    UMETA(DisplayName = "Midnight")
};

USTRUCT(BlueprintType)
struct FLight_TimeOfDaySettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunPitchDegrees = -45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunYawDegrees = 180.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor = FLinearColor(1.0f, 0.95f, 0.8f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor FogColor = FLinearColor(0.6f, 0.7f, 0.9f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SkyLightIntensity = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SkyLightColor = FLinearColor(0.8f, 0.85f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    bool bVolumetricFog = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float BloomIntensity = 0.5f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ADayNightCycleManager : public AActor
{
    GENERATED_BODY()

public:
    ADayNightCycleManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // === REFERENCES ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|References")
    TObjectPtr<ADirectionalLight> SunLight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|References")
    TObjectPtr<AExponentialHeightFog> HeightFog;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|References")
    TObjectPtr<ASkyLight> SkyLightActor;

    // === TIME SETTINGS ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|Time")
    float CurrentTimeOfDayHours = 18.0f;  // Start at dusk (18:00)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|Time")
    float TimeSpeedMultiplier = 1.0f;  // 1.0 = real-time, 60.0 = 1 min = 1 game hour

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|Time")
    bool bEnableDayNightCycle = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|Time")
    float DayLengthMinutes = 20.0f;  // Real minutes per full game day

    // === CURRENT STATE ===
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Day Night Cycle|State")
    ELight_TimeOfDay CurrentTimeOfDayEnum = ELight_TimeOfDay::Dusk;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Day Night Cycle|State")
    float NormalizedTimeOfDay = 0.75f;  // 0.0 = midnight, 0.5 = noon, 1.0 = midnight

    // === PRESET SETTINGS ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|Presets")
    FLight_TimeOfDaySettings DawnSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|Presets")
    FLight_TimeOfDaySettings MiddaySettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|Presets")
    FLight_TimeOfDaySettings DuskSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|Presets")
    FLight_TimeOfDaySettings NightSettings;

    // === FUNCTIONS ===
    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    void SetTimeOfDay(float HoursIn24Format);

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    void SetTimeOfDayPreset(ELight_TimeOfDay TimePreset);

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    float GetCurrentHour() const;

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    ELight_TimeOfDay GetCurrentTimeOfDayEnum() const;

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    void ApplyLightingSettings(const FLight_TimeOfDaySettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    void FindAndCacheSceneLights();

    UFUNCTION(CallInEditor, Category = "Day Night Cycle")
    void ApplyDuskPreset();

    UFUNCTION(CallInEditor, Category = "Day Night Cycle")
    void ApplyDawnPreset();

    UFUNCTION(CallInEditor, Category = "Day Night Cycle")
    void ApplyNightPreset();

private:
    void UpdateDayNightCycle(float DeltaTime);
    ELight_TimeOfDay HoursToTimeOfDayEnum(float Hours) const;
    FLight_TimeOfDaySettings InterpolateSettings(
        const FLight_TimeOfDaySettings& A,
        const FLight_TimeOfDaySettings& B,
        float Alpha) const;

    void InitializeDefaultPresets();

    float AccumulatedTime = 0.0f;
};
