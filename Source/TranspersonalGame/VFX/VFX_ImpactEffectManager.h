#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "VFX_ImpactEffectManager.generated.h"

UENUM(BlueprintType)
enum class EVFX_ImpactType : uint8
{
    FootstepDust    UMETA(DisplayName = "Footstep Dust"),
    BloodSplatter   UMETA(DisplayName = "Blood Splatter"),
    RockDebris      UMETA(DisplayName = "Rock Debris"),
    WaterSplash     UMETA(DisplayName = "Water Splash"),
    FireSparks      UMETA(DisplayName = "Fire Sparks")
};

USTRUCT(BlueprintType)
struct FVFX_ImpactEffectData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    EVFX_ImpactType EffectType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FVector Scale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float Intensity;

    FVFX_ImpactEffectData()
    {
        EffectType = EVFX_ImpactType::FootstepDust;
        Location = FVector::ZeroVector;
        Scale = FVector::OneVector;
        Duration = 2.0f;
        Intensity = 1.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UVFX_ImpactEffectManager : public UObject
{
    GENERATED_BODY()

public:
    UVFX_ImpactEffectManager();

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void CreateImpactEffect(const FVFX_ImpactEffectData& EffectData);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void CreateFootstepDust(FVector Location, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void CreateBloodSplatter(FVector Location, FVector Direction, float Amount = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void CreateWaterSplash(FVector Location, float SplashSize = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void CreateFireSparks(FVector Location, int32 SparkCount = 10);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void CleanupExpiredEffects();

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX")
    TArray<AActor*> ActiveEffects;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float MaxEffectLifetime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    int32 MaxActiveEffects;

private:
    void SpawnEffectActor(const FVFX_ImpactEffectData& EffectData);
    void RegisterEffect(AActor* EffectActor);
};