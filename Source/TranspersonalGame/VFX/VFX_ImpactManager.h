#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Engine/World.h"
#include "VFX_ImpactManager.generated.h"

UENUM(BlueprintType)
enum class EVFX_ImpactType : uint8
{
    DinosaurFootstep    UMETA(DisplayName = "Dinosaur Footstep"),
    WeaponHit          UMETA(DisplayName = "Weapon Hit"),
    RockImpact         UMETA(DisplayName = "Rock Impact"),
    BloodSplatter      UMETA(DisplayName = "Blood Splatter"),
    DustCloud          UMETA(DisplayName = "Dust Cloud")
};

USTRUCT(BlueprintType)
struct FVFX_ImpactData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    EVFX_ImpactType ImpactType = EVFX_ImpactType::DinosaurFootstep;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    TSoftObjectPtr<UNiagaraSystem> NiagaraSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FVector Scale = FVector(1.0f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float Duration = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    bool bAttachToActor = false;

    FVFX_ImpactData()
    {
        ImpactType = EVFX_ImpactType::DinosaurFootstep;
        Scale = FVector(1.0f, 1.0f, 1.0f);
        Duration = 2.0f;
        bAttachToActor = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UVFX_ImpactManager : public AActor
{
    GENERATED_BODY()

public:
    UVFX_ImpactManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Impact")
    TArray<FVFX_ImpactData> ImpactEffects;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Impact")
    float MaxActiveEffects = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Impact")
    bool bDebugMode = false;

private:
    UPROPERTY()
    TArray<UNiagaraComponent*> ActiveEffects;

public:
    UFUNCTION(BlueprintCallable, Category = "VFX Impact")
    void SpawnImpactEffect(EVFX_ImpactType ImpactType, FVector Location, FRotator Rotation = FRotator::ZeroRotator, AActor* AttachActor = nullptr);

    UFUNCTION(BlueprintCallable, Category = "VFX Impact")
    void SpawnDinosaurFootstep(FVector Location, float DinosaurSize = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX Impact")
    void SpawnBloodEffect(FVector Location, FVector Direction);

    UFUNCTION(BlueprintCallable, Category = "VFX Impact")
    void SpawnDustCloud(FVector Location, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX Impact")
    void CleanupExpiredEffects();

    UFUNCTION(BlueprintCallable, Category = "VFX Impact")
    int32 GetActiveEffectCount() const { return ActiveEffects.Num(); }

protected:
    UFUNCTION()
    void OnEffectFinished(UNiagaraComponent* FinishedComponent);

    void RegisterImpactEffect(UNiagaraComponent* Effect);
    void UnregisterImpactEffect(UNiagaraComponent* Effect);
};