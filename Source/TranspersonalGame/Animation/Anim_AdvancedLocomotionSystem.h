#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "Animation/AnimSequence.h"
#include "SharedTypes.h"
#include "Anim_AdvancedLocomotionSystem.generated.h"

UENUM(BlueprintType)
enum class EAnim_LocomotionState : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Walking         UMETA(DisplayName = "Walking"),
    Running         UMETA(DisplayName = "Running"),
    Sprinting       UMETA(DisplayName = "Sprinting"),
    Crouching       UMETA(DisplayName = "Crouching"),
    Crawling        UMETA(DisplayName = "Crawling"),
    Jumping         UMETA(DisplayName = "Jumping"),
    Falling         UMETA(DisplayName = "Falling"),
    Landing         UMETA(DisplayName = "Landing"),
    Climbing        UMETA(DisplayName = "Climbing"),
    Swimming        UMETA(DisplayName = "Swimming"),
    Injured         UMETA(DisplayName = "Injured")
};

UENUM(BlueprintType)
enum class EAnim_MovementDirection : uint8
{
    Forward         UMETA(DisplayName = "Forward"),
    Backward        UMETA(DisplayName = "Backward"),
    Left            UMETA(DisplayName = "Left"),
    Right           UMETA(DisplayName = "Right"),
    ForwardLeft     UMETA(DisplayName = "Forward Left"),
    ForwardRight    UMETA(DisplayName = "Forward Right"),
    BackwardLeft    UMETA(DisplayName = "Backward Left"),
    BackwardRight   UMETA(DisplayName = "Backward Right")
};

UENUM(BlueprintType)
enum class EAnim_Stance : uint8
{
    Standing        UMETA(DisplayName = "Standing"),
    Crouched        UMETA(DisplayName = "Crouched"),
    Prone           UMETA(DisplayName = "Prone")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_LocomotionData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion")
    EAnim_LocomotionState CurrentState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion")
    EAnim_MovementDirection MovementDirection;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion")
    EAnim_Stance CurrentStance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion")
    float Speed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion")
    float Direction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion")
    float LeanAmount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion")
    bool bIsMoving;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion")
    bool bIsInAir;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion")
    bool bIsAccelerating;

    FAnim_LocomotionData()
    {
        CurrentState = EAnim_LocomotionState::Idle;
        MovementDirection = EAnim_MovementDirection::Forward;
        CurrentStance = EAnim_Stance::Standing;
        Speed = 0.0f;
        Direction = 0.0f;
        LeanAmount = 0.0f;
        bIsMoving = false;
        bIsInAir = false;
        bIsAccelerating = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_LocomotionSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float WalkSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float RunSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float SprintSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float CrouchSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float CrawlSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float SwimSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float InjuredSpeedMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float AccelerationThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float DirectionChangeThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float LeanInterpSpeed;

    FAnim_LocomotionSettings()
    {
        WalkSpeed = 150.0f;
        RunSpeed = 375.0f;
        SprintSpeed = 600.0f;
        CrouchSpeed = 100.0f;
        CrawlSpeed = 50.0f;
        SwimSpeed = 200.0f;
        InjuredSpeedMultiplier = 0.5f;
        AccelerationThreshold = 10.0f;
        DirectionChangeThreshold = 45.0f;
        LeanInterpSpeed = 5.0f;
    }
};

UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_AdvancedLocomotionSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_AdvancedLocomotionSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core locomotion data
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Locomotion")
    FAnim_LocomotionData LocomotionData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    FAnim_LocomotionSettings LocomotionSettings;

    // Animation assets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    TObjectPtr<UBlendSpace> IdleBlendSpace;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    TObjectPtr<UBlendSpace> WalkBlendSpace;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    TObjectPtr<UBlendSpace> RunBlendSpace;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    TObjectPtr<UBlendSpace> SprintBlendSpace;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    TObjectPtr<UBlendSpace> CrouchBlendSpace;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    TObjectPtr<UAnimSequence> JumpStartAnimation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    TObjectPtr<UAnimSequence> JumpLoopAnimation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    TObjectPtr<UAnimSequence> JumpLandAnimation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    TObjectPtr<UAnimMontage> ClimbMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    TObjectPtr<UAnimMontage> SwimMontage;

    // State management functions
    UFUNCTION(BlueprintCallable, Category = "Locomotion")
    void UpdateLocomotionState(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Locomotion")
    void SetLocomotionState(EAnim_LocomotionState NewState);

    UFUNCTION(BlueprintCallable, Category = "Locomotion")
    void SetStance(EAnim_Stance NewStance);

    UFUNCTION(BlueprintCallable, Category = "Locomotion")
    EAnim_LocomotionState GetLocomotionState() const { return LocomotionData.CurrentState; }

    UFUNCTION(BlueprintCallable, Category = "Locomotion")
    EAnim_Stance GetStance() const { return LocomotionData.CurrentStance; }

    // Movement analysis functions
    UFUNCTION(BlueprintCallable, Category = "Locomotion")
    void CalculateMovementDirection();

    UFUNCTION(BlueprintCallable, Category = "Locomotion")
    void CalculateSpeed();

    UFUNCTION(BlueprintCallable, Category = "Locomotion")
    void CalculateLean(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Locomotion")
    bool ShouldTransitionToState(EAnim_LocomotionState TargetState) const;

    // Getters for animation blueprint
    UFUNCTION(BlueprintPure, Category = "Locomotion")
    float GetSpeed() const { return LocomotionData.Speed; }

    UFUNCTION(BlueprintPure, Category = "Locomotion")
    float GetDirection() const { return LocomotionData.Direction; }

    UFUNCTION(BlueprintPure, Category = "Locomotion")
    float GetLeanAmount() const { return LocomotionData.LeanAmount; }

    UFUNCTION(BlueprintPure, Category = "Locomotion")
    bool IsMoving() const { return LocomotionData.bIsMoving; }

    UFUNCTION(BlueprintPure, Category = "Locomotion")
    bool IsInAir() const { return LocomotionData.bIsInAir; }

    UFUNCTION(BlueprintPure, Category = "Locomotion")
    bool IsAccelerating() const { return LocomotionData.bIsAccelerating; }

    UFUNCTION(BlueprintPure, Category = "Locomotion")
    EAnim_MovementDirection GetMovementDirection() const { return LocomotionData.MovementDirection; }

    // Utility functions
    UFUNCTION(BlueprintCallable, Category = "Locomotion")
    void ResetLocomotionData();

    UFUNCTION(BlueprintCallable, Category = "Locomotion")
    void SetInjured(bool bIsInjured);

    UFUNCTION(BlueprintCallable, Category = "Locomotion")
    float GetMaxSpeedForCurrentState() const;

    UFUNCTION(BlueprintCallable, Category = "Locomotion")
    bool CanTransitionFromState(EAnim_LocomotionState FromState, EAnim_LocomotionState ToState) const;

private:
    // Cached references
    UPROPERTY()
    TObjectPtr<class ACharacter> OwnerCharacter;

    UPROPERTY()
    TObjectPtr<class UCharacterMovementComponent> MovementComponent;

    // Internal state tracking
    FVector LastVelocity;
    FVector LastLocation;
    float LastSpeed;
    float TimeSinceLastMovement;
    bool bIsInjured;

    // Internal update functions
    void UpdateMovementState();
    void UpdateAirState();
    void UpdateAcceleration();
    void CheckForStateTransitions();

    // Helper functions
    float CalculateDirectionAngle(const FVector& Velocity, const FVector& Forward) const;
    EAnim_MovementDirection VectorToMovementDirection(const FVector& Direction) const;
    bool IsValidStateTransition(EAnim_LocomotionState FromState, EAnim_LocomotionState ToState) const;
};