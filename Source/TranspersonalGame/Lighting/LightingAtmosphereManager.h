#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyLightComponent.h"
#include "LightingAtmosphereManager.generated.h"

// ============================================================
// ELight_TimeOfDay — Prehistoric day cycle phases
// ============================================================
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

// ============================================================
// FLight_SunConfig — Sun directional light settings per phase
// ============================================================
USTRUCT(BlueprintType)
struct FLight_SunConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunPitch = -45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor FogInscatteringColor = FLinearColor(0.5f, 0.6f, 0.8f, 1.0f);
};

// ============================================================
// ALightingAtmosphereManager — Day/Night cycle controller
// ============================================================
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ALightingAtmosphereManager : public AActor
{
    GENERATED_BODY()

public:
    ALightingAtmosphereManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Current time of day (0.0 = midnight, 0.5 = noon, 1.0 = midnight)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|DayNight")
    float TimeOfDayNormalized = 0.25f;

    // Speed multiplier for day/night cycle (1.0 = real-time, 100.0 = fast cycle)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|DayNight")
    float DayNightCycleSpeed = 50.0f;

    // Whether the day/night cycle is active
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|DayNight")
    bool bDayNightCycleActive = true;

    // Reference to the directional light (sun)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|References")
    TObjectPtr<ADirectionalLight> SunLight;

    // Reference to the sky light
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|References")
    TObjectPtr<ASkyLight> SkyLightActor;

    // Reference to the exponential height fog
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|References")
    TObjectPtr<AExponentialHeightFog> HeightFog;

    // Sun configs per time-of-day phase
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Phases")
    TMap<ELight_TimeOfDay, FLight_SunConfig> PhaseConfigs;

    // Current phase
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting|DayNight")
    ELight_TimeOfDay CurrentPhase = ELight_TimeOfDay::Morning;

    // Get current time as enum phase
    UFUNCTION(BlueprintCallable, Category = "Lighting")
    ELight_TimeOfDay GetCurrentPhase() const;

    // Force a specific time of day
    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetTimeOfDay(float NormalizedTime);

    // Apply sun config for current phase
    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void ApplyPhaseConfig(ELight_TimeOfDay Phase);

    // Auto-find lighting actors in the world
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Lighting")
    void AutoFindLightingActors();

private:
    void UpdateDayNightCycle(float DeltaTime);
    void UpdateSunRotation();
    void UpdateFogForPhase(const FLight_SunConfig& Config);
    ELight_TimeOfDay NormalizedTimeToPhase(float T) const;
    void InitDefaultPhaseConfigs();
};
