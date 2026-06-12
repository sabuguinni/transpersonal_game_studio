#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DirectionalLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Light_DayNightSystem.generated.h"

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
    FLinearColor SunColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunAngle = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SkyColor = FLinearColor::Blue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor FogColor = FLinearColor::Gray;

    FLight_TimeSettings()
    {
        SunColor = FLinearColor::White;
        SunIntensity = 5.0f;
        SunAngle = 45.0f;
        SkyColor = FLinearColor::Blue;
        FogDensity = 0.02f;
        FogColor = FLinearColor::Gray;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API ULight_DayNightSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    ULight_DayNightSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Day/Night Cycle Control
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle")
    bool bEnableDayNightCycle = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle", meta = (ClampMin = "0.1", ClampMax = "10.0"))
    float CycleSpeed = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle")
    ELight_TimeOfDay CurrentTimeOfDay = ELight_TimeOfDay::Midday;

    // Time Settings for Each Period
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Settings")
    TMap<ELight_TimeOfDay, FLight_TimeSettings> TimeSettings;

    // Light References
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light References")
    ADirectionalLight* SunLight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light References")
    class ASkyLight* SkyLight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light References")
    class AExponentialHeightFog* HeightFog;

    // Blueprint Events
    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    void SetTimeOfDay(ELight_TimeOfDay NewTimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    void TransitionToTime(ELight_TimeOfDay TargetTime, float TransitionDuration = 2.0f);

    UFUNCTION(BlueprintPure, Category = "Day Night Cycle")
    float GetCurrentCycleProgress() const;

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    void InitializeLightReferences();

private:
    void InitializeTimeSettings();
    void UpdateLighting(const FLight_TimeSettings& Settings);
    void FindLightActors();
    
    float CurrentCycleTime = 0.0f;
    float TotalCycleTime = 1200.0f; // 20 minutes real time = 24 hours game time
    bool bIsTransitioning = false;
    float TransitionTimer = 0.0f;
    float TransitionDuration = 2.0f;
    ELight_TimeOfDay TransitionTarget = ELight_TimeOfDay::Midday;
    FLight_TimeSettings TransitionStartSettings;
};