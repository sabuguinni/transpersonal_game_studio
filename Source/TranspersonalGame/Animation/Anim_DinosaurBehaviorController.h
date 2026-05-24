#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Anim_DinosaurBehaviorController.generated.h"

UENUM(BlueprintType)
enum class EAnim_DinosaurBehaviorState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walking     UMETA(DisplayName = "Walking"),
    Running     UMETA(DisplayName = "Running"),
    Hunting     UMETA(DisplayName = "Hunting"),
    Feeding     UMETA(DisplayName = "Feeding"),
    Sleeping    UMETA(DisplayName = "Sleeping"),
    Alert       UMETA(DisplayName = "Alert"),
    Aggressive  UMETA(DisplayName = "Aggressive"),
    Wounded     UMETA(DisplayName = "Wounded"),
    Dead        UMETA(DisplayName = "Dead")
};

UENUM(BlueprintType)
enum class EAnim_DinosaurSpecies : uint8
{
    TRex        UMETA(DisplayName = "T-Rex"),
    Raptor      UMETA(DisplayName = "Raptor"),
    Triceratops UMETA(DisplayName = "Triceratops"),
    Brachiosaurus UMETA(DisplayName = "Brachiosaurus"),
    Stegosaurus UMETA(DisplayName = "Stegosaurus"),
    Pteranodon  UMETA(DisplayName = "Pteranodon"),
    Compsognathus UMETA(DisplayName = "Compsognathus"),
    Ankylosaurus UMETA(DisplayName = "Ankylosaurus")
};

USTRUCT(BlueprintType)
struct FAnim_DinosaurAnimationSet
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    class UAnimMontage* IdleMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    class UAnimMontage* WalkMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    class UAnimMontage* RunMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    class UAnimMontage* AttackMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    class UAnimMontage* FeedingMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    class UAnimMontage* SleepMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    class UAnimMontage* DeathMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    class UBlendSpace* LocomotionBlendSpace;

    FAnim_DinosaurAnimationSet()
    {
        IdleMontage = nullptr;
        WalkMontage = nullptr;
        RunMontage = nullptr;
        AttackMontage = nullptr;
        FeedingMontage = nullptr;
        SleepMontage = nullptr;
        DeathMontage = nullptr;
        LocomotionBlendSpace = nullptr;
    }
};

USTRUCT(BlueprintType)
struct FAnim_DinosaurBehaviorData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    EAnim_DinosaurBehaviorState CurrentState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    EAnim_DinosaurSpecies Species;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float MovementSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float AggressionLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float HealthPercentage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float HungerLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    bool bIsInCombat;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    bool bIsAlerted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    AActor* TargetActor;

    FAnim_DinosaurBehaviorData()
    {
        CurrentState = EAnim_DinosaurBehaviorState::Idle;
        Species = EAnim_DinosaurSpecies::TRex;
        MovementSpeed = 0.0f;
        AggressionLevel = 0.5f;
        HealthPercentage = 1.0f;
        HungerLevel = 0.5f;
        bIsInCombat = false;
        bIsAlerted = false;
        TargetActor = nullptr;
    }
};

/**
 * Anim_DinosaurBehaviorController
 * 
 * Controlador avançado de comportamento e animação para dinossauros.
 * Gerencia estados comportamentais, transições de animação e adaptação
 * baseada em espécie e situação.
 */
UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_DinosaurBehaviorController : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_DinosaurBehaviorController();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // === CONFIGURAÇÃO ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    EAnim_DinosaurSpecies DinosaurSpecies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    FAnim_DinosaurAnimationSet AnimationSet;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    TMap<EAnim_DinosaurSpecies, FAnim_DinosaurAnimationSet> SpeciesAnimationSets;

    // === ESTADO ATUAL ===
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    FAnim_DinosaurBehaviorData BehaviorData;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    class UAnimInstance* CurrentAnimInstance;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    class USkeletalMeshComponent* SkeletalMeshComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    class ACharacter* OwnerCharacter;

    // === PARÂMETROS DE COMPORTAMENTO ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Parameters")
    float StateTransitionCooldown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Parameters")
    float AggressionThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Parameters")
    float AlertRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Parameters")
    float AttackRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Parameters")
    float MovementSpeedMultiplier;

    // === MÉTODOS PÚBLICOS ===

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Animation")
    void SetBehaviorState(EAnim_DinosaurBehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Animation")
    void SetDinosaurSpecies(EAnim_DinosaurSpecies NewSpecies);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Animation")
    void UpdateMovementSpeed(float Speed);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Animation")
    void SetTarget(AActor* NewTarget);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Animation")
    void TriggerAttackAnimation();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Animation")
    void TriggerFeedingAnimation();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Animation")
    void TriggerDeathAnimation();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Animation")
    bool IsInState(EAnim_DinosaurBehaviorState State) const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Animation")
    float GetCurrentAnimationLength() const;

    UFUNCTION(BlueprintPure, Category = "Dinosaur Animation")
    EAnim_DinosaurBehaviorState GetCurrentBehaviorState() const;

    UFUNCTION(BlueprintPure, Category = "Dinosaur Animation")
    EAnim_DinosaurSpecies GetDinosaurSpecies() const;

    UFUNCTION(BlueprintPure, Category = "Dinosaur Animation")
    bool CanTransitionToState(EAnim_DinosaurBehaviorState TargetState) const;

protected:
    // === MÉTODOS INTERNOS ===

    void InitializeComponent();
    void UpdateBehaviorLogic(float DeltaTime);
    void ProcessStateTransitions();
    void UpdateAnimationParameters();
    void HandleSpeciesSpecificBehavior();
    
    void PlayMontageForState(EAnim_DinosaurBehaviorState State);
    UAnimMontage* GetMontageForState(EAnim_DinosaurBehaviorState State) const;
    
    bool ValidateStateTransition(EAnim_DinosaurBehaviorState FromState, EAnim_DinosaurBehaviorState ToState) const;
    void OnStateChanged(EAnim_DinosaurBehaviorState OldState, EAnim_DinosaurBehaviorState NewState);

private:
    // === VARIÁVEIS INTERNAS ===
    
    float LastStateChangeTime;
    EAnim_DinosaurBehaviorState PreviousState;
    bool bIsInitialized;
    
    // Cache de componentes
    class UCharacterMovementComponent* MovementComponent;
    
    // Timers para comportamentos específicos
    float FeedingTimer;
    float AlertTimer;
    float CombatTimer;
};