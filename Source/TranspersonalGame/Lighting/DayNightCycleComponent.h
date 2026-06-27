#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/SkyLight.h"
#include "DayNightCycleComponent.generated.h"

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
    FLinearColor SunColor = FLinearColor(1.0f, 0.78f, 0.55f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity = 8.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunPitch = -45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor FogColor = FLinearColor(0.6f, 0.7f, 0.9f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SkyLightIntensity = 1.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor AmbientColor = FLinearColor(0.1f, 0.12f, 0.18f, 1.0f);
};

/**
 * ULight_DayNightCycleComponent
 * Drives the full day/night cycle for the prehistoric survival world.
 * Attach to a persistent actor (e.g., GameMode or WorldSettings actor).
 * Controls: DirectionalLight (sun), ExponentialHeightFog, SkyLight.
 */
UCLASS(ClassGroup = (Lighting), meta = (BlueprintSpawnableComponent), DisplayName = "Day Night Cycle Component")
class TRANSPERSONALGAME_API ULight_DayNightCycleComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    ULight_DayNightCycleComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // --- Public API ---

    /** Current time of day in hours (0.0 = midnight, 12.0 = noon, 24.0 = midnight) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle")
    float CurrentHour = 8.0f;

    /** Speed multiplier for time progression (1.0 = real time, 60.0 = 1 min = 1 game hour) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle")
    float TimeScale = 60.0f;

    /** Whether the cycle is actively running */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle")
    bool bCycleActive = true;

    /** Reference to the sun DirectionalLight actor */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|References")
    TObjectPtr<ADirectionalLight> SunLight = nullptr;

    /** Reference to the ExponentialHeightFog actor */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|References")
    TObjectPtr<AExponentialHeightFog> HeightFog = nullptr;

    /** Reference to the SkyLight actor */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|References")
    TObjectPtr<ASkyLight> SkyLightActor = nullptr;

    /** Presets for each time of day phase */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|Presets")
    FLight_TimeOfDaySettings DawnSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|Presets")
    FLight_TimeOfDaySettings MiddaySettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|Presets")
    FLight_TimeOfDaySettings DuskSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle|Presets")
    FLight_TimeOfDaySettings NightSettings;

    /** Get current time of day enum */
    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    ELight_TimeOfDay GetCurrentTimeOfDay() const;

    /** Get normalized time (0.0 to 1.0 over 24h) */
    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    float GetNormalizedTime() const;

    /** Set time directly (0-24 hours) */
    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    void SetHour(float NewHour);

    /** Auto-discover lighting actors in the world */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Day Night Cycle")
    void AutoDiscoverLightingActors();

    /** Apply current time settings immediately */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Day Night Cycle")
    void ApplyCurrentTimeSettings();

private:
    void AdvanceTime(float DeltaTime);
    void UpdateLighting(float Alpha, const FLight_TimeOfDaySettings& From, const FLight_TimeOfDaySettings& To);
    void ApplySettingsToLights(const FLight_TimeOfDaySettings& Settings);
    FLight_TimeOfDaySettings LerpSettings(const FLight_TimeOfDaySettings& A, const FLight_TimeOfDaySettings& B, float Alpha) const;
    void GetPhaseSettings(float Hour, FLight_TimeOfDaySettings& OutFrom, FLight_TimeOfDaySettings& OutTo, float& OutAlpha) const;

    float AccumulatedTime = 0.0f;
};
