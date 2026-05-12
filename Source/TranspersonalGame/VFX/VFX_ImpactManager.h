#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "../SharedTypes.h"
#include "VFX_ImpactManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVFX_ImpactData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact VFX")
    TSoftObjectPtr<UNiagaraSystem> ParticleSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact VFX")
    FVector ParticleScale = FVector(1.0f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact VFX")
    float Duration = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact VFX")
    bool bAttachToSurface = false;

    FVFX_ImpactData()
    {
        ParticleScale = FVector(1.0f, 1.0f, 1.0f);
        Duration = 2.0f;
        bAttachToSurface = false;
    }
};

UENUM(BlueprintType)
enum class EVFX_ImpactType : uint8
{
    FootstepDirt        UMETA(DisplayName = "Footstep on Dirt"),
    FootstepRock        UMETA(DisplayName = "Footstep on Rock"),
    FootstepWater       UMETA(DisplayName = "Footstep in Water"),
    WeaponHitFlesh      UMETA(DisplayName = "Weapon Hit Flesh"),
    WeaponHitBone       UMETA(DisplayName = "Weapon Hit Bone"),
    FallImpact          UMETA(DisplayName = "Fall Impact"),
    ObjectBreak         UMETA(DisplayName = "Object Break"),
    BloodSplatter       UMETA(DisplayName = "Blood Splatter")
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AVFX_ImpactManager : public AActor
{
    GENERATED_BODY()

public:
    AVFX_ImpactManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact VFX")
    TMap<EVFX_ImpactType, FVFX_ImpactData> ImpactEffects;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float MaxImpactDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    int32 MaxActiveEffects = 50;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime")
    TArray<class UNiagaraComponent*> ActiveEffects;

public:
    UFUNCTION(BlueprintCallable, Category = "VFX Impact")
    void PlayImpactEffect(EVFX_ImpactType ImpactType, FVector Location, FVector Normal = FVector::UpVector, float Scale = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX Impact")
    void PlayFootstepEffect(EDinosaurSpecies Species, FVector Location, ESurfaceType SurfaceType);

    UFUNCTION(BlueprintCallable, Category = "VFX Impact")
    void PlayCombatImpact(FVector Location, float Damage, bool bIsBloodHit = false);

    UFUNCTION(BlueprintCallable, Category = "VFX Impact")
    void CleanupOldEffects();

    UFUNCTION(BlueprintCallable, Category = "VFX Impact")
    void SetMaxActiveEffects(int32 NewMax);

protected:
    UFUNCTION()
    void InitializeDefaultEffects();

    UFUNCTION()
    UNiagaraComponent* SpawnEffectAtLocation(UNiagaraSystem* System, FVector Location, FRotator Rotation, FVector Scale);

    UFUNCTION()
    void RemoveFinishedEffects();

    UFUNCTION()
    EVFX_ImpactType GetImpactTypeForSurface(ESurfaceType SurfaceType);

private:
    FTimerHandle CleanupTimerHandle;
    float CleanupInterval = 5.0f;
};