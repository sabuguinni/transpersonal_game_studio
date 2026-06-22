#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "LightingAtmosphereSystem.generated.h"

UENUM(BlueprintType)
enum class ELight_TimeOfDay : uint8
{
    Dawn       UMETA(DisplayName = "Dawn"),
    Morning    UMETA(DisplayName = "Morning"),
    Midday     UMETA(DisplayName = "Midday"),
    Afternoon  UMETA(DisplayName = "Afternoon"),
    Dusk       UMETA(DisplayName = "Dusk"),
    Night      UMETA(DisplayName = "Night")
};

USTRUCT(BlueprintType)
struct FLight_AtmosphereSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunPitch = -45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SkyLightIntensity = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float FogHeightFalloff = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor = FLinearColor(1.0f, 0.95f, 0.8f, 1.0f);
};

/**
 * ALightingAtmosphereSystem
 * Manages the dynamic day/night cycle and atmospheric conditions
 * for the Cretaceous prehistoric survival world.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ALightingAtmosphereSystem : public AActor
{
    GENERATED_BODY()

public:
    ALightingAtmosphereSystem();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    /** Current time of day (0.0 = midnight, 0.5 = noon, 1.0 = midnight) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Cycle")
    float TimeOfDayNormalized = 0.35f;

    /** Speed of day/night cycle (1.0 = real-time, 100.0 = 100x faster) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Cycle")
    float DayCycleSpeed = 50.0f;

    /** Whether the day/night cycle is active */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Cycle")
    bool bDayCycleActive = true;

    /** Current atmospheric preset */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FLight_AtmosphereSettings AtmosphereSettings;

    /** Reference to the directional light (sun) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting References")
    TObjectPtr<ADirectionalLight> SunLight;

    /** Reference to the sky light */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting References")
    TObjectPtr<ASkyLight> SkyLightActor;

    /** Reference to the exponential height fog */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting References")
    TObjectPtr<AExponentialHeightFog> HeightFogActor;

    /** Get the current time of day enum */
    UFUNCTION(BlueprintCallable, Category = "Day/Night Cycle")
    ELight_TimeOfDay GetCurrentTimeOfDay() const;

    /** Set time of day directly (0.0 - 1.0) */
    UFUNCTION(BlueprintCallable, Category = "Day/Night Cycle")
    void SetTimeOfDay(float NewTime);

    /** Apply current atmosphere settings to all light actors */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Lighting")
    void ApplyAtmosphereSettings();

    /** Auto-find and cache light references in the world */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Lighting")
    void AutoFindLightActors();

protected:
    void UpdateSunPosition(float DeltaTime);
    void UpdateFogForTimeOfDay();
    void UpdateSkyLightForTimeOfDay();

    FLight_AtmosphereSettings GetSettingsForTime(float NormalizedTime) const;
};
