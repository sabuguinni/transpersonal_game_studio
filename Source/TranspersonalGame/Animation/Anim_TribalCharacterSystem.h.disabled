#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Character.h"
#include "Anim_TribalCharacterSystem.generated.h"

// Tribal character animation states
UENUM(BlueprintType)
enum class EAnim_TribalState : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Walking         UMETA(DisplayName = "Walking"),
    Running         UMETA(DisplayName = "Running"),
    Crouching       UMETA(DisplayName = "Crouching"),
    Hunting         UMETA(DisplayName = "Hunting"),
    Crafting        UMETA(DisplayName = "Crafting"),
    Gathering       UMETA(DisplayName = "Gathering"),
    Ritual          UMETA(DisplayName = "Ritual"),
    Combat          UMETA(DisplayName = "Combat"),
    Injured         UMETA(DisplayName = "Injured"),
    Sleeping        UMETA(DisplayName = "Sleeping")
};

// Tribal character types with unique animation sets
UENUM(BlueprintType)
enum class EAnim_TribalType : uint8
{
    Elder           UMETA(DisplayName = "Elder"),
    Warrior         UMETA(DisplayName = "Warrior"),
    Scout           UMETA(DisplayName = "Scout"),
    Healer          UMETA(DisplayName = "Healer"),
    Child           UMETA(DisplayName = "Child"),
    Hunter          UMETA(DisplayName = "Hunter"),
    Crafter         UMETA(DisplayName = "Crafter")
};

// Tribal animation data structure
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_TribalAnimationSet
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Animation")
    class UAnimMontage* IdleMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Animation")
    class UAnimMontage* WalkMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Animation")
    class UAnimMontage* RunMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Animation")
    class UAnimMontage* CombatMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Animation")
    class UAnimMontage* CraftingMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Animation")
    class UAnimMontage* RitualMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Animation")
    class UBlendSpace* LocomotionBlendSpace;

    FAnim_TribalAnimationSet()
    {
        IdleMontage = nullptr;
        WalkMontage = nullptr;
        RunMontage = nullptr;
        CombatMontage = nullptr;
        CraftingMontage = nullptr;
        RitualMontage = nullptr;
        LocomotionBlendSpace = nullptr;
    }
};

/**
 * Tribal Character Animation System
 * Manages animation sets for different tribal character types
 * Each tribal type has unique movement patterns and cultural animations
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(Animation))
class TRANSPERSONALGAME_API UAnim_TribalCharacterSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_TribalCharacterSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Tribal character configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Character")
    EAnim_TribalType TribalType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Character")
    EAnim_TribalState CurrentState;

    // Animation sets for each tribal type
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Sets")
    TMap<EAnim_TribalType, FAnim_TribalAnimationSet> TribalAnimationSets;

    // Current animation properties
    UPROPERTY(BlueprintReadOnly, Category = "Animation State")
    float MovementSpeed;

    UPROPERTY(BlueprintReadOnly, Category = "Animation State")
    float MovementDirection;

    UPROPERTY(BlueprintReadOnly, Category = "Animation State")
    bool bIsInCombat;

    UPROPERTY(BlueprintReadOnly, Category = "Animation State")
    bool bIsCrafting;

    UPROPERTY(BlueprintReadOnly, Category = "Animation State")
    bool bIsPerformingRitual;

    // Animation control functions
    UFUNCTION(BlueprintCallable, Category = "Tribal Animation")
    void SetTribalType(EAnim_TribalType NewType);

    UFUNCTION(BlueprintCallable, Category = "Tribal Animation")
    void SetTribalState(EAnim_TribalState NewState);

    UFUNCTION(BlueprintCallable, Category = "Tribal Animation")
    void PlayTribalMontage(EAnim_TribalState StateType);

    UFUNCTION(BlueprintCallable, Category = "Tribal Animation")
    void StopCurrentMontage();

    UFUNCTION(BlueprintCallable, Category = "Tribal Animation")
    FAnim_TribalAnimationSet GetCurrentAnimationSet() const;

    // Cultural animation functions
    UFUNCTION(BlueprintCallable, Category = "Cultural Animation")
    void PlayElderWisdomGesture();

    UFUNCTION(BlueprintCallable, Category = "Cultural Animation")
    void PlayWarriorBattleCry();

    UFUNCTION(BlueprintCallable, Category = "Cultural Animation")
    void PlayHealerRitual();

    UFUNCTION(BlueprintCallable, Category = "Cultural Animation")
    void PlayScoutAlert();

    UFUNCTION(BlueprintCallable, Category = "Cultural Animation")
    void PlayChildCuriosity();

    // Animation blending functions
    UFUNCTION(BlueprintCallable, Category = "Animation Blending")
    void UpdateLocomotionBlending(float Speed, float Direction);

    UFUNCTION(BlueprintCallable, Category = "Animation Blending")
    void BlendToState(EAnim_TribalState TargetState, float BlendTime = 0.25f);

protected:
    // Internal animation management
    void InitializeTribalAnimationSets();
    void UpdateAnimationState();
    void HandleStateTransition(EAnim_TribalState NewState);

    // Reference to the character's anim instance
    UPROPERTY()
    class UAnimInstance* CharacterAnimInstance;

    // Current montage tracking
    UPROPERTY()
    class UAnimMontage* CurrentMontage;

    // Blend timing
    UPROPERTY(EditAnywhere, Category = "Animation Timing")
    float DefaultBlendTime;

    // Cultural animation timing
    UPROPERTY(EditAnywhere, Category = "Cultural Timing")
    float RitualDuration;

    UPROPERTY(EditAnywhere, Category = "Cultural Timing")
    float CraftingCycleDuration;

private:
    // Internal state tracking
    EAnim_TribalState PreviousState;
    float StateTransitionTime;
    bool bIsTransitioning;
};

/**
 * Tribal Character Animation Instance
 * Custom AnimInstance for tribal characters with cultural behaviors
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAnim_TribalCharacterAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UAnim_TribalCharacterAnimInstance();

    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaTimeX) override;

    // Animation properties exposed to Blueprint
    UPROPERTY(BlueprintReadOnly, Category = "Tribal Animation")
    EAnim_TribalType TribalType;

    UPROPERTY(BlueprintReadOnly, Category = "Tribal Animation")
    EAnim_TribalState AnimationState;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float Speed;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float Direction;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsMoving;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    bool bIsInCombat;

    UPROPERTY(BlueprintReadOnly, Category = "Activities")
    bool bIsCrafting;

    UPROPERTY(BlueprintReadOnly, Category = "Activities")
    bool bIsPerformingRitual;

    UPROPERTY(BlueprintReadOnly, Category = "Activities")
    bool bIsGathering;

    // Cultural animation triggers
    UPROPERTY(BlueprintReadOnly, Category = "Cultural")
    bool bShouldPlayWisdomGesture;

    UPROPERTY(BlueprintReadOnly, Category = "Cultural")
    bool bShouldPlayBattleCry;

    UPROPERTY(BlueprintReadOnly, Category = "Cultural")
    bool bShouldPlayHealingRitual;

protected:
    // Reference to tribal character system
    UPROPERTY()
    UAnim_TribalCharacterSystem* TribalSystem;

    // Character reference
    UPROPERTY()
    class ACharacter* OwningCharacter;

private:
    void UpdateTribalAnimationProperties();
    void UpdateMovementProperties();
    void UpdateCulturalProperties();
};