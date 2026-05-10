#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DirectionalLight.h"
#include "Components/SkyLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/PostProcessVolume.h"
#include "EnvArt_LightingController.generated.h"

UENUM(BlueprintType)
enum class EEnvArt_TimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Morning     UMETA(DisplayName = "Morning"),
    Midday      UMETA(DisplayName = "Midday"),
    Afternoon   UMETA(DisplayName = "Afternoon"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Night       UMETA(DisplayName = "Night")
};

USTRUCT(BlueprintType)
struct FEnvArt_LightingSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    FRotator SunRotation = FRotator(-30.0f, 45.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    FLinearColor SunColor = FLinearColor(1.0f, 0.95f, 0.8f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    float SunIntensity = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky")
    FLinearColor SkyColor = FLinearColor(0.3f, 0.6f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky")
    float SkyIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor FogColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogHeightFalloff = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogStartDistance = 500.0f;

    FEnvArt_LightingSettings()
    {
        // Cretaceous period tropical lighting defaults
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UEnvArt_LightingController : public UActorComponent
{
    GENERATED_BODY()

public:
    UEnvArt_LightingController();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Lighting presets for different times of day
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Presets")
    TMap<EEnvArt_TimeOfDay, FEnvArt_LightingSettings> TimeOfDayPresets;

    // Current time of day
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    EEnvArt_TimeOfDay CurrentTimeOfDay = EEnvArt_TimeOfDay::Morning;

    // Day/night cycle settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    bool bEnableDayNightCycle = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time", meta = (ClampMin = "0.1", ClampMax = "60.0"))
    float DayDurationMinutes = 20.0f;

    // Lighting transition settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transitions")
    float TransitionSpeed = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transitions")
    bool bSmoothTransitions = true;

    // References to lighting actors
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
    ADirectionalLight* SunLight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
    AActor* SkyLightActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
    AActor* SkyAtmosphereActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
    AActor* FogActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
    APostProcessVolume* PostProcessVolume;

    // Functions
    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetTimeOfDay(EEnvArt_TimeOfDay NewTimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void ApplyLightingSettings(const FEnvArt_LightingSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetupCretaceousLighting();

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void CreateGoldenHourAtmosphere();

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetupVolumetricFog();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Editor")
    void FindLightingActors();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Editor")
    void InitializeTimeOfDayPresets();

private:
    // Internal state
    float CurrentDayTime = 0.0f;
    FEnvArt_LightingSettings CurrentSettings;
    FEnvArt_LightingSettings TargetSettings;
    bool bTransitioning = false;

    // Helper functions
    void UpdateDayNightCycle(float DeltaTime);
    void InterpolateLightingSettings(float DeltaTime);
    EEnvArt_TimeOfDay GetTimeOfDayFromDayTime(float DayTime);
    void UpdateSunLight(const FEnvArt_LightingSettings& Settings);
    void UpdateSkyLight(const FEnvArt_LightingSettings& Settings);
    void UpdateFog(const FEnvArt_LightingSettings& Settings);
    void UpdatePostProcess(const FEnvArt_LightingSettings& Settings);
};