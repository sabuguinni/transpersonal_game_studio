#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/DirectionalLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "EnvironmentAtmosphere.generated.h"

UENUM(BlueprintType)
enum class EEnvArt_TimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Morning     UMETA(DisplayName = "Morning"), 
    Noon        UMETA(DisplayName = "Noon"),
    Afternoon   UMETA(DisplayName = "Afternoon"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Night       UMETA(DisplayName = "Night")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_AtmosphereSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity = 3.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FRotator SunRotation = FRotator(-30.0f, 45.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogHeightFalloff = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor FogColor = FLinearColor(0.7f, 0.8f, 0.9f, 1.0f);

    FEnvArt_AtmosphereSettings()
    {
        SunIntensity = 3.5f;
        SunColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);
        SunRotation = FRotator(-30.0f, 45.0f, 0.0f);
        FogDensity = 0.02f;
        FogHeightFalloff = 0.2f;
        FogColor = FLinearColor(0.7f, 0.8f, 0.9f, 1.0f);
    }
};

UCLASS(ClassGroup=(Environment), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UEnvironmentAtmosphere : public UActorComponent
{
    GENERATED_BODY()

public:
    UEnvironmentAtmosphere();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    EEnvArt_TimeOfDay CurrentTimeOfDay = EEnvArt_TimeOfDay::Afternoon;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FEnvArt_AtmosphereSettings AtmosphereSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    bool bEnableDynamicLighting = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    bool bEnableVolumetricFog = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float DayDurationSeconds = 1200.0f; // 20 minutes

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SetTimeOfDay(EEnvArt_TimeOfDay NewTimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void UpdateAtmosphere();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void CreateVolumetricFog(FVector Location, const FEnvArt_AtmosphereSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void UpdateDirectionalLight(const FEnvArt_AtmosphereSettings& Settings);

private:
    UPROPERTY()
    class ADirectionalLight* SunLight;

    UPROPERTY()
    class AExponentialHeightFog* VolumeFog;

    float CurrentDayTime = 0.0f;

    void FindOrCreateLightingActors();
    FEnvArt_AtmosphereSettings GetSettingsForTimeOfDay(EEnvArt_TimeOfDay TimeOfDay) const;
};