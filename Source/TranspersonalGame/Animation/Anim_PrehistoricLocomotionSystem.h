#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Animation/BlendSpace.h"
#include "Animation/AnimMontage.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Anim_PrehistoricLocomotionSystem.generated.h"

UENUM(BlueprintType)
enum class EAnim_LocomotionState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walking     UMETA(DisplayName = "Walking"),
    Running     UMETA(DisplayName = "Running"),
    Sprinting   UMETA(DisplayName = "Sprinting"),
    Crouching   UMETA(DisplayName = "Crouching"),
    Jumping     UMETA(DisplayName = "Jumping"),
    Falling     UMETA(DisplayName = "Falling"),
    Landing     UMETA(DisplayName = "Landing"),
    Climbing    UMETA(DisplayName = "Climbing"),
    Swimming    UMETA(DisplayName = "Swimming")
};

UENUM(BlueprintType)
enum class EAnim_MovementDirection : uint8
{
    Forward     UMETA(DisplayName = "Forward"),
    Backward    UMETA(DisplayName = "Backward"),
    Left        UMETA(DisplayName = "Left"),
    Right       UMETA(DisplayName = "Right"),
    ForwardLeft UMETA(DisplayName = "Forward Left"),
    ForwardRight UMETA(DisplayName = "Forward Right"),
    BackwardLeft UMETA(DisplayName = "Backward Left"),
    BackwardRight UMETA(DisplayName = "Backward Right")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_LocomotionData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion")
    float Speed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion")
    float Direction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion")
    bool bIsMoving;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion")
    bool bIsInAir;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion")
    bool bIsCrouching;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion")
    EAnim_LocomotionState CurrentState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion")
    EAnim_MovementDirection MovementDirection;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion")
    float LeanAmount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion")
    float TurnInPlace;

    FAnim_LocomotionData()
    {
        Speed = 0.0f;
        Direction = 0.0f;
        bIsMoving = false;
        bIsInAir = false;
        bIsCrouching = false;
        CurrentState = EAnim_LocomotionState::Idle;
        MovementDirection = EAnim_MovementDirection::Forward;
        LeanAmount = 0.0f;
        TurnInPlace = 0.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_PrehistoricLocomotionSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_PrehistoricLocomotionSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core locomotion functions
    UFUNCTION(BlueprintCallable, Category = "Prehistoric Locomotion")
    void UpdateLocomotionData(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Prehistoric Locomotion")
    void SetLocomotionState(EAnim_LocomotionState NewState);

    UFUNCTION(BlueprintPure, Category = "Prehistoric Locomotion")
    FAnim_LocomotionData GetLocomotionData() const { return LocomotionData; }

    UFUNCTION(BlueprintPure, Category = "Prehistoric Locomotion")
    EAnim_LocomotionState GetCurrentState() const { return LocomotionData.CurrentState; }

    UFUNCTION(BlueprintCallable, Category = "Prehistoric Locomotion")
    void PlayLocomotionMontage(UAnimMontage* Montage, float PlayRate = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Prehistoric Locomotion")
    void StopLocomotionMontage(float BlendOutTime = 0.25f);

    // Movement analysis
    UFUNCTION(BlueprintCallable, Category = "Prehistoric Locomotion")
    void AnalyzeMovementInput();

    UFUNCTION(BlueprintCallable, Category = "Prehistoric Locomotion")
    void CalculateMovementDirection();

    UFUNCTION(BlueprintCallable, Category = "Prehistoric Locomotion")
    void UpdateTurnInPlace(float DeltaTime);

    // Terrain adaptation
    UFUNCTION(BlueprintCallable, Category = "Prehistoric Locomotion")
    void AdaptToTerrain();

    UFUNCTION(BlueprintCallable, Category = "Prehistoric Locomotion")
    bool IsOnSteepSlope() const;

    UFUNCTION(BlueprintCallable, Category = "Prehistoric Locomotion")
    float GetSlopeAngle() const;

protected:
    // Core locomotion data
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Locomotion Data")
    FAnim_LocomotionData LocomotionData;

    // Character references
    UPROPERTY()
    ACharacter* OwnerCharacter;

    UPROPERTY()
    UCharacterMovementComponent* MovementComponent;

    UPROPERTY()
    USkeletalMeshComponent* MeshComponent;

    // Animation assets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    UBlendSpace* LocomotionBlendSpace;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    UAnimMontage* JumpMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    UAnimMontage* LandMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    UAnimMontage* CrouchMontage;

    // Locomotion settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion Settings")
    float WalkSpeedThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion Settings")
    float RunSpeedThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion Settings")
    float SprintSpeedThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion Settings")
    float TurnInPlaceThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion Settings")
    float SlopeAngleThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion Settings")
    float MovementDirectionDeadzone;

    // State tracking
    UPROPERTY()
    EAnim_LocomotionState PreviousState;

    UPROPERTY()
    float StateChangeTimer;

    UPROPERTY()
    float LastGroundedTime;

    UPROPERTY()
    bool bWasMovingLastFrame;

    UPROPERTY()
    FVector LastVelocity;

    UPROPERTY()
    float AccumulatedTurnAmount;

private:
    void InitializeComponent();
    void CacheCharacterReferences();
    void UpdateMovementState(float DeltaTime);
    void UpdateAirborneState();
    void UpdateGroundedState();
    void HandleStateTransitions();
    float CalculateSpeedRatio() const;
    float CalculateDirectionAngle() const;
    void SmoothLocomotionValues(float DeltaTime);
};