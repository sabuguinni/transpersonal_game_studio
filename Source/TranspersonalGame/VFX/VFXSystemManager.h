#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Engine/StaticMeshActor.h"
#include "SharedTypes.h"
#include "VFXSystemManager.generated.h"

UENUM(BlueprintType)
enum class EVFX_EffectType : uint8
{
    None = 0,
    FootstepDust,
    CampfireFire,
    WeatherRain,
    WeatherDustStorm,
    WeatherMist,
    BloodSplatter,
    ImpactSpark,
    BreathVapor,
    DinosaurRoar
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVFX_EffectData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    EVFX_EffectType EffectType = EVFX_EffectType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FRotator Rotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FVector Scale = FVector::OneVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float Duration = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    bool bAutoDestroy = true;

    FVFX_EffectData()
    {
        EffectType = EVFX_EffectType::None;
        Location = FVector::ZeroVector;
        Rotation = FRotator::ZeroRotator;
        Scale = FVector::OneVector;
        Duration = 5.0f;
        bAutoDestroy = true;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UVFX_SystemManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UVFX_SystemManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Core VFX spawning functions
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnVFXEffect(EVFX_EffectType EffectType, FVector Location, FRotator Rotation = FRotator::ZeroRotator, FVector Scale = FVector::OneVector);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnFootstepDust(FVector Location, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnCampfire(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnWeatherEffect(EVFX_EffectType WeatherType, FVector Location, FVector Scale = FVector(5.0f, 5.0f, 5.0f));

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnImpactEffect(FVector Location, EVFX_EffectType ImpactType = EVFX_EffectType::ImpactSpark);

    // VFX management
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void ClearAllVFX();

    UFUNCTION(BlueprintCallable, Category = "VFX")
    int32 GetActiveVFXCount() const;

    // Biome-specific VFX
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SetupBiomeVFX(EBiomeType BiomeType, FVector BiomeCenter);

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX")
    TArray<class UParticleSystemComponent*> ActiveVFXComponents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    int32 MaxActiveVFX = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    bool bEnableVFXLOD = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float VFXCullingDistance = 10000.0f;

private:
    void CleanupExpiredVFX();
    UParticleSystemComponent* CreateVFXComponent(EVFX_EffectType EffectType, const FVFX_EffectData& EffectData);
    void ApplyVFXLOD(UParticleSystemComponent* VFXComponent, float DistanceToPlayer);
};