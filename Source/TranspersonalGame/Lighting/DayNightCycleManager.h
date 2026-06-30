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
    float SunYaw = 180.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor = FLinearColor(1.0f, 0.95f, 0.85f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor FogColor = FLinearColor(0.7f, 0.8f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SkyLightIntensity = 2.0f;

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

    // Current time of day (0.0 = midnight, 0.5 = noon, 1.0 = midnight)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle")
    float TimeOfDayNormalized = 0.5f;

    // Speed multiplier for day/night progression (1.0 = real time, 100.0 = fast)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle")
    float DayCycleSpeed = 60.0f;

    // Whether the cycle is actively running
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle")
    bool bCycleActive = true;

    // Current time of day enum
    UPROPERTY(BlueprintReadOnly, Category = "Day Night Cycle")
    ELight_TimeOfDay CurrentTimeOfDay = ELight_TimeOfDay::Midday;

    // Settings for each time of day
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|Settings")
    FLight_TimeOfDaySettings DawnSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|Settings")
    FLight_TimeOfDaySettings MiddaySettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|Settings")
    FLight_TimeOfDaySettings DuskSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|Settings")
    FLight_TimeOfDaySettings NightSettings;

    // References to scene lighting actors
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|References")
    ADirectionalLight* SunLight = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|References")
    AExponentialHeightFog* HeightFog = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|References")
    ASkyLight* SkyLightActor = nullptr;

    // Get current time as hours (0-24)
    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    float GetCurrentHour() const;

    // Set time directly (0.0 - 1.0)
    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    void SetTimeOfDay(float NewTime);

    // Force a specific time of day preset
    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    void ApplyTimeOfDayPreset(ELight_TimeOfDay Preset);

    // Get current ambient temperature based on time
    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    float GetAmbientTemperature() const;

private:
    void UpdateLighting(float DeltaTime);
    void UpdateTimeOfDayEnum();
    FLight_TimeOfDaySettings InterpolateSettings(const FLight_TimeOfDaySettings& A, const FLight_TimeOfDaySettings& B, float Alpha) const;
    void ApplySettingsToScene(const FLight_TimeOfDaySettings& Settings);
    void InitializeDefaultSettings();
};
