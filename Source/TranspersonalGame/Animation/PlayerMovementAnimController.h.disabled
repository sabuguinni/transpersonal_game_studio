#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "PlayerMovementAnimController.generated.h"

// Animation states for player movement
UENUM(BlueprintType)
enum class EAnim_PlayerMovementState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walking     UMETA(DisplayName = "Walking"),
    Running     UMETA(DisplayName = "Running"),
    Jumping     UMETA(DisplayName = "Jumping"),
    Falling     UMETA(DisplayName = "Falling"),
    Landing     UMETA(DisplayName = "Landing"),
    Crouching   UMETA(DisplayName = "Crouching"),
    Crawling    UMETA(DisplayName = "Crawling")
};

// Player animation data structure
USTRUCT(BlueprintType)
struct FAnim_PlayerMovementData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float Speed = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float Direction = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsInAir = false;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsCrouching = false;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsMoving = false;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    EAnim_PlayerMovementState MovementState = EAnim_PlayerMovementState::Idle;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float FearLevel = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float StaminaLevel = 1.0f;

    FAnim_PlayerMovementData()
    {
        Speed = 0.0f;
        Direction = 0.0f;
        bIsInAir = false;
        bIsCrouching = false;
        bIsMoving = false;
        MovementState = EAnim_PlayerMovementState::Idle;
        FearLevel = 0.0f;
        StaminaLevel = 1.0f;
    }
};

/**
 * Controls player character animations based on movement and survival states
 * Integrates with UE5 Animation Blueprint system for smooth transitions
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPlayerMovementAnimController : public UActorComponent
{
    GENERATED_BODY()

public:
    UPlayerMovementAnimController();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Initialize the animation controller with character reference
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void InitializeWithCharacter(ACharacter* InCharacter);

    // Update animation state based on character movement
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateAnimationState(float DeltaTime);

    // Get current movement data for animation blueprint
    UFUNCTION(BlueprintPure, Category = "Animation")
    FAnim_PlayerMovementData GetMovementData() const { return MovementData; }

    // Set fear level (affects animation speed and behavior)
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetFearLevel(float InFearLevel);

    // Set stamina level (affects movement speed)
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetStaminaLevel(float InStaminaLevel);

    // Play specific animation montage
    UFUNCTION(BlueprintCallable, Category = "Animation")
    bool PlayAnimationMontage(UAnimMontage* Montage, float PlayRate = 1.0f);

    // Stop current animation montage
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void StopAnimationMontage(float BlendOutTime = 0.25f);

protected:
    // Character reference
    UPROPERTY(BlueprintReadOnly, Category = "Animation")
    TObjectPtr<ACharacter> OwnerCharacter;

    // Character movement component reference
    UPROPERTY(BlueprintReadOnly, Category = "Animation")
    TObjectPtr<UCharacterMovementComponent> MovementComponent;

    // Skeletal mesh component reference
    UPROPERTY(BlueprintReadOnly, Category = "Animation")
    TObjectPtr<USkeletalMeshComponent> MeshComponent;

    // Current movement data
    UPROPERTY(BlueprintReadOnly, Category = "Animation")
    FAnim_PlayerMovementData MovementData;

    // Animation thresholds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float WalkSpeedThreshold = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float RunSpeedThreshold = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float MovementDeadZone = 10.0f;

    // Fear animation modifiers
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fear Settings")
    float FearSpeedMultiplier = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fear Settings")
    float HighFearThreshold = 0.7f;

    // Stamina animation modifiers
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina Settings")
    float LowStaminaThreshold = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina Settings")
    float LowStaminaSpeedMultiplier = 0.7f;

private:
    // Calculate movement direction relative to character facing
    float CalculateMovementDirection() const;

    // Determine current movement state based on speed and conditions
    EAnim_PlayerMovementState DetermineMovementState() const;

    // Update movement data structure
    void UpdateMovementData(float DeltaTime);

    // Previous movement state for transition detection
    EAnim_PlayerMovementState PreviousMovementState;

    // Animation state change cooldown
    float StateChangeCooldown = 0.0f;
    const float StateChangeCooldownTime = 0.1f;
};