#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "PlayerAnimationController.generated.h"

UENUM(BlueprintType)
enum class EPlayerMovementState : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Walking         UMETA(DisplayName = "Walking"),
    Running         UMETA(DisplayName = "Running"), 
    Crouching       UMETA(DisplayName = "Crouching"),
    Sneaking        UMETA(DisplayName = "Sneaking"),
    Climbing        UMETA(DisplayName = "Climbing"),
    Falling         UMETA(DisplayName = "Falling"),
    Landing         UMETA(DisplayName = "Landing"),
    Interacting     UMETA(DisplayName = "Interacting"),
    Hiding          UMETA(DisplayName = "Hiding"),
    Observing       UMETA(DisplayName = "Observing"),
    Startled        UMETA(DisplayName = "Startled")
};

UENUM(BlueprintType)
enum class EPlayerEmotionalState : uint8
{
    Calm            UMETA(DisplayName = "Calm"),
    Cautious        UMETA(DisplayName = "Cautious"),
    Afraid          UMETA(DisplayName = "Afraid"),
    Terrified       UMETA(DisplayName = "Terrified"),
    Curious         UMETA(DisplayName = "Curious"),
    Focused         UMETA(DisplayName = "Focused"),
    Exhausted       UMETA(DisplayName = "Exhausted"),
    Injured         UMETA(DisplayName = "Injured")
};

/**
 * Specialized Animation Instance for the Player Character
 * Handles Motion Matching, IK, and paleontologist-specific behaviors
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPlayerAnimationController : public UAnimInstance
{
    GENERATED_BODY()

public:
    UPlayerAnimationController();

protected:
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaTimeX) override;

public:
    // Movement State
    UPROPERTY(BlueprintReadOnly, Category = "Player Animation")
    EPlayerMovementState CurrentMovementState;

    UPROPERTY(BlueprintReadOnly, Category = "Player Animation")
    EPlayerEmotionalState CurrentEmotionalState;

    // Motion Matching Variables
    UPROPERTY(BlueprintReadOnly, Category = "Motion Matching")
    float Speed;

    UPROPERTY(BlueprintReadOnly, Category = "Motion Matching")
    float Direction;

    UPROPERTY(BlueprintReadOnly, Category = "Motion Matching")
    FVector Velocity;

    UPROPERTY(BlueprintReadOnly, Category = "Motion Matching")
    bool bIsMoving;

    UPROPERTY(BlueprintReadOnly, Category = "Motion Matching")
    bool bIsFalling;

    UPROPERTY(BlueprintReadOnly, Category = "Motion Matching")
    bool bIsGrounded;

    // IK Variables
    UPROPERTY(BlueprintReadOnly, Category = "IK System")
    float LeftFootIKOffset;

    UPROPERTY(BlueprintReadOnly, Category = "IK System")
    float RightFootIKOffset;

    UPROPERTY(BlueprintReadOnly, Category = "IK System")
    FRotator LeftFootIKRotation;

    UPROPERTY(BlueprintReadOnly, Category = "IK System")
    FRotator RightFootIKRotation;

    UPROPERTY(BlueprintReadOnly, Category = "IK System")
    float HipOffset;

    // Paleontologist-Specific Behaviors
    UPROPERTY(BlueprintReadOnly, Category = "Paleontologist Behavior")
    bool bIsExaminingFossil;

    UPROPERTY(BlueprintReadOnly, Category = "Paleontologist Behavior")
    bool bIsDigging;

    UPROPERTY(BlueprintReadOnly, Category = "Paleontologist Behavior")
    bool bIsTakingNotes;

    UPROPERTY(BlueprintReadOnly, Category = "Paleontologist Behavior")
    bool bIsUsingTool;

    UPROPERTY(BlueprintReadOnly, Category = "Paleontologist Behavior")
    float ToolType; // 0=None, 1=Brush, 2=Pick, 3=Magnifying Glass

    // Survival Behaviors
    UPROPERTY(BlueprintReadOnly, Category = "Survival Behavior")
    bool bIsHiding;

    UPROPERTY(BlueprintReadOnly, Category = "Survival Behavior")
    bool bIsListening;

    UPROPERTY(BlueprintReadOnly, Category = "Survival Behavior")
    bool bIsLookingAround;

    UPROPERTY(BlueprintReadOnly, Category = "Survival Behavior")
    float AlertnessLevel; // 0.0 = Calm, 1.0 = Maximum Alert

    UPROPERTY(BlueprintReadOnly, Category = "Survival Behavior")
    float FearLevel; // 0.0 = No Fear, 1.0 = Terrified

    // Environmental Adaptation
    UPROPERTY(BlueprintReadOnly, Category = "Environment")
    float GroundSlope;

    UPROPERTY(BlueprintReadOnly, Category = "Environment")
    bool bIsOnUnstableGround;

    UPROPERTY(BlueprintReadOnly, Category = "Environment")
    bool bIsInDenseVegetation;

    UPROPERTY(BlueprintReadOnly, Category = "Environment")
    float VisibilityLevel; // How hidden the player is

    // Animation Blending
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Blending")
    float BlendSpaceX; // Forward/Backward movement

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Blending")
    float BlendSpaceY; // Left/Right movement

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Blending")
    float LeanAmount; // Body lean for turning

    // Functions
    UFUNCTION(BlueprintCallable, Category = "Player Animation")
    void SetMovementState(EPlayerMovementState NewState);

    UFUNCTION(BlueprintCallable, Category = "Player Animation")
    void SetEmotionalState(EPlayerEmotionalState NewState);

    UFUNCTION(BlueprintCallable, Category = "Player Animation")
    void TriggerStartleReaction(float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Player Animation")
    void SetAlertness(float NewAlertness);

    UFUNCTION(BlueprintCallable, Category = "Player Animation")
    void SetFearLevel(float NewFearLevel);

private:
    // Character Reference
    UPROPERTY()
    ACharacter* OwningCharacter;

    // IK Calculation Functions
    void CalculateFootIK();
    void PerformFootTrace(FName SocketName, float& OutOffset, FRotator& OutRotation);
    
    // State Update Functions
    void UpdateMovementState();
    void UpdateEmotionalState();
    void UpdateEnvironmentalFactors();
    
    // Motion Matching Support
    void UpdateMotionMatchingVariables();
    
    // Utility Functions
    float CalculateGroundSlope();
    bool CheckForDenseVegetation();
    float CalculateVisibilityLevel();
};