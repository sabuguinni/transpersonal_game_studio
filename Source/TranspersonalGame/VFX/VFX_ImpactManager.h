#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "VFX_ImpactManager.generated.h"

/**
 * VFX Impact Manager - Gere efeitos visuais de impacto para dinossauros e combate
 * Sistemas: footsteps, blood splatter, debris, dust clouds
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AVFX_ImpactManager : public AActor
{
    GENERATED_BODY()

public:
    AVFX_ImpactManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Componentes principais
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Core")
    UStaticMeshComponent* RootMeshComponent;

    // Sistemas Niagara para diferentes tipos de impacto
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Systems")
    class UNiagaraSystem* FootstepDustSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Systems")
    class UNiagaraSystem* BloodSplatterSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Systems")
    class UNiagaraSystem* DebrisSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Systems")
    class UNiagaraSystem* RockImpactSystem;

    // Configurações de impacto
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact Settings")
    float FootstepIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact Settings")
    float BloodIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact Settings")
    float DebrisSpread;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact Settings")
    bool bEnableFootstepVFX;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact Settings")
    bool bEnableBloodVFX;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact Settings")
    bool bEnableDebrisVFX;

public:
    // Métodos para triggerar efeitos VFX
    UFUNCTION(BlueprintCallable, Category = "VFX Impact")
    void TriggerFootstepVFX(FVector Location, float DinosaurSize);

    UFUNCTION(BlueprintCallable, Category = "VFX Impact")
    void TriggerBloodSplatterVFX(FVector Location, FVector ImpactDirection);

    UFUNCTION(BlueprintCallable, Category = "VFX Impact")
    void TriggerDebrisVFX(FVector Location, FVector ExplosionDirection, float Force);

    UFUNCTION(BlueprintCallable, Category = "VFX Impact")
    void TriggerRockImpactVFX(FVector Location, float ImpactForce);

    // Configuração e gestão
    UFUNCTION(BlueprintCallable, Category = "VFX Management")
    void SetVFXIntensity(float NewIntensity);

    UFUNCTION(BlueprintCallable, Category = "VFX Management")
    void EnableAllVFX(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "VFX Management")
    void CleanupOldVFX();

private:
    // Arrays para gestão de efeitos activos
    UPROPERTY()
    TArray<class UNiagaraComponent*> ActiveFootstepVFX;

    UPROPERTY()
    TArray<class UNiagaraComponent*> ActiveBloodVFX;

    UPROPERTY()
    TArray<class UNiagaraComponent*> ActiveDebrisVFX;

    // Timers para cleanup
    FTimerHandle FootstepCleanupTimer;
    FTimerHandle BloodCleanupTimer;
    FTimerHandle DebrisCleanupTimer;

    // Métodos internos
    void InitializeVFXSystems();
    void SetupCleanupTimers();
    UNiagaraComponent* SpawnVFXAtLocation(UNiagaraSystem* System, FVector Location, FRotator Rotation);
    void CleanupVFXArray(TArray<UNiagaraComponent*>& VFXArray);
};