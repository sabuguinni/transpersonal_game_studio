#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "Animation/BlendSpace1D.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "SharedTypes.h"
#include "Anim_PrehistoricAnimationManager.generated.h"

UENUM(BlueprintType)
enum class EAnim_PrehistoricMovementState : uint8
{
    Idle UMETA(DisplayName = "Idle"),
    Walking UMETA(DisplayName = "Walking"),
    Running UMETA(DisplayName = "Running"),
    Sprinting UMETA(DisplayName = "Sprinting"),
    Crouching UMETA(DisplayName = "Crouching"),
    Crawling UMETA(DisplayName = "Crawling"),
    Jumping UMETA(DisplayName = "Jumping"),
    Falling UMETA(DisplayName = "Falling"),
    Landing UMETA(DisplayName = "Landing"),
    Swimming UMETA(DisplayName = "Swimming"),
    Climbing UMETA(DisplayName = "Climbing")
};

UENUM(BlueprintType)
enum class EAnim_PrehistoricActionState : uint8
{
    None UMETA(DisplayName = "None"),
    Gathering UMETA(DisplayName = "Gathering"),
    Crafting UMETA(DisplayName = "Crafting"),
    Hunting UMETA(DisplayName = "Hunting"),
    Fighting UMETA(DisplayName = "Fighting"),
    Eating UMETA(DisplayName = "Eating"),
    Drinking UMETA(DisplayName = "Drinking"),
    Sleeping UMETA(DisplayName = "Sleeping"),
    Injured UMETA(DisplayName = "Injured"),
    Dead UMETA(DisplayName = "Dead")
};

USTRUCT(BlueprintType)
struct FAnim_PrehistoricAnimationData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float Speed = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float Direction = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    bool bIsInAir = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    bool bIsCrouching = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    EAnim_PrehistoricMovementState MovementState = EAnim_PrehistoricMovementState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action")
    EAnim_PrehistoricActionState ActionState = EAnim_PrehistoricActionState::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    bool bIsInCombat = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float HealthPercent = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float StaminaPercent = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float FearLevel = 0.0f;

    FAnim_PrehistoricAnimationData()
    {
        Speed = 0.0f;
        Direction = 0.0f;
        bIsInAir = false;
        bIsCrouching = false;
        MovementState = EAnim_PrehistoricMovementState::Idle;
        ActionState = EAnim_PrehistoricActionState::None;
        bIsInCombat = false;
        HealthPercent = 1.0f;
        StaminaPercent = 1.0f;
        FearLevel = 0.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_PrehistoricAnimationManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_PrehistoricAnimationManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Animation Data
    UPROPERTY(BlueprintReadOnly, Category = "Animation Data")
    FAnim_PrehistoricAnimationData AnimationData;

    // Character References
    UPROPERTY(BlueprintReadOnly, Category = "Character")
    ACharacter* OwnerCharacter;

    UPROPERTY(BlueprintReadOnly, Category = "Character")
    USkeletalMeshComponent* CharacterMesh;

    UPROPERTY(BlueprintReadOnly, Category = "Character")
    UCharacterMovementComponent* MovementComponent;

    // Animation Assets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    UBlendSpace* LocomotionBlendSpace;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    UBlendSpace1D* IdleBlendSpace;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Montages")
    UAnimMontage* JumpMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Montages")
    UAnimMontage* LandMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Montages")
    UAnimMontage* CrouchMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Montages")
    UAnimMontage* GatheringMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Montages")
    UAnimMontage* CraftingMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Montages")
    UAnimMontage* AttackMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Montages")
    UAnimMontage* DeathMontage;

    // Core Functions
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateAnimationData(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetMovementState(EAnim_PrehistoricMovementState NewState);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetActionState(EAnim_PrehistoricActionState NewState);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void PlayMontage(UAnimMontage* Montage, float PlayRate = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void StopMontage(UAnimMontage* Montage);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    bool IsMontagePlayingAny() const;

    // Movement State Functions
    UFUNCTION(BlueprintCallable, Category = "Movement")
    void StartJump();

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void StartLanding();

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void StartCrouch();

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void StopCrouch();

    // Action Functions
    UFUNCTION(BlueprintCallable, Category = "Actions")
    void StartGathering();

    UFUNCTION(BlueprintCallable, Category = "Actions")
    void StartCrafting();

    UFUNCTION(BlueprintCallable, Category = "Actions")
    void StartAttack();

    UFUNCTION(BlueprintCallable, Category = "Actions")
    void StartDeath();

    UFUNCTION(BlueprintCallable, Category = "Actions")
    void StopCurrentAction();

    // Utility Functions
    UFUNCTION(BlueprintCallable, Category = "Animation")
    float CalculateDirection() const;

    UFUNCTION(BlueprintCallable, Category = "Animation")
    EAnim_PrehistoricMovementState DetermineMovementState() const;

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateSurvivalStats();

private:
    // Internal state tracking
    float LastUpdateTime;
    FVector LastVelocity;
    bool bWasInAir;
    float LandingTimer;
    float ActionTimer;

    // Helper functions
    void InitializeCharacterReferences();
    void UpdateMovementData(float DeltaTime);
    void UpdateActionData(float DeltaTime);
    void HandleStateTransitions();
    bool IsValidForAnimation() const;
};