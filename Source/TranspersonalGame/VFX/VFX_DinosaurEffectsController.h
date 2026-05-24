#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/StaticMeshComponent.h"
#include "VFX_DinosaurEffectsController.generated.h"

UENUM(BlueprintType)
enum class EVFX_DinosaurType : uint8
{
    TRex           UMETA(DisplayName = "T-Rex"),
    Raptor         UMETA(DisplayName = "Raptor"),
    Brachiosaurus  UMETA(DisplayName = "Brachiosaurus"),
    Generic        UMETA(DisplayName = "Generic Dinosaur")
};

UENUM(BlueprintType)
enum class EVFX_EffectType : uint8
{
    Footstep       UMETA(DisplayName = "Footstep Dust"),
    Breathing      UMETA(DisplayName = "Breathing Steam"),
    Movement       UMETA(DisplayName = "Movement Particles"),
    Impact         UMETA(DisplayName = "Impact Effects"),
    Ambient        UMETA(DisplayName = "Ambient Atmosphere")
};

USTRUCT(BlueprintType)
struct FVFX_DinosaurEffectData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    EVFX_DinosaurType DinosaurType = EVFX_DinosaurType::Generic;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    EVFX_EffectType EffectType = EVFX_EffectType::Footstep;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float EffectIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float EffectDuration = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FVector EffectScale = FVector(1.0f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    bool bAutoTrigger = true;

    FVFX_DinosaurEffectData()
    {
        DinosaurType = EVFX_DinosaurType::Generic;
        EffectType = EVFX_EffectType::Footstep;
        EffectIntensity = 1.0f;
        EffectDuration = 2.0f;
        EffectScale = FVector(1.0f, 1.0f, 1.0f);
        bAutoTrigger = true;
    }
};

/**
 * VFX Controller para efeitos visuais específicos de dinossauros
 * Gere efeitos de pegadas, respiração, movimento e impacto
 */
UCLASS(ClassGroup=(TranspersonalGame), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UVFX_DinosaurEffectsController : public UActorComponent
{
    GENERATED_BODY()

public:
    UVFX_DinosaurEffectsController();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // === CONFIGURAÇÃO DE EFEITOS ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    EVFX_DinosaurType DinosaurType = EVFX_DinosaurType::Generic;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    float GlobalEffectIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    bool bEnableFootstepEffects = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    bool bEnableBreathingEffects = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    bool bEnableMovementEffects = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    bool bEnableImpactEffects = true;

    // === SISTEMAS NIAGARA ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Niagara Systems")
    class UNiagaraSystem* FootstepDustSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Niagara Systems")
    class UNiagaraSystem* BreathingSteamSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Niagara Systems")
    class UNiagaraSystem* MovementParticleSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Niagara Systems")
    class UNiagaraSystem* ImpactEffectSystem;

    // === COMPONENTES DE EFEITO ===
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Components")
    class UNiagaraComponent* FootstepComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Components")
    class UNiagaraComponent* BreathingComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Components")
    class UNiagaraComponent* MovementComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Components")
    class UNiagaraComponent* ImpactComponent;

    // === MÉTODOS PÚBLICOS ===
    
    UFUNCTION(BlueprintCallable, Category = "VFX Control")
    void TriggerFootstepEffect(const FVector& FootLocation, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX Control")
    void TriggerBreathingEffect(const FVector& MouthLocation, float Temperature = 20.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX Control")
    void TriggerMovementEffect(const FVector& MovementDirection, float Speed = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX Control")
    void TriggerImpactEffect(const FVector& ImpactLocation, const FVector& ImpactNormal, float Force = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX Control")
    void SetEffectIntensity(EVFX_EffectType EffectType, float NewIntensity);

    UFUNCTION(BlueprintCallable, Category = "VFX Control")
    void EnableEffect(EVFX_EffectType EffectType, bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "VFX Control")
    void StopAllEffects();

    UFUNCTION(BlueprintCallable, Category = "VFX Control")
    void UpdateDinosaurType(EVFX_DinosaurType NewType);

    // === CONFIGURAÇÃO AUTOMÁTICA ===
    
    UFUNCTION(BlueprintCallable, Category = "VFX Setup")
    void InitializeEffectsForDinosaur();

    UFUNCTION(BlueprintCallable, Category = "VFX Setup")
    void LoadDefaultNiagaraSystems();

    // === INFORMAÇÕES DE ESTADO ===
    
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "VFX Info")
    bool IsEffectActive(EVFX_EffectType EffectType) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "VFX Info")
    float GetEffectIntensity(EVFX_EffectType EffectType) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "VFX Info")
    int32 GetActiveEffectCount() const;

private:
    // === MÉTODOS INTERNOS ===
    
    void SetupNiagaraComponents();
    void ConfigureEffectForDinosaurType();
    void UpdateEffectParameters();
    
    // === DADOS INTERNOS ===
    
    TMap<EVFX_EffectType, float> EffectIntensities;
    TMap<EVFX_EffectType, bool> EffectStates;
    
    float LastFootstepTime = 0.0f;
    float LastBreathingTime = 0.0f;
    float FootstepInterval = 0.5f;
    float BreathingInterval = 2.0f;
    
    bool bIsInitialized = false;
};