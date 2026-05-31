#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Anim_MotionMatchingComponent.generated.h"

UENUM(BlueprintType)
enum class EAnim_MovementState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walking     UMETA(DisplayName = "Walking"),
    Running     UMETA(DisplayName = "Running"),
    Sprinting   UMETA(DisplayName = "Sprinting"),
    Jumping     UMETA(DisplayName = "Jumping"),
    Falling     UMETA(DisplayName = "Falling"),
    Landing     UMETA(DisplayName = "Landing"),
    Crouching   UMETA(DisplayName = "Crouching"),
    Crawling    UMETA(DisplayName = "Crawling")
};

UENUM(BlueprintType)
enum class EAnim_CombatState : uint8
{
    None            UMETA(DisplayName = "None"),
    Ready           UMETA(DisplayName = "Ready"),
    Attacking       UMETA(DisplayName = "Attacking"),
    Blocking        UMETA(DisplayName = "Blocking"),
    Dodging         UMETA(DisplayName = "Dodging"),
    Stunned         UMETA(DisplayName = "Stunned"),
    Dead            UMETA(DisplayName = "Dead")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MotionData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    float Speed = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    float Direction = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    FVector Velocity = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    FVector Acceleration = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    bool bIsInAir = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    bool bIsMoving = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    float TimeSinceLastMovement = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    EAnim_MovementState MovementState = EAnim_MovementState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    EAnim_CombatState CombatState = EAnim_CombatState::None;
};

UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_MotionMatchingComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_MotionMatchingComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Motion data analysis
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void UpdateMotionData(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    FAnim_MotionData GetCurrentMotionData() const { return CurrentMotionData; }

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    EAnim_MovementState GetMovementState() const { return CurrentMotionData.MovementState; }

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    EAnim_CombatState GetCombatState() const { return CurrentMotionData.CombatState; }

    // Animation selection
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    UAnimSequence* SelectBestAnimation();

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void SetCombatState(EAnim_CombatState NewState);

    // Transition handling
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    bool CanTransitionTo(EAnim_MovementState NewState) const;

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    float GetTransitionBlendTime(EAnim_MovementState FromState, EAnim_MovementState ToState) const;

protected:
    // Character reference
    UPROPERTY(BlueprintReadOnly, Category = "References")
    ACharacter* OwnerCharacter;

    UPROPERTY(BlueprintReadOnly, Category = "References")
    UCharacterMovementComponent* MovementComponent;

    // Current motion data
    UPROPERTY(BlueprintReadOnly, Category = "Motion Data")
    FAnim_MotionData CurrentMotionData;

    UPROPERTY(BlueprintReadOnly, Category = "Motion Data")
    FAnim_MotionData PreviousMotionData;

    // Animation assets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    TMap<EAnim_MovementState, UAnimSequence*> MovementAnimations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    TMap<EAnim_CombatState, UAnimSequence*> CombatAnimations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    UBlendSpace* MovementBlendSpace;

    // Thresholds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds", meta = (ClampMin = "0.0", ClampMax = "1000.0"))
    float WalkSpeedThreshold = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds", meta = (ClampMin = "0.0", ClampMax = "1000.0"))
    float RunSpeedThreshold = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds", meta = (ClampMin = "0.0", ClampMax = "1000.0"))
    float SprintSpeedThreshold = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds", meta = (ClampMin = "0.0", ClampMax = "10.0"))
    float IdleTimeThreshold = 2.0f;

    // Transition settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transitions", meta = (ClampMin = "0.0", ClampMax = "2.0"))
    float DefaultTransitionTime = 0.25f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transitions", meta = (ClampMin = "0.0", ClampMax = "2.0"))
    float CombatTransitionTime = 0.15f;

private:
    // Internal state tracking
    float TimeSinceStateChange = 0.0f;
    EAnim_MovementState LastMovementState = EAnim_MovementState::Idle;
    EAnim_CombatState LastCombatState = EAnim_CombatState::None;

    // Helper functions
    void AnalyzeMovementState(float DeltaTime);
    void UpdateVelocityData();
    bool IsCharacterInAir() const;
    float CalculateMovementDirection() const;
};