#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Anim_MotionMatchingController.generated.h"

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
    Climbing    UMETA(DisplayName = "Climbing")
};

UENUM(BlueprintType)
enum class EAnim_TerrainType : uint8
{
    Flat        UMETA(DisplayName = "Flat"),
    Uphill      UMETA(DisplayName = "Uphill"),
    Downhill    UMETA(DisplayName = "Downhill"),
    Rocky       UMETA(DisplayName = "Rocky"),
    Muddy       UMETA(DisplayName = "Muddy"),
    Sand        UMETA(DisplayName = "Sand"),
    Water       UMETA(DisplayName = "Water")
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
    bool bIsCrouching = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    EAnim_MovementState MovementState = EAnim_MovementState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    EAnim_TerrainType TerrainType = EAnim_TerrainType::Flat;

    FAnim_MotionData()
    {
        Speed = 0.0f;
        Direction = 0.0f;
        Velocity = FVector::ZeroVector;
        Acceleration = FVector::ZeroVector;
        bIsInAir = false;
        bIsCrouching = false;
        MovementState = EAnim_MovementState::Idle;
        TerrainType = EAnim_TerrainType::Flat;
    }
};

UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_MotionMatchingController : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_MotionMatchingController();

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
    EAnim_MovementState DetermineMovementState() const;

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void SetMovementState(EAnim_MovementState NewState);

    // Terrain Analysis
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    EAnim_TerrainType AnalyzeTerrainType() const;

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    float GetTerrainSlope() const;

    // Animation Selection
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    class UAnimSequence* SelectBestAnimation() const;

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    float CalculateAnimationScore(class UAnimSequence* Animation) const;

    // Blend Space Management
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void UpdateBlendSpaceParameters();

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    FVector2D GetBlendSpaceCoordinates() const;

protected:
    // Current motion data
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Motion Data")
    FAnim_MotionData CurrentMotionData;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Motion Data")
    FAnim_MotionData PreviousMotionData;

    // Character references
    UPROPERTY()
    class ACharacter* OwnerCharacter;

    UPROPERTY()
    class UCharacterMovementComponent* MovementComponent;

    UPROPERTY()
    class USkeletalMeshComponent* MeshComponent;

    // Animation assets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    class UBlendSpace* MovementBlendSpace;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    TArray<class UAnimSequence*> IdleAnimations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    TArray<class UAnimSequence*> WalkAnimations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    TArray<class UAnimSequence*> RunAnimations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    class UAnimMontage* JumpMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    class UAnimMontage* LandMontage;

    // Motion matching parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching", meta = (ClampMin = "0.0", ClampMax = "1000.0"))
    float WalkThreshold = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching", meta = (ClampMin = "0.0", ClampMax = "1000.0"))
    float RunThreshold = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching", meta = (ClampMin = "0.0", ClampMax = "1000.0"))
    float SprintThreshold = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching", meta = (ClampMin = "0.0", ClampMax = "10.0"))
    float MotionSmoothingRate = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float TerrainAnalysisRadius = 100.0f;

private:
    // Internal state tracking
    float StateTransitionTimer = 0.0f;
    float LastGroundTime = 0.0f;
    FVector LastGroundLocation = FVector::ZeroVector;
    
    // Performance optimization
    float LastUpdateTime = 0.0f;
    const float UpdateFrequency = 0.016f; // ~60fps
};