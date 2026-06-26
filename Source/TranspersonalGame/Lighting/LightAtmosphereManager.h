#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "LightAtmosphereManager.generated.h"

UENUM(BlueprintType)
enum class ELight_TimeOfDay : uint8
{
    Dawn       UMETA(DisplayName = "Dawn"),
    Morning    UMETA(DisplayName = "Morning"),
    GoldenHour UMETA(DisplayName = "Golden Hour"),
    Noon       UMETA(DisplayName = "Noon"),
    Dusk       UMETA(DisplayName = "Dusk"),
    Night      UMETA(DisplayName = "Night")
};

USTRUCT(BlueprintType)
struct FLight_SunConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunPitchAngle = -25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunYawAngle = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor = FLinearColor(1.0f, 0.92f, 0.75f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float VolumetricScatteringIntensity = 4.0f;
};

USTRUCT(BlueprintType)
struct FLight_FogConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogDensity = 0.035f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogHeightFalloff = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor FogColor = FLinearColor(0.55f, 0.72f, 0.85f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    bool bVolumetricFog = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float VolumetricFogViewDistance = 80000.0f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ALightAtmosphereManager : public AActor
{
    GENERATED_BODY()

public:
    ALightAtmosphereManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Current time of day
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Time")
    ELight_TimeOfDay CurrentTimeOfDay = ELight_TimeOfDay::GoldenHour;

    // Day/night cycle speed (0 = static)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Time")
    float DayCycleSpeed = 0.0f;

    // Sun configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Sun")
    FLight_SunConfig SunConfig;

    // Fog configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Fog")
    FLight_FogConfig FogConfig;

    // SkyLight intensity
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Sky")
    float SkyLightIntensity = 2.5f;

    // Apply current lighting configuration
    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void ApplyLightingConfig();

    // Transition to a new time of day
    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetTimeOfDay(ELight_TimeOfDay NewTime);

    // Get normalized time (0-1, 0=midnight, 0.5=noon)
    UFUNCTION(BlueprintPure, Category = "Lighting")
    float GetNormalizedTime() const;

private:
    float CurrentDayProgress = 0.35f; // Start at golden hour

    UPROPERTY()
    class ADirectionalLight* SunActor;

    UPROPERTY()
    class ASkyLight* SkyLightActor;

    UPROPERTY()
    class AExponentialHeightFog* FogActor;

    void FindLightingActors();
    void UpdateSunForTimeOfDay();
    FLight_SunConfig GetSunConfigForTime(ELight_TimeOfDay Time) const;
};
