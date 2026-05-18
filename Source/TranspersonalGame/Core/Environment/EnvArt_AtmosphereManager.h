#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/AtmosphericFog.h"
#include "Engine/ExponentialHeightFog.h"
#include "Particles/ParticleSystemComponent.h"
#include "EnvArt_AtmosphereManager.generated.h"

UENUM(BlueprintType)
enum class EEnvArt_TimeOfDay : uint8
{
    Dawn,
    Morning,
    Noon,
    Afternoon,
    Dusk,
    Night
};

USTRUCT(BlueprintType)
struct FEnvArt_AtmosphereSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FRotator SunRotation = FRotator(-15.0f, 45.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor FogColor = FLinearColor(0.7f, 0.8f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particles")
    bool bEnableAtmosphericParticles = true;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UEnvArt_AtmosphereManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UEnvArt_AtmosphereManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    EEnvArt_TimeOfDay CurrentTimeOfDay = EEnvArt_TimeOfDay::Morning;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    float TimeOfDaySpeed = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    TMap<EEnvArt_TimeOfDay, FEnvArt_AtmosphereSettings> TimeSettings;

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SetTimeOfDay(EEnvArt_TimeOfDay NewTimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void UpdateAtmosphere();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void CreateAtmosphericEffects();

private:
    UPROPERTY()
    ADirectionalLight* SunLight;

    UPROPERTY()
    AAtmosphericFog* AtmosphericFog;

    UPROPERTY()
    AExponentialHeightFog* HeightFog;

    UPROPERTY()
    TArray<UParticleSystemComponent*> ParticleComponents;

    float CurrentTimeValue = 0.5f;

    void FindLightingActors();
    void UpdateSunLighting(const FEnvArt_AtmosphereSettings& Settings);
    void UpdateFogSettings(const FEnvArt_AtmosphereSettings& Settings);
    void InitializeTimeSettings();
};