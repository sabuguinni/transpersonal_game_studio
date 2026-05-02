#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "VFX_ImpactManager.generated.h"

UENUM(BlueprintType)
enum class EVFX_ImpactType : uint8
{
    DinosaurFootstep    UMETA(DisplayName = "Dinosaur Footstep"),
    PlayerDamage        UMETA(DisplayName = "Player Damage"),
    GroundImpact        UMETA(DisplayName = "Ground Impact"),
    BloodSplatter       UMETA(DisplayName = "Blood Splatter"),
    DustExplosion       UMETA(DisplayName = "Dust Explosion")
};

USTRUCT(BlueprintType)
struct FVFX_ImpactData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Impact")
    EVFX_ImpactType ImpactType = EVFX_ImpactType::GroundImpact;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Impact")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Impact")
    FVector Scale = FVector(1.0f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Impact")
    float Duration = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Impact")
    float Intensity = 1.0f;

    FVFX_ImpactData()
    {
        ImpactType = EVFX_ImpactType::GroundImpact;
        Location = FVector::ZeroVector;
        Scale = FVector(1.0f, 1.0f, 1.0f);
        Duration = 2.0f;
        Intensity = 1.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UVFX_ImpactManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UVFX_ImpactManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Trigger impact effect
    UFUNCTION(BlueprintCallable, Category = "VFX Impact")
    void TriggerImpact(const FVFX_ImpactData& ImpactData);

    // Trigger dinosaur footstep
    UFUNCTION(BlueprintCallable, Category = "VFX Impact")
    void TriggerDinosaurFootstep(FVector Location, float DinosaurSize = 1.0f);

    // Trigger player damage effect
    UFUNCTION(BlueprintCallable, Category = "VFX Impact")
    void TriggerPlayerDamage(float DamageAmount);

    // Screen shake effect
    UFUNCTION(BlueprintCallable, Category = "VFX Impact")
    void TriggerScreenShake(float Intensity, float Duration);

    // Cleanup expired effects
    UFUNCTION(BlueprintCallable, Category = "VFX Impact")
    void CleanupExpiredEffects();

protected:
    // Niagara systems for different effects
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Systems")
    class UNiagaraSystem* DustParticleSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Systems")
    class UNiagaraSystem* BloodParticleSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Systems")
    class UNiagaraSystem* DebrisParticleSystem;

    // Active particle components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Runtime")
    TArray<class UNiagaraComponent*> ActiveParticleComponents;

    // Effect timers
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Runtime")
    TArray<FTimerHandle> EffectTimers;

    // Screen shake parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Effects")
    float MaxScreenShakeIntensity = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Effects")
    float MaxScreenShakeDuration = 3.0f;

    // Damage flash parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Effects")
    float DamageFlashDuration = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Effects")
    FLinearColor DamageFlashColor = FLinearColor::Red;

private:
    // Create particle component for effect
    UNiagaraComponent* CreateParticleEffect(UNiagaraSystem* System, const FVector& Location, const FVector& Scale);

    // Remove particle component after duration
    void RemoveParticleEffect(UNiagaraComponent* Component);

    // Timer callback for effect cleanup
    UFUNCTION()
    void OnEffectExpired();
};