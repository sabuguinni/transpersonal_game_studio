#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/PointLightComponent.h"
#include "VFXSystemManager.generated.h"

UENUM(BlueprintType)
enum class EVFX_EffectCategory : uint8
{
    Environment     UMETA(DisplayName = "Environment"),
    Dinosaur        UMETA(DisplayName = "Dinosaur"),
    Combat          UMETA(DisplayName = "Combat"),
    Weather         UMETA(DisplayName = "Weather"),
    Crafting        UMETA(DisplayName = "Crafting")
};

USTRUCT(BlueprintType)
struct FVFX_CampfireData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Campfire")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Campfire")
    float Intensity = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Campfire")
    FLinearColor FireColor = FLinearColor(1.0f, 0.45f, 0.1f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Campfire")
    float AttenuationRadius = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Campfire")
    bool bIsActive = true;
};

USTRUCT(BlueprintType)
struct FVFX_FootstepData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Footstep")
    FVector ImpactLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Footstep")
    float DustRadius = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Footstep")
    float ImpactForce = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Footstep")
    FName DinosaurSpecies = NAME_None;
};

UCLASS(ClassGroup = (VFX), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UVFX_SystemManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UVFX_SystemManager();

    UFUNCTION(BlueprintCallable, Category = "VFX|Campfire")
    void SpawnCampfireEffect(const FVFX_CampfireData& CampfireData);

    UFUNCTION(BlueprintCallable, Category = "VFX|Footstep")
    void SpawnFootstepDust(const FVFX_FootstepData& FootstepData);

    UFUNCTION(BlueprintCallable, Category = "VFX|Weather")
    void SetWeatherIntensity(float RainIntensity, float FogDensity, float WindStrength);

    UFUNCTION(BlueprintCallable, Category = "VFX|Combat")
    void SpawnBloodImpact(FVector Location, FVector Direction, float Intensity);

    UFUNCTION(BlueprintCallable, Category = "VFX|Environment")
    void SpawnVolcanicAsh(FVector SourceLocation, float Radius);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Settings")
    float GlobalVFXQuality = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Settings")
    int32 MaxActiveParticleSystems = 32;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Settings")
    bool bEnableLOD = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Settings")
    float LOD_HighQualityDistance = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Settings")
    float LOD_MediumQualityDistance = 4000.0f;

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    UPROPERTY()
    TArray<AActor*> ActiveCampfires;

    UPROPERTY()
    int32 ActiveParticleCount = 0;

    float WeatherRainIntensity = 0.0f;
    float WeatherFogDensity = 0.0f;
    float WeatherWindStrength = 0.0f;
};
