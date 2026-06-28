#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/SkyLight.h"
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
struct FLight_SunState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunPitch = -45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunYaw = -60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity = 8.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor = FLinearColor(1.0f, 0.85f, 0.6f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor FogColor = FLinearColor(0.9f, 0.85f, 0.75f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SkyLightIntensity = 1.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SkyLightColor = FLinearColor(0.6f, 0.7f, 1.0f, 1.0f);
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ADayNightCycleManager : public AActor
{
    GENERATED_BODY()

public:
    ADayNightCycleManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // --- Configuration ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle")
    float DayDurationSeconds = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle")
    float CurrentTimeOfDayNormalized = 0.35f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle")
    bool bCycleEnabled = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle")
    float CycleSpeedMultiplier = 1.0f;

    // --- Scene References ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scene References")
    ADirectionalLight* SunLight = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scene References")
    AExponentialHeightFog* HeightFog = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scene References")
    ASkyLight* SceneSkyLight = nullptr;

    // --- State ---
    UPROPERTY(BlueprintReadOnly, Category = "Day Night Cycle")
    ELight_TimeOfDay CurrentTimeOfDayEnum = ELight_TimeOfDay::GoldenHour;

    UPROPERTY(BlueprintReadOnly, Category = "Day Night Cycle")
    FLight_SunState CurrentSunState;

    // --- Keyframe States ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Keyframes")
    FLight_SunState DawnState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Keyframes")
    FLight_SunState MiddayState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Keyframes")
    FLight_SunState GoldenHourState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Keyframes")
    FLight_SunState NightState;

    // --- Blueprint Events ---
    UFUNCTION(BlueprintImplementableEvent, Category = "Day Night Cycle")
    void OnTimeOfDayChanged(ELight_TimeOfDay NewTimeOfDay);

    UFUNCTION(BlueprintImplementableEvent, Category = "Day Night Cycle")
    void OnDawnBegins();

    UFUNCTION(BlueprintImplementableEvent, Category = "Day Night Cycle")
    void OnNightBegins();

    // --- Callable Functions ---
    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    void SetTimeOfDay(float NormalizedTime);

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    float GetCurrentHour() const;

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    ELight_TimeOfDay GetTimeOfDayEnum() const;

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    void AutoFindSceneActors();

    UFUNCTION(CallInEditor, Category = "Day Night Cycle")
    void ApplyCurrentStateToScene();

private:
    void AdvanceCycle(float DeltaTime);
    void UpdateSceneLighting();
    FLight_SunState InterpolateSunStates(const FLight_SunState& A, const FLight_SunState& B, float Alpha) const;
    ELight_TimeOfDay TimeToEnum(float NormalizedTime) const;
    void InitKeyframeDefaults();
};
