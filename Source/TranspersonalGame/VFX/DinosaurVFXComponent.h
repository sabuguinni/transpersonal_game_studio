#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "VFXManager.h"
#include "DinosaurVFXComponent.generated.h"

UENUM(BlueprintType)
enum class EDinosaurSize : uint8
{
    Tiny,       // < 50cm (Compsognathus)
    Small,      // 50cm - 2m (Velociraptor)
    Medium,     // 2m - 5m (Allosaurus)
    Large,      // 5m - 10m (T-Rex)
    Massive     // > 10m (Brontosaurus)
};

UENUM(BlueprintType)
enum class EDinosaurBehaviorState : uint8
{
    Idle,
    Walking,
    Running,
    Hunting,
    Feeding,
    Sleeping,
    Alert,
    Aggressive,
    Injured,
    Dead
};

USTRUCT(BlueprintType)
struct FDinosaurVFXSet
{
    GENERATED_BODY()

    // Breathing effects
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Breathing")
    UNiagaraSystem* IdleBreathing;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Breathing")
    UNiagaraSystem* HeavyBreathing;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Breathing")
    UNiagaraSystem* ColdBreath; // Para clima frio

    // Footstep effects
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    UNiagaraSystem* FootstepDust;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    UNiagaraSystem* FootstepSplash; // Para água/lama

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    UNiagaraSystem* FootstepLeaves;

    // Combat effects
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    UNiagaraSystem* BloodSplatter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    UNiagaraSystem* ClawScratch;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    UNiagaraSystem* BiteImpact;

    // Behavioral effects
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    UNiagaraSystem* TerritorialRoar; // Partículas de intimidação

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    UNiagaraSystem* FeedingBlood;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    UNiagaraSystem* SleepingZzz; // Efeito sutil de sono

    FDinosaurVFXSet()
    {
        IdleBreathing = nullptr;
        HeavyBreathing = nullptr;
        ColdBreath = nullptr;
        FootstepDust = nullptr;
        FootstepSplash = nullptr;
        FootstepLeaves = nullptr;
        BloodSplatter = nullptr;
        ClawScratch = nullptr;
        BiteImpact = nullptr;
        TerritorialRoar = nullptr;
        FeedingBlood = nullptr;
        SleepingZzz = nullptr;
    }
};

/**
 * Componente especializado para efeitos visuais de dinossauros
 * Gere breathing, footsteps, combat effects e variações procedurais
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UDinosaurVFXComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UDinosaurVFXComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Configuração do dinossauro
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Config")
    EDinosaurSize DinosaurSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Config")
    FDinosaurVFXSet VFXSet;

    // Estado atual
    UPROPERTY(BlueprintReadOnly, Category = "State")
    EDinosaurBehaviorState CurrentState;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    float HealthPercentage;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    float StaminaPercentage;

    // Variação procedural
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variation")
    int32 IndividualSeed; // Seed único para este dinossauro

    UPROPERTY(BlueprintReadOnly, Category = "Variation")
    FLinearColor BreathTint; // Cor da respiração (baseada em genética)

    UPROPERTY(BlueprintReadOnly, Category = "Variation")
    float BreathIntensity; // Intensidade da respiração

    UPROPERTY(BlueprintReadOnly, Category = "Variation")
    float FootstepScale; // Escala das pegadas

    // Componentes ativos
    UPROPERTY()
    UNiagaraComponent* ActiveBreathingEffect;

    UPROPERTY()
    TArray<UNiagaraComponent*> ActiveFootstepEffects;

    // Timers e controle
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
    float BreathingInterval; // Segundos entre respirações

    UPROPERTY()
    float LastBreathTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MaxEffectDistance; // Distância máxima para spawnar efeitos

public:
    // Interface principal
    UFUNCTION(BlueprintCallable, Category = "Dinosaur VFX")
    void SetBehaviorState(EDinosaurBehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur VFX")
    void UpdateHealthStatus(float HealthPercent);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur VFX")
    void TriggerFootstep(FVector Location, FName SurfaceType);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur VFX")
    void TriggerCombatEffect(FName EffectType, FVector Location, FRotator Rotation);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur VFX")
    void SetIndividualVariation(int32 Seed);

    // Breathing system
    UFUNCTION(BlueprintCallable, Category = "Breathing")
    void StartBreathing();

    UFUNCTION(BlueprintCallable, Category = "Breathing")
    void StopBreathing();

    UFUNCTION(BlueprintCallable, Category = "Breathing")
    void UpdateBreathingIntensity(float Intensity);

private:
    void GenerateProceduralVariation();
    void UpdateBreathingEffect();
    bool ShouldSpawnEffect(FVector EffectLocation);
    UNiagaraSystem* GetFootstepEffect(FName SurfaceType);
    void CleanupFinishedEffects();
    
    // Performance optimization
    float GetDistanceToPlayer();
    EVFXPriority GetEffectPriority(FName EffectType);
};