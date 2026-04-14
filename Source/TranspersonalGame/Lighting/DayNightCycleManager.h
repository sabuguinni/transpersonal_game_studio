#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyAtmosphere.h"
#include "Engine/ExponentialHeightFog.h"
#include "Subsystems/WorldSubsystem.h"
#include "../SharedTypes.h"
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
struct TRANSPERSONALGAME_API FLight_TimeOfDaySettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor = FLinearColor(1.0f, 0.95f, 0.8f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunAngle = -45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor FogColor = FLinearColor(0.4f, 0.6f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SkyAtmosphereIntensity = 1.0f;

    FLight_TimeOfDaySettings()
    {
        SunIntensity = 3.0f;
        SunColor = FLinearColor(1.0f, 0.95f, 0.8f, 1.0f);
        SunAngle = -45.0f;
        FogDensity = 0.02f;
        FogColor = FLinearColor(0.4f, 0.6f, 1.0f, 1.0f);
        SkyAtmosphereIntensity = 1.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ADayNightCycleManager : public AActor
{
    GENERATED_BODY()

public:
    ADayNightCycleManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Day/Night Cycle Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle", meta = (ClampMin = "0.1", ClampMax = "10.0"))
    float CycleSpeed = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle", meta = (ClampMin = "0.0", ClampMax = "24.0"))
    float CurrentTimeOfDay = 12.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle")
    bool bEnableAutomaticCycle = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle")
    bool bUseSmoothTransitions = true;

    // Lighting References
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting References")
    ADirectionalLight* SunLight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting References")
    ASkyAtmosphere* SkyAtmosphere;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting References")
    AExponentialHeightFog* AtmosphericFog;

    // Time of Day Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Settings")
    TMap<ELight_TimeOfDay, FLight_TimeOfDaySettings> TimeOfDaySettings;

    // Public Functions
    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    void SetTimeOfDay(float NewTime);

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    ELight_TimeOfDay GetCurrentTimeOfDayEnum() const;

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    void SetCycleSpeed(float NewSpeed);

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    void PauseCycle();

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    void ResumeCycle();

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    void TransitionToTimeOfDay(ELight_TimeOfDay TargetTime, float TransitionDuration = 5.0f);

    // Editor Functions
    UFUNCTION(CallInEditor, Category = "Day Night Cycle")
    void SetupDefaultLightingActors();

    UFUNCTION(CallInEditor, Category = "Day Night Cycle")
    void InitializeDefaultTimeSettings();

protected:
    // Internal Functions
    void UpdateLighting();
    void UpdateSunPosition();
    void UpdateSunLighting();
    void UpdateAtmosphericFog();
    void UpdateSkyAtmosphere();
    FLight_TimeOfDaySettings GetInterpolatedSettings() const;
    ELight_TimeOfDay TimeFloatToEnum(float TimeValue) const;

    // Transition System
    bool bIsTransitioning = false;
    float TransitionStartTime = 0.0f;
    float TransitionDuration = 5.0f;
    float TransitionTargetTime = 12.0f;
    float TransitionStartTimeValue = 12.0f;

    // Internal State
    bool bCyclePaused = false;
    ELight_TimeOfDay LastTimeOfDay = ELight_TimeOfDay::Midday;
};

UCLASS()
class TRANSPERSONALGAME_API ULight_DayNightSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    ADayNightCycleManager* GetDayNightManager() const;

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    void RegisterDayNightManager(ADayNightCycleManager* Manager);

protected:
    UPROPERTY()
    ADayNightCycleManager* DayNightManager;
};