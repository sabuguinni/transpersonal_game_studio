#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Core_WalkAroundPhysics.generated.h"

TRANSPERSONALGAME_API DECLARE_LOG_CATEGORY_EXTERN(LogWalkAroundPhysics, Log, All);

/**
 * Movement state for WALK AROUND milestone
 */
UENUM(BlueprintType)
enum class ECore_WalkAroundState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walking     UMETA(DisplayName = "Walking"),
    Running     UMETA(DisplayName = "Running"),
    Jumping     UMETA(DisplayName = "Jumping"),
    Falling     UMETA(DisplayName = "Falling"),
    Landing     UMETA(DisplayName = "Landing")
};

/**
 * Movement configuration for basic WALK AROUND gameplay
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_WalkAroundConfig
{
    GENERATED_BODY()

    /** Base walking speed */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float WalkSpeed = 300.0f;

    /** Running speed multiplier */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float RunSpeedMultiplier = 2.0f;

    /** Jump velocity */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float JumpVelocity = 600.0f;

    /** Air control factor */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float AirControl = 0.2f;

    /** Ground friction */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float GroundFriction = 8.0f;

    /** Braking deceleration walking */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float BrakingDecelerationWalking = 2000.0f;

    /** Braking deceleration falling */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float BrakingDecelerationFalling = 0.0f;
};

/**
 * Physics component for WALK AROUND milestone
 * Provides basic character movement physics for minimum viable prototype
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_WalkAroundPhysics : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_WalkAroundPhysics();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    /** Initialize movement component with WALK AROUND settings */
    UFUNCTION(BlueprintCallable, Category = "Walk Around Physics")
    void InitializeWalkAroundMovement(UCharacterMovementComponent* MovementComp);

    /** Configure camera boom for third person view */
    UFUNCTION(BlueprintCallable, Category = "Walk Around Physics")
    void ConfigureCameraBoom(class USpringArmComponent* SpringArm, class UCameraComponent* Camera);

    /** Update movement state based on character velocity */
    UFUNCTION(BlueprintCallable, Category = "Walk Around Physics")
    void UpdateMovementState(const FVector& Velocity, bool bIsOnGround, bool bIsJumping);

    /** Get current movement state */
    UFUNCTION(BlueprintPure, Category = "Walk Around Physics")
    ECore_WalkAroundState GetMovementState() const { return CurrentState; }

    /** Check if character can jump */
    UFUNCTION(BlueprintPure, Category = "Walk Around Physics")
    bool CanJump() const;

    /** Apply terrain interaction forces */
    UFUNCTION(BlueprintCallable, Category = "Walk Around Physics")
    void ApplyTerrainInteraction(const FHitResult& GroundHit);

    /** Validate WALK AROUND milestone requirements */
    UFUNCTION(BlueprintCallable, Category = "Walk Around Physics")
    bool ValidateWalkAroundMilestone() const;

protected:
    /** Current movement state */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    ECore_WalkAroundState CurrentState = ECore_WalkAroundState::Idle;

    /** Movement configuration */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    FCore_WalkAroundConfig MovementConfig;

    /** Reference to character movement component */
    UPROPERTY()
    TWeakObjectPtr<UCharacterMovementComponent> MovementComponent;

    /** Reference to capsule component */
    UPROPERTY()
    TWeakObjectPtr<UCapsuleComponent> CapsuleComponent;

    /** Time since last state change */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    float StateTime = 0.0f;

    /** Previous velocity for acceleration calculation */
    FVector PreviousVelocity = FVector::ZeroVector;

    /** Ground normal for terrain interaction */
    FVector GroundNormal = FVector::UpVector;

private:
    /** Update state timers */
    void UpdateStateTimers(float DeltaTime);

    /** Determine movement state from velocity */
    ECore_WalkAroundState DetermineStateFromVelocity(const FVector& Velocity, bool bIsOnGround, bool bIsJumping) const;

    /** Apply movement configuration to character movement component */
    void ApplyMovementConfiguration();

    /** Validate component references */
    bool ValidateComponents() const;
};