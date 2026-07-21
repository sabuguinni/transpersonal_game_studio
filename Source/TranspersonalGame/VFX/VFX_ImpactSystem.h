#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Particles/ParticleSystem.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "Sound/SoundBase.h"
#include "SharedTypes.h"
#include "VFX_ImpactSystem.generated.h"

UENUM(BlueprintType)
enum class EVFX_ImpactType : uint8
{
    FootstepLight    UMETA(DisplayName = "Light Footstep"),
    FootstepHeavy    UMETA(DisplayName = "Heavy Footstep"),
    DinosaurStomp    UMETA(DisplayName = "Dinosaur Stomp"),
    WeaponHit        UMETA(DisplayName = "Weapon Hit"),
    RockImpact       UMETA(DisplayName = "Rock Impact"),
    WaterSplash      UMETA(DisplayName = "Water Splash"),
    BloodSplatter    UMETA(DisplayName = "Blood Splatter"),
    Explosion        UMETA(DisplayName = "Explosion")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVFX_ImpactData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact VFX")
    UNiagaraSystem* ParticleEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact VFX")
    USoundBase* ImpactSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact VFX")
    float EffectScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact VFX")
    float SoundVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact VFX")
    FLinearColor ParticleColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact VFX")
    float Duration;

    FVFX_ImpactData()
    {
        ParticleEffect = nullptr;
        ImpactSound = nullptr;
        EffectScale = 1.0f;
        SoundVolume = 1.0f;
        ParticleColor = FLinearColor::White;
        Duration = 2.0f;
    }
};

UCLASS(ClassGroup=(VFX), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UVFX_ImpactSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UVFX_ImpactSystem();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact System")
    TMap<EVFX_ImpactType, FVFX_ImpactData> ImpactEffects;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact System")
    bool bAutoCleanupEffects;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact System")
    float CleanupDelay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact System")
    int32 MaxSimultaneousEffects;

    UPROPERTY()
    TArray<UNiagaraComponent*> ActiveEffects;

public:
    UFUNCTION(BlueprintCallable, Category = "Impact System")
    void TriggerImpact(EVFX_ImpactType ImpactType, FVector Location, FRotator Rotation = FRotator::ZeroRotator, float ScaleMultiplier = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Impact System")
    void TriggerFootstepImpact(FVector Location, float DinosaurSize = 1.0f, ESurfaceType SurfaceType = ESurfaceType::Default);

    UFUNCTION(BlueprintCallable, Category = "Impact System")
    void TriggerWeaponImpact(FVector Location, FVector ImpactNormal, float Damage = 10.0f);

    UFUNCTION(BlueprintCallable, Category = "Impact System")
    void TriggerBloodEffect(FVector Location, FVector Direction, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Impact System")
    void CleanupOldEffects();

    UFUNCTION(BlueprintCallable, Category = "Impact System")
    void SetImpactData(EVFX_ImpactType ImpactType, const FVFX_ImpactData& NewData);

    UFUNCTION(BlueprintCallable, Category = "Impact System")
    FVFX_ImpactData GetImpactData(EVFX_ImpactType ImpactType) const;

private:
    void SpawnImpactEffect(const FVFX_ImpactData& ImpactData, FVector Location, FRotator Rotation, float Scale);
    void PlayImpactSound(USoundBase* Sound, FVector Location, float Volume);
    ESurfaceType GetSurfaceTypeAtLocation(FVector Location);
};