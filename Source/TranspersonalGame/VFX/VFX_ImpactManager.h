#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Engine/World.h"
#include "VFX_ImpactManager.generated.h"

UENUM(BlueprintType)
enum class EVFX_ImpactType : uint8
{
    DinosaurFootstep    UMETA(DisplayName = "Dinosaur Footstep"),
    PlayerFootstep      UMETA(DisplayName = "Player Footstep"),
    RockImpact         UMETA(DisplayName = "Rock Impact"),
    WaterSplash        UMETA(DisplayName = "Water Splash"),
    BloodSpray         UMETA(DisplayName = "Blood Spray"),
    DustCloud          UMETA(DisplayName = "Dust Cloud")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVFX_ImpactData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    EVFX_ImpactType ImpactType = EVFX_ImpactType::DinosaurFootstep;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    TSoftObjectPtr<UNiagaraSystem> NiagaraEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FVector EffectScale = FVector(1.0f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float EffectDuration = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    bool bAttachToGround = true;

    FVFX_ImpactData()
    {
        ImpactType = EVFX_ImpactType::DinosaurFootstep;
        EffectScale = FVector(1.0f, 1.0f, 1.0f);
        EffectDuration = 3.0f;
        bAttachToGround = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AVFX_ImpactManager : public AActor
{
    GENERATED_BODY()

public:
    AVFX_ImpactManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Core VFX functions
    UFUNCTION(BlueprintCallable, Category = "VFX Impact")
    void TriggerImpactEffect(EVFX_ImpactType ImpactType, FVector Location, FRotator Rotation = FRotator::ZeroRotator);

    UFUNCTION(BlueprintCallable, Category = "VFX Impact")
    void TriggerDinosaurFootstep(FVector Location, float DinosaurSize = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX Impact")
    void TriggerWaterSplash(FVector Location, float SplashIntensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX Impact")
    void TriggerDustCloud(FVector Location, float CloudSize = 1.0f);

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    TMap<EVFX_ImpactType, FVFX_ImpactData> ImpactEffects;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    float GlobalVFXScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    bool bEnableVFX = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    int32 MaxActiveEffects = 20;

protected:
    // Internal components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    // Active effect tracking
    UPROPERTY()
    TArray<UNiagaraComponent*> ActiveEffects;

    // Helper functions
    void CleanupFinishedEffects();
    UNiagaraComponent* SpawnNiagaraEffect(UNiagaraSystem* NiagaraSystem, FVector Location, FRotator Rotation, FVector Scale);
    void InitializeDefaultEffects();
};