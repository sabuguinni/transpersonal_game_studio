#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/PointLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/PointLightComponent.h"
#include "Light_CretaceousAtmosphereController.generated.h"

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
struct FLight_AtmosphereSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    float SunIntensity = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    FLinearColor SunColor = FLinearColor(1.0f, 0.96f, 0.88f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    float SunAngle = -45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky")
    float SkyLightIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky")
    FLinearColor SkyLightColor = FLinearColor(0.5f, 0.7f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor FogColor = FLinearColor(0.5f, 0.6f, 0.8f, 1.0f);

    FLight_AtmosphereSettings()
    {
        SunIntensity = 10.0f;
        SunColor = FLinearColor(1.0f, 0.96f, 0.88f, 1.0f);
        SunAngle = -45.0f;
        SkyLightIntensity = 1.0f;
        SkyLightColor = FLinearColor(0.5f, 0.7f, 1.0f, 1.0f);
        FogDensity = 0.02f;
        FogColor = FLinearColor(0.5f, 0.6f, 0.8f, 1.0f);
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ALight_CretaceousAtmosphereController : public AActor
{
    GENERATED_BODY()

public:
    ALight_CretaceousAtmosphereController();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    // Current time of day (0-24 hours)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time", meta = (ClampMin = "0.0", ClampMax = "24.0"))
    float CurrentTime = 12.0f;

    // Day cycle speed multiplier
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    float DayCycleSpeed = 1.0f;

    // Enable automatic day/night cycle
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    bool bEnableAutomaticCycle = true;

    // Atmosphere settings for different times
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FLight_AtmosphereSettings DawnSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FLight_AtmosphereSettings NoonSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FLight_AtmosphereSettings DuskSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FLight_AtmosphereSettings NightSettings;

    // Light references
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lights")
    ADirectionalLight* SunLight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lights")
    ADirectionalLight* MoonLight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lights")
    ASkyLight* SkyLightActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lights")
    AExponentialHeightFog* FogActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lights")
    TArray<APointLight*> BioluminescentLights;

    // Functions
    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SetTimeOfDay(float NewTime);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    ELight_TimeOfDay GetCurrentTimeOfDay() const;

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void UpdateAtmosphere();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SetupCretaceousAtmosphere();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void FindLightActors();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    FLight_AtmosphereSettings GetInterpolatedSettings() const;

private:
    void UpdateSunLight(const FLight_AtmosphereSettings& Settings);
    void UpdateMoonLight(const FLight_AtmosphereSettings& Settings);
    void UpdateSkyLight(const FLight_AtmosphereSettings& Settings);
    void UpdateFog(const FLight_AtmosphereSettings& Settings);
    void UpdateBioluminescence(const FLight_AtmosphereSettings& Settings);

    float InterpolateFloat(float A, float B, float Alpha) const;
    FLinearColor InterpolateColor(const FLinearColor& A, const FLinearColor& B, float Alpha) const;
};