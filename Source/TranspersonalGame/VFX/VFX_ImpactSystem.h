#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Engine/World.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "VFX_ImpactSystem.generated.h"

UENUM(BlueprintType)
enum class EVFX_ImpactType : uint8
{
    DinosaurFootstep    UMETA(DisplayName = "Dinosaur Footstep"),
    WeaponHit          UMETA(DisplayName = "Weapon Hit"),
    RockFall           UMETA(DisplayName = "Rock Fall"),
    TreeFall           UMETA(DisplayName = "Tree Fall"),
    BodySlam           UMETA(DisplayName = "Body Slam")
};

UENUM(BlueprintType)
enum class EVFX_SurfaceType : uint8
{
    Dirt               UMETA(DisplayName = "Dirt"),
    Rock               UMETA(DisplayName = "Rock"),
    Sand               UMETA(DisplayName = "Sand"),
    Mud                UMETA(DisplayName = "Mud"),
    Grass              UMETA(DisplayName = "Grass"),
    Water              UMETA(DisplayName = "Water")
};

USTRUCT(BlueprintType)
struct FVFX_ImpactParams
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact VFX")
    EVFX_ImpactType ImpactType = EVFX_ImpactType::DinosaurFootstep;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact VFX")
    EVFX_SurfaceType SurfaceType = EVFX_SurfaceType::Dirt;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact VFX")
    float ImpactForce = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact VFX")
    float DustAmount = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact VFX")
    float DebrisCount = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact VFX")
    FVector ImpactLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact VFX")
    FVector ImpactNormal = FVector::UpVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact VFX")
    bool bCreateDecal = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact VFX")
    float DecalSize = 100.0f;

    FVFX_ImpactParams()
    {
        ImpactType = EVFX_ImpactType::DinosaurFootstep;
        SurfaceType = EVFX_SurfaceType::Dirt;
        ImpactForce = 1.0f;
        DustAmount = 1.0f;
        DebrisCount = 20.0f;
        ImpactLocation = FVector::ZeroVector;
        ImpactNormal = FVector::UpVector;
        bCreateDecal = true;
        DecalSize = 100.0f;
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

public:
    // Impact effect functions
    UFUNCTION(BlueprintCallable, Category = "VFX Impact")
    void TriggerImpact(const FVFX_ImpactParams& Params);

    UFUNCTION(BlueprintCallable, Category = "VFX Impact")
    void TriggerDinosaurFootstep(const FVector& Location, float DinosaurSize = 1.0f, EVFX_SurfaceType Surface = EVFX_SurfaceType::Dirt);

    UFUNCTION(BlueprintCallable, Category = "VFX Impact")
    void TriggerWeaponImpact(const FVector& Location, const FVector& Normal, float Force = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX Impact")
    static void SpawnImpactAtLocation(UWorld* World, const FVFX_ImpactParams& Params);

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Impact Assets")
    TMap<EVFX_SurfaceType, TSoftObjectPtr<UNiagaraSystem>> DustEffects;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Impact Assets")
    TMap<EVFX_SurfaceType, TSoftObjectPtr<UNiagaraSystem>> DebrisEffects;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Impact Assets")
    TMap<EVFX_ImpactType, TSoftObjectPtr<UNiagaraSystem>> ImpactEffects;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Impact Assets")
    TMap<EVFX_SurfaceType, TSoftObjectPtr<USoundCue>> ImpactSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Impact Assets")
    TMap<EVFX_SurfaceType, TSoftObjectPtr<UMaterialInterface>> DecalMaterials;

protected:
    // Internal functions
    void SpawnDustEffect(const FVFX_ImpactParams& Params);
    void SpawnDebrisEffect(const FVFX_ImpactParams& Params);
    void SpawnImpactDecal(const FVFX_ImpactParams& Params);
    void PlayImpactSound(const FVFX_ImpactParams& Params);
    
    UNiagaraSystem* GetEffectForSurface(const TMap<EVFX_SurfaceType, TSoftObjectPtr<UNiagaraSystem>>& EffectMap, EVFX_SurfaceType Surface);
    void LoadImpactAssets();
};

#include "VFX_ImpactSystem.generated.h"