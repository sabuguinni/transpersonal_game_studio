#pragma once

#include "CoreMinimal.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "VFX_NiagaraLibrary.generated.h"

TRANSPERSONALGAME_API DECLARE_LOG_CATEGORY_EXTERN(LogVFXNiagara, Log, All);

/**
 * VFX Library for prehistoric survival game effects
 * Manages Niagara particle systems for realistic environmental effects
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UVFX_NiagaraLibrary : public UObject
{
    GENERATED_BODY()

public:
    UVFX_NiagaraLibrary();

    // Environment VFX
    UFUNCTION(BlueprintCallable, Category = "VFX|Environment")
    static class UNiagaraSystem* GetCampfireVFX();

    UFUNCTION(BlueprintCallable, Category = "VFX|Environment")
    static class UNiagaraSystem* GetRainVFX();

    UFUNCTION(BlueprintCallable, Category = "VFX|Environment")
    static class UNiagaraSystem* GetFogVFX();

    UFUNCTION(BlueprintCallable, Category = "VFX|Environment")
    static class UNiagaraSystem* GetWindVFX();

    // Dinosaur VFX
    UFUNCTION(BlueprintCallable, Category = "VFX|Dinosaur")
    static class UNiagaraSystem* GetFootstepDustVFX();

    UFUNCTION(BlueprintCallable, Category = "VFX|Dinosaur")
    static class UNiagaraSystem* GetBreathVaporVFX();

    UFUNCTION(BlueprintCallable, Category = "VFX|Dinosaur")
    static class UNiagaraSystem* GetBloodImpactVFX();

    UFUNCTION(BlueprintCallable, Category = "VFX|Dinosaur")
    static class UNiagaraSystem* GetRoarDistortionVFX();

    // Combat VFX
    UFUNCTION(BlueprintCallable, Category = "VFX|Combat")
    static class UNiagaraSystem* GetSpearImpactVFX();

    UFUNCTION(BlueprintCallable, Category = "VFX|Combat")
    static class UNiagaraSystem* GetStoneImpactVFX();

    UFUNCTION(BlueprintCallable, Category = "VFX|Combat")
    static class UNiagaraSystem* GetWoundBleedingVFX();

    // Crafting VFX
    UFUNCTION(BlueprintCallable, Category = "VFX|Crafting")
    static class UNiagaraSystem* GetSparksVFX();

    UFUNCTION(BlueprintCallable, Category = "VFX|Crafting")
    static class UNiagaraSystem* GetCookingSmokeVFX();

    // Utility functions
    UFUNCTION(BlueprintCallable, Category = "VFX|Utility")
    static class UNiagaraComponent* SpawnVFXAtLocation(
        UWorld* World,
        class UNiagaraSystem* VFXSystem,
        const FVector& Location,
        const FRotator& Rotation = FRotator::ZeroRotator,
        const FVector& Scale = FVector::OneVector
    );

    UFUNCTION(BlueprintCallable, Category = "VFX|Utility")
    static void AttachVFXToActor(
        AActor* TargetActor,
        class UNiagaraSystem* VFXSystem,
        const FName& SocketName = NAME_None,
        const FVector& LocationOffset = FVector::ZeroVector
    );

protected:
    // VFX asset references
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX Assets")
    static TSoftObjectPtr<class UNiagaraSystem> CampfireVFX;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX Assets")
    static TSoftObjectPtr<class UNiagaraSystem> RainVFX;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX Assets")
    static TSoftObjectPtr<class UNiagaraSystem> FootstepDustVFX;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX Assets")
    static TSoftObjectPtr<class UNiagaraSystem> BloodImpactVFX;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX Assets")
    static TSoftObjectPtr<class UNiagaraSystem> SparksVFX;
};