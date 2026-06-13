#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/World.h"
#include "Light_AtmosphericSystem.generated.h"

UENUM(BlueprintType)
enum class ELight_TimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Morning     UMETA(DisplayName = "Morning"),
    Noon        UMETA(DisplayName = "Noon"),
    Afternoon   UMETA(DisplayName = "Afternoon"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Night       UMETA(DisplayName = "Night")
};

USTRUCT(BlueprintType)
struct FLight_AtmosphericSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor = FLinearColor(1.0f, 0.94f, 0.78f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SkyLightIntensity = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogHeightFalloff = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor FogInscatteringColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetric")
    bool bVolumetricFog = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetric")
    float VolumetricScatteringDistribution = 0.2f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ALight_AtmosphericSystem : public AActor
{
    GENERATED_BODY()

public:
    ALight_AtmosphericSystem();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    ELight_TimeOfDay CurrentTimeOfDay = ELight_TimeOfDay::Morning;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    FLight_AtmosphericSettings AtmosphericSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cycle")
    bool bEnableDayNightCycle = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cycle", meta = (EditCondition = "bEnableDayNightCycle"))
    float DayDurationMinutes = 20.0f;

    UPROPERTY(BlueprintReadOnly, Category = "References")
    ADirectionalLight* SunLight;

    UPROPERTY(BlueprintReadOnly, Category = "References")
    ASkyLight* SkyLightActor;

    UPROPERTY(BlueprintReadOnly, Category = "References")
    AExponentialHeightFog* FogActor;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetTimeOfDay(ELight_TimeOfDay NewTimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void ApplyAtmosphericSettings(const FLight_AtmosphericSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetCretaceousLighting();

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void UpdateVolumetricFog(bool bEnable, float Density = 0.02f);

    UFUNCTION(CallInEditor, Category = "Setup")
    void FindAndConfigureLightingActors();

protected:
    UFUNCTION()
    void UpdateDayNightCycle(float DeltaTime);

    UFUNCTION()
    void ConfigureDirectionalLight();

    UFUNCTION()
    void ConfigureSkyLight();

    UFUNCTION()
    void ConfigureHeightFog();

private:
    float CurrentCycleTime = 0.0f;
};