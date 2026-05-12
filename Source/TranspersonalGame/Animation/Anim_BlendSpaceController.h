#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/BlendSpace.h"
#include "Animation/BlendSpace1D.h"
#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "TranspersonalGame/SharedTypes.h"
#include "Anim_BlendSpaceController.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_BlendSpaceData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Space")
    UBlendSpace* BlendSpace;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Space")
    UBlendSpace1D* BlendSpace1D;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Space")
    FString BlendSpaceName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Space")
    ECharacterMovementState AssociatedState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Space")
    float BlendSpaceX;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Space")
    float BlendSpaceY;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Space")
    float BlendWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Space")
    bool bIsActive;

    FAnim_BlendSpaceData()
    {
        BlendSpace = nullptr;
        BlendSpace1D = nullptr;
        BlendSpaceName = TEXT("Default");
        AssociatedState = ECharacterMovementState::Idle;
        BlendSpaceX = 0.0f;
        BlendSpaceY = 0.0f;
        BlendWeight = 0.0f;
        bIsActive = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MovementBlendParams
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Blend")
    float Speed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Blend")
    float Direction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Blend")
    float Lean;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Blend")
    float TurnRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Blend")
    bool bIsMoving;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Blend")
    bool bIsTurning;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Blend")
    bool bIsAccelerating;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Blend")
    bool bIsDecelerating;

    FAnim_MovementBlendParams()
    {
        Speed = 0.0f;
        Direction = 0.0f;
        Lean = 0.0f;
        TurnRate = 0.0f;
        bIsMoving = false;
        bIsTurning = false;
        bIsAccelerating = false;
        bIsDecelerating = false;
    }
};

UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_BlendSpaceController : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_BlendSpaceController();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Blend space management
    UFUNCTION(BlueprintCallable, Category = "Blend Space Controller")
    void InitializeBlendSpaces();

    UFUNCTION(BlueprintCallable, Category = "Blend Space Controller")
    void UpdateBlendSpaceParameters();

    UFUNCTION(BlueprintCallable, Category = "Blend Space Controller")
    void SetActiveBlendSpace(ECharacterMovementState MovementState);

    UFUNCTION(BlueprintCallable, Category = "Blend Space Controller")
    void RegisterBlendSpace(UBlendSpace* BlendSpaceAsset, ECharacterMovementState State, const FString& Name);

    UFUNCTION(BlueprintCallable, Category = "Blend Space Controller")
    void RegisterBlendSpace1D(UBlendSpace1D* BlendSpaceAsset, ECharacterMovementState State, const FString& Name);

    // Movement parameter calculation
    UFUNCTION(BlueprintCallable, Category = "Movement Parameters")
    FAnim_MovementBlendParams CalculateMovementParameters();

    UFUNCTION(BlueprintCallable, Category = "Movement Parameters")
    void UpdateMovementDirection();

    UFUNCTION(BlueprintCallable, Category = "Movement Parameters")
    void UpdateMovementSpeed();

    UFUNCTION(BlueprintCallable, Category = "Movement Parameters")
    void UpdateTurnRate();

    UFUNCTION(BlueprintCallable, Category = "Movement Parameters")
    void UpdateLeanAmount();

    // Blend space parameter mapping
    UFUNCTION(BlueprintCallable, Category = "Blend Space Parameters")
    void MapSpeedToBlendSpace(float Speed, float& OutX, float& OutY);

    UFUNCTION(BlueprintCallable, Category = "Blend Space Parameters")
    void MapDirectionToBlendSpace(float Direction, float& OutX, float& OutY);

    UFUNCTION(BlueprintCallable, Category = "Blend Space Parameters")
    void SmoothBlendSpaceTransition(float DeltaTime);

    // Character reference setup
    UFUNCTION(BlueprintCallable, Category = "Setup")
    void SetCharacterReference(ACharacter* InCharacter);

    // Debug and utility functions
    UFUNCTION(BlueprintCallable, Category = "Debug")
    void DebugDrawBlendSpaceInfo();

    UFUNCTION(BlueprintCallable, Category = "Debug")
    FString GetCurrentBlendSpaceInfo();

    UFUNCTION(BlueprintCallable, Category = "Utility")
    FAnim_BlendSpaceData GetActiveBlendSpaceData();

protected:
    // Blend space database
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Spaces")
    TArray<FAnim_BlendSpaceData> BlendSpaceDatabase;

    // Current blend state
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Current State")
    FAnim_MovementBlendParams CurrentMovementParams;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Current State")
    FAnim_BlendSpaceData ActiveBlendSpace;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Current State")
    ECharacterMovementState CurrentMovementState;

    // Character references
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "References")
    ACharacter* OwnerCharacter;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "References")
    UCharacterMovementComponent* MovementComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "References")
    USkeletalMeshComponent* MeshComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "References")
    UAnimInstance* AnimInstance;

    // Blend space settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Space Settings")
    float BlendSpaceUpdateRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Space Settings")
    float ParameterSmoothingSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Space Settings")
    float DirectionSmoothingSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Space Settings")
    float SpeedSmoothingSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Space Settings")
    float LeanSmoothingSpeed;

    // Movement thresholds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Thresholds")
    float WalkSpeedThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Thresholds")
    float RunSpeedThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Thresholds")
    float SprintSpeedThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Thresholds")
    float TurnRateThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Thresholds")
    float AccelerationThreshold;

    // Smoothed values for interpolation
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Smoothed Values")
    float SmoothedSpeed;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Smoothed Values")
    float SmoothedDirection;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Smoothed Values")
    float SmoothedLean;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Smoothed Values")
    float SmoothedTurnRate;

private:
    float LastUpdateTime;
    FVector PreviousVelocity;
    FRotator PreviousRotation;
    bool bIsInitialized;

    // Helper functions
    void CacheCharacterReferences();
    void ValidateBlendSpaces();
    ECharacterMovementState DetermineMovementState();
    void InterpolateBlendSpaceParameters(float DeltaTime);
};