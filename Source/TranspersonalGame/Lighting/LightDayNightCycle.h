#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/SkyLight.h"
#include "LightDayNightCycle.generated.h"

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
    ELight_TimeOfDay TimeOfDay = ELight_TimeOfDay::Midday;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunPitch = -45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunYaw = 180.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor = FLinearColor(1.0f, 0.95f, 0.85f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FLinearColor FogInscatteringColor = FLinearColor(0.6f, 0.7f, 0.9f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float FogHeightFalloff = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float VolumetricFogExtinctionScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FLinearColor VolumetricFogAlbedo = FLinearColor(0.9f, 0.9f, 0.9f, 1.0f);
};

UCLASS(BlueprintType, Blueprintable, ClassGroup = "TranspersonalGame|Lighting")
class TRANSPERSONALGAME_API ALightDayNightCycle : public AActor
{
    GENERATED_BODY()

public:
    ALightDayNightCycle();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Current time of day (0.0 = midnight, 0.5 = noon, 1.0 = midnight)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Cycle", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float TimeOfDayNormalized = 0.35f;

    // Speed multiplier for day/night cycle (1.0 = real-time, 60.0 = 1 min per day)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Cycle", meta = (ClampMin = "0.0", ClampMax = "3600.0"))
    float DayCycleSpeed = 120.0f;

    // Whether the cycle is actively running
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Cycle")
    bool bCycleActive = true;

    // Preset settings for each time of day
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Presets")
    TArray<FLight_TimeOfDaySettings> TimeOfDayPresets;

    // Reference to the directional light (sun)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
    ADirectionalLight* SunLight = nullptr;

    // Reference to the height fog
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
    AExponentialHeightFog* HeightFog = nullptr;

    // Reference to the sky light
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
    ASkyLight* SkyLightActor = nullptr;

    // Get current time of day enum
    UFUNCTION(BlueprintCallable, Category = "Day/Night Cycle")
    ELight_TimeOfDay GetCurrentTimeOfDay() const;

    // Set time of day directly (0.0-1.0)
    UFUNCTION(BlueprintCallable, Category = "Day/Night Cycle")
    void SetTimeOfDay(float NormalizedTime);

    // Jump to a specific time of day preset
    UFUNCTION(BlueprintCallable, Category = "Day/Night Cycle")
    void JumpToTimeOfDay(ELight_TimeOfDay TargetTime);

    // Apply current lighting settings to scene
    UFUNCTION(BlueprintCallable, Category = "Day/Night Cycle")
    void ApplyLightingSettings(const FLight_TimeOfDaySettings& Settings);

    // Auto-find scene references
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Day/Night Cycle")
    void AutoFindSceneReferences();

protected:
    void InitializePresets();
    FLight_TimeOfDaySettings InterpolateSettings(const FLight_TimeOfDaySettings& A, const FLight_TimeOfDaySettings& B, float Alpha) const;
    FLight_TimeOfDaySettings GetSettingsForTime(float NormalizedTime) const;
    void UpdateSunLight(const FLight_TimeOfDaySettings& Settings);
    void UpdateFog(const FLight_TimeOfDaySettings& Settings);

private:
    ELight_TimeOfDay CachedTimeOfDay = ELight_TimeOfDay::Midday;
};
