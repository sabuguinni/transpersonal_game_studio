#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "GameFramework/Pawn.h"
#include "../SharedTypes.h"
#include "DinosaurAnimationController.generated.h"

UENUM(BlueprintType)
enum class EAnim_DinosaurState : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Walking         UMETA(DisplayName = "Walking"),
    Running         UMETA(DisplayName = "Running"),
    Hunting         UMETA(DisplayName = "Hunting"),
    Feeding         UMETA(DisplayName = "Feeding"),
    Sleeping        UMETA(DisplayName = "Sleeping"),
    Alert           UMETA(DisplayName = "Alert"),
    Attacking       UMETA(DisplayName = "Attacking"),
    Fleeing         UMETA(DisplayName = "Fleeing"),
    Roaming         UMETA(DisplayName = "Roaming"),
    Drinking        UMETA(DisplayName = "Drinking"),
    Socializing     UMETA(DisplayName = "Socializing")
};

UENUM(BlueprintType)
enum class EAnim_DinosaurSize : uint8
{
    Small           UMETA(DisplayName = "Small (Compsognathus)"),
    Medium          UMETA(DisplayName = "Medium (Raptor)"),
    Large           UMETA(DisplayName = "Large (T-Rex)"),
    Massive         UMETA(DisplayName = "Massive (Brachiosaurus)")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_DinosaurBehaviorData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float AggressionLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float HungerLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float AlertnessLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float SocialLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    bool bIsPackAnimal;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    bool bIsNearThreat;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    bool bIsNearFood;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    bool bIsNearWater;

    FAnim_DinosaurBehaviorData()
    {
        AggressionLevel = 0.5f;
        HungerLevel = 0.3f;
        AlertnessLevel = 0.2f;
        SocialLevel = 0.4f;
        bIsPackAnimal = false;
        bIsNearThreat = false;
        bIsNearFood = false;
        bIsNearWater = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_DinosaurMovementData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float Speed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float TurnRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    FVector MovementDirection;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    bool bIsMoving;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    bool bIsTurning;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float GroundSlope;

    FAnim_DinosaurMovementData()
    {
        Speed = 0.0f;
        TurnRate = 0.0f;
        MovementDirection = FVector::ZeroVector;
        bIsMoving = false;
        bIsTurning = false;
        GroundSlope = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_DinosaurSpeciesConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    EAnim_DinosaurSize SizeCategory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    float BaseSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    float BaseAggressionLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    bool bIsCarnivore;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    bool bIsHerbivore;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    bool bCanFly;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    bool bCanSwim;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    TMap<EAnim_DinosaurState, class UAnimMontage*> SpeciesMontages;

    FAnim_DinosaurSpeciesConfig()
    {
        SizeCategory = EAnim_DinosaurSize::Medium;
        BaseSpeed = 300.0f;
        BaseAggressionLevel = 0.5f;
        bIsCarnivore = false;
        bIsHerbivore = true;
        bCanFly = false;
        bCanSwim = false;
    }
};

UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UDinosaurAnimationController : public UActorComponent
{
    GENERATED_BODY()

public:
    UDinosaurAnimationController();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Estado actual do dinossauro
    UPROPERTY(BlueprintReadOnly, Category = "Dinosaur State")
    EAnim_DinosaurState CurrentState;

    // Dados de comportamento
    UPROPERTY(BlueprintReadOnly, Category = "Dinosaur Data")
    FAnim_DinosaurBehaviorData BehaviorData;

    // Dados de movimento
    UPROPERTY(BlueprintReadOnly, Category = "Dinosaur Data")
    FAnim_DinosaurMovementData MovementData;

    // Configuração da espécie
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Config")
    FAnim_DinosaurSpeciesConfig SpeciesConfig;

    // Montagens de animação por estado
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    TMap<EAnim_DinosaurState, class UAnimMontage*> StateMontages;

    // Blend space para movimento
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    class UBlendSpace* MovementBlendSpace;

    // Funções de controlo de estado
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Control")
    void SetDinosaurState(EAnim_DinosaurState NewState);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Control")
    void UpdateBehaviorData(float Aggression, float Hunger, float Alertness, float Social);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Control")
    void UpdateMovementData(float Speed, FVector Direction, float TurnRate);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Control")
    void SetEnvironmentalFlags(bool bNearThreat, bool bNearFood, bool bNearWater);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Control")
    void PlayStateAnimation(EAnim_DinosaurState State);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Control")
    void ConfigureSpecies(EAnim_DinosaurSize Size, bool bCarnivore, bool bHerbivore, float BaseSpeed);

    // Funções de consulta
    UFUNCTION(BlueprintPure, Category = "Dinosaur Query")
    bool CanTransitionToState(EAnim_DinosaurState TargetState) const;

    UFUNCTION(BlueprintPure, Category = "Dinosaur Query")
    float GetCurrentAnimationPlayRate() const;

    UFUNCTION(BlueprintPure, Category = "Dinosaur Query")
    bool IsInCombatState() const;

    UFUNCTION(BlueprintPure, Category = "Dinosaur Query")
    bool IsInPassiveState() const;

    // Funções de IA
    UFUNCTION(BlueprintCallable, Category = "Dinosaur AI")
    void ProcessAIBehavior(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur AI")
    EAnim_DinosaurState DetermineOptimalState() const;

private:
    // Referência para o componente de mesh
    UPROPERTY()
    class USkeletalMeshComponent* DinosaurMeshComponent;

    // Timer para mudanças de estado
    float StateChangeTimer;
    float StateChangeCooldown;

    // Sistema de transições
    bool bIsTransitioning;
    EAnim_DinosaurState PendingState;
    float TransitionDuration;

    // Funções internas
    void InitializeSpeciesDefaults();
    void UpdateAnimationInstance();
    void ProcessStateTransition(float DeltaTime);
    bool ValidateStateTransition(EAnim_DinosaurState From, EAnim_DinosaurState To) const;
    void ApplySpeciesModifiers();
    void HandleEnvironmentalInfluences();
};