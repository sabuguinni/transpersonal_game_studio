#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Sound/SoundCue.h"
#include "VFX_ImpactEffectManager.generated.h"

UENUM(BlueprintType)
enum class EVFX_ImpactType : uint8
{
    Footstep        UMETA(DisplayName = "Footstep"),
    WeaponHit       UMETA(DisplayName = "Weapon Hit"),
    Fall            UMETA(DisplayName = "Fall Impact"),
    RockCollision   UMETA(DisplayName = "Rock Collision"),
    WaterSplash     UMETA(DisplayName = "Water Splash")
};

UENUM(BlueprintType)
enum class EVFX_SurfaceType : uint8
{
    Dirt            UMETA(DisplayName = "Dirt"),
    Rock            UMETA(DisplayName = "Rock"),
    Water           UMETA(DisplayName = "Water"),
    Mud             UMETA(DisplayName = "Mud"),
    Sand            UMETA(DisplayName = "Sand"),
    Grass           UMETA(DisplayName = "Grass"),
    Snow            UMETA(DisplayName = "Snow")
};

USTRUCT(BlueprintType)
struct FVFX_ImpactEffectData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    class UNiagaraSystem* ParticleEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    class USoundCue* ImpactSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float EffectScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float VolumeMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    bool bSpawnDecal = true;

    FVFX_ImpactEffectData()
    {
        ParticleEffect = nullptr;
        ImpactSound = nullptr;
        EffectScale = 1.0f;
        VolumeMultiplier = 1.0f;
        bSpawnDecal = true;
    }
};

UCLASS(ClassGroup=(VFX), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UVFX_ImpactEffectManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UVFX_ImpactEffectManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Spawn impact effect at location
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnImpactEffect(EVFX_ImpactType ImpactType, EVFX_SurfaceType SurfaceType, FVector Location, FVector Normal = FVector::UpVector);

    // Spawn footstep effect for dinosaurs
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnFootstepEffect(FVector Location, float DinosaurSize = 1.0f, EVFX_SurfaceType SurfaceType = EVFX_SurfaceType::Dirt);

    // Configure effect data for different combinations
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SetEffectData(EVFX_ImpactType ImpactType, EVFX_SurfaceType SurfaceType, const FVFX_ImpactEffectData& EffectData);

protected:
    // Effect data storage
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Effects")
    TMap<EVFX_ImpactType, TMap<EVFX_SurfaceType, FVFX_ImpactEffectData>> ImpactEffects;

    // Default Niagara systems
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default Effects")
    class UNiagaraSystem* DefaultDustEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default Effects")
    class UNiagaraSystem* DefaultWaterSplash;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default Effects")
    class UNiagaraSystem* DefaultRockImpact;

    // Default sounds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default Audio")
    class USoundCue* DefaultFootstepSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default Audio")
    class USoundCue* DefaultImpactSound;

private:
    // Initialize default effect data
    void InitializeDefaultEffects();

    // Get effect data for specific combination
    FVFX_ImpactEffectData GetEffectData(EVFX_ImpactType ImpactType, EVFX_SurfaceType SurfaceType);

    // Detect surface type from hit result
    EVFX_SurfaceType DetectSurfaceType(FVector Location);
};