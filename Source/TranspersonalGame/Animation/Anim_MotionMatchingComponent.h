#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "Engine/Engine.h"
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
enum class EAnim_ActionState : uint8
{
    None            UMETA(DisplayName = "None"),
    Crafting        UMETA(DisplayName = "Crafting"),
    Gathering       UMETA(DisplayName = "Gathering"),
    Combat          UMETA(DisplayName = "Combat"),
    Eating          UMETA(DisplayName = "Eating"),
    Drinking        UMETA(DisplayName = "Drinking"),
    Building        UMETA(DisplayName = "Building"),
    Climbing        UMETA(DisplayName = "Climbing")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MotionData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    float Speed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    float Direction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    FVector Velocity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    bool bIsOnGround;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    bool bIsMoving;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    float TimeSinceLastMove;

    FAnim_MotionData()
    {
        Speed = 0.0f;
        Direction = 0.0f;
        Velocity = FVector::ZeroVector;
        bIsOnGround = true;
        bIsMoving = false;
        TimeSinceLastMove = 0.0f;
    }
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

    // Motion Matching Core Functions
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void UpdateMotionData(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    FAnim_MotionData GetCurrentMotionData() const { return CurrentMotionData; }

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    EAnim_MovementState GetCurrentMovementState() const { return CurrentMovementState; }

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    EAnim_ActionState GetCurrentActionState() const { return CurrentActionState; }

    // State Management
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void SetMovementState(EAnim_MovementState NewState);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void SetActionState(EAnim_ActionState NewState);

    // Animation Blending
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    float GetBlendWeight(EAnim_MovementState State) const;

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void UpdateBlendWeights(float DeltaTime);

    // Terrain Adaptation
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void AdaptToTerrain();

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    float GetTerrainSlope() const { return TerrainSlope; }

protected:
    // Core Motion Data
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Motion Data")
    FAnim_MotionData CurrentMotionData;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Motion Data")
    FAnim_MotionData PreviousMotionData;

    // State Management
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    EAnim_MovementState CurrentMovementState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    EAnim_MovementState PreviousMovementState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    EAnim_ActionState CurrentActionState;

    // Blend Weights for smooth transitions
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Blending")
    TMap<EAnim_MovementState, float> BlendWeights;

    // Terrain Adaptation
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Terrain")
    float TerrainSlope;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float MaxTerrainAdaptationAngle;

    // Animation References
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    class UBlendSpace* MovementBlendSpace;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    class UAnimMontage* JumpMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    class UAnimMontage* LandingMontage;

    // Timing and Smoothing
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float StateTransitionSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float BlendSmoothingSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float MinMovementThreshold;

private:
    // Internal state tracking
    float StateTransitionTimer;
    bool bPendingStateChange;
    EAnim_MovementState PendingMovementState;

    // Cached references
    class ACharacter* OwnerCharacter;
    class UCharacterMovementComponent* MovementComponent;
    class USkeletalMeshComponent* MeshComponent;

    // Helper functions
    void InitializeBlendWeights();
    void UpdateCharacterReferences();
    EAnim_MovementState CalculateMovementState() const;
    void PerformTerrainTrace();
    float CalculateBlendAlpha(float CurrentValue, float TargetValue, float DeltaTime) const;
};