#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Particles/ParticleSystemComponent.h"
#include "VFX_EnvironmentalEffects.generated.h"

UENUM(BlueprintType)
enum class EVFX_EnvironmentalType : uint8
{
    Campfire        UMETA(DisplayName = "Campfire"),
    WaterSplash     UMETA(DisplayName = "Water Splash"),
    DustCloud       UMETA(DisplayName = "Dust Cloud"),
    FootstepImpact  UMETA(DisplayName = "Footstep Impact"),
    WindEffect      UMETA(DisplayName = "Wind Effect")
};

USTRUCT(BlueprintType)
struct FVFX_EnvironmentalConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    EVFX_EnvironmentalType EffectType = EVFX_EnvironmentalType::Campfire;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    float IntensityScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    float Duration = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    bool bAutoDestroy = true;

    FVFX_EnvironmentalConfig()
    {
        EffectType = EVFX_EnvironmentalType::Campfire;
        IntensityScale = 1.0f;
        Duration = 5.0f;
        bAutoDestroy = true;
    }
};

UCLASS(ClassGroup=(VFX), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UVFX_EnvironmentalEffects : public UActorComponent
{
    GENERATED_BODY()

public:
    UVFX_EnvironmentalEffects();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Environmental VFX spawning
    UFUNCTION(BlueprintCallable, Category = "VFX Environmental")
    void SpawnEnvironmentalEffect(EVFX_EnvironmentalType EffectType, FVector Location, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX Environmental")
    void SpawnCampfireEffect(FVector Location, float FireIntensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX Environmental")
    void SpawnWaterSplashEffect(FVector Location, float SplashSize = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX Environmental")
    void SpawnDustCloudEffect(FVector Location, float CloudSize = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX Environmental")
    void SpawnFootstepImpact(FVector Location, float ImpactForce = 1.0f);

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    TArray<FVFX_EnvironmentalConfig> EnvironmentalConfigs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    float GlobalIntensityMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    bool bEnableEnvironmentalVFX = true;

private:
    // Active particle systems tracking
    UPROPERTY()
    TArray<UParticleSystemComponent*> ActiveParticleSystems;

    // Cleanup timer
    FTimerHandle CleanupTimerHandle;

    void CleanupExpiredEffects();
    UParticleSystemComponent* CreateParticleEffect(FVector Location, EVFX_EnvironmentalType EffectType);
};