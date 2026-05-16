#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DirectionalLight.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "EnvironmentalAtmosphere.generated.h"

UENUM(BlueprintType)
enum class EEnvArt_AtmosphereType : uint8
{
    Forest      UMETA(DisplayName = "Forest Atmosphere"),
    Swamp       UMETA(DisplayName = "Swamp Atmosphere"),
    Desert      UMETA(DisplayName = "Desert Atmosphere"),
    Mountain    UMETA(DisplayName = "Mountain Atmosphere"),
    Savanna     UMETA(DisplayName = "Savanna Atmosphere")
};

USTRUCT(BlueprintType)
struct FEnvArt_AtmosphereSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunAngle = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor FogColor = FLinearColor(0.7f, 0.8f, 0.9f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogHeightFalloff = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particles")
    bool bEnablePollenParticles = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particles")
    float ParticleSpawnRate = 10.0f;
};

UCLASS(ClassGroup=(Environment), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UEnvironmentalAtmosphere : public UActorComponent
{
    GENERATED_BODY()

public:
    UEnvironmentalAtmosphere();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    EEnvArt_AtmosphereType AtmosphereType = EEnvArt_AtmosphereType::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FEnvArt_AtmosphereSettings AtmosphereSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
    ADirectionalLight* SunLight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
    UExponentialHeightFogComponent* HeightFog;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
    UParticleSystemComponent* PollenParticles;

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SetAtmosphereType(EEnvArt_AtmosphereType NewType);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void ApplyAtmosphereSettings();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void UpdateSunPosition(float TimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SetFogSettings(FLinearColor Color, float Density, float HeightFalloff);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void TogglePollenParticles(bool bEnabled);

private:
    UPROPERTY()
    float CurrentTimeOfDay = 12.0f; // Noon by default

    void InitializeAtmosphereForType();
    FEnvArt_AtmosphereSettings GetDefaultSettingsForType(EEnvArt_AtmosphereType Type);
};

#include "EnvironmentalAtmosphere.generated.h"