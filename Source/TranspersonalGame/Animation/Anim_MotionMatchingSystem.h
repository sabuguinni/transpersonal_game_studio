#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/ActorComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "../SharedTypes.h"
#include "Anim_MotionMatchingSystem.generated.h"

UENUM(BlueprintType)
enum class EAnim_MovementState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walking     UMETA(DisplayName = "Walking"),
    Running     UMETA(DisplayName = "Running"),
    Crouching   UMETA(DisplayName = "Crouching"),
    Jumping     UMETA(DisplayName = "Jumping"),
    Falling     UMETA(DisplayName = "Falling"),
    Landing     UMETA(DisplayName = "Landing"),
    Climbing    UMETA(DisplayName = "Climbing")
};

UENUM(BlueprintType)
enum class EAnim_TribalRole : uint8
{
    Elder       UMETA(DisplayName = "Tribal Elder"),
    Hunter      UMETA(DisplayName = "Hunter"),
    Gatherer    UMETA(DisplayName = "Gatherer"),
    Crafter     UMETA(DisplayName = "Crafter"),
    Scout       UMETA(DisplayName = "Scout"),
    Warrior     UMETA(DisplayName = "Warrior"),
    Shaman      UMETA(DisplayName = "Shaman")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MotionData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    FVector Velocity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    FVector Acceleration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    float Speed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    float Direction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    bool bIsMoving;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    bool bIsInAir;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    EAnim_MovementState MovementState;

    FAnim_MotionData()
    {
        Velocity = FVector::ZeroVector;
        Acceleration = FVector::ZeroVector;
        Speed = 0.0f;
        Direction = 0.0f;
        bIsMoving = false;
        bIsInAir = false;
        MovementState = EAnim_MovementState::Idle;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_TribalAnimSet
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Animations")
    class UAnimMontage* IdleAnimation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Animations")
    class UAnimMontage* WalkAnimation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Animations")
    class UAnimMontage* RunAnimation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Animations")
    class UAnimMontage* WorkAnimation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Animations")
    class UAnimMontage* CombatAnimation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Animations")
    TArray<class UAnimMontage*> GestureAnimations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Animations")
    EAnim_TribalRole TribalRole;

    FAnim_TribalAnimSet()
    {
        IdleAnimation = nullptr;
        WalkAnimation = nullptr;
        RunAnimation = nullptr;
        WorkAnimation = nullptr;
        CombatAnimation = nullptr;
        TribalRole = EAnim_TribalRole::Elder;
    }
};

UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_MotionMatchingSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_MotionMatchingSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Motion Matching Core Functions
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void UpdateMotionData(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void FindBestMatchingAnimation();

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void BlendToNewAnimation(class UAnimMontage* NewAnimation, float BlendTime = 0.2f);

    // Tribal Animation Functions
    UFUNCTION(BlueprintCallable, Category = "Tribal Animations")
    void SetTribalRole(EAnim_TribalRole NewRole);

    UFUNCTION(BlueprintCallable, Category = "Tribal Animations")
    void PlayTribalGesture(const FString& GestureName);

    UFUNCTION(BlueprintCallable, Category = "Tribal Animations")
    void PlayWorkAnimation();

    UFUNCTION(BlueprintCallable, Category = "Tribal Animations")
    void PlayCombatAnimation();

    // Movement State Management
    UFUNCTION(BlueprintCallable, Category = "Movement")
    void SetMovementState(EAnim_MovementState NewState);

    UFUNCTION(BlueprintPure, Category = "Movement")
    EAnim_MovementState GetCurrentMovementState() const { return CurrentMotionData.MovementState; }

    UFUNCTION(BlueprintPure, Category = "Movement")
    float GetCurrentSpeed() const { return CurrentMotionData.Speed; }

    // Terrain Adaptation
    UFUNCTION(BlueprintCallable, Category = "Terrain")
    void UpdateTerrainAdaptation();

    UFUNCTION(BlueprintCallable, Category = "Terrain")
    void EnableFootIK(bool bEnable);

protected:
    // Core Motion Data
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Motion Data")
    FAnim_MotionData CurrentMotionData;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Motion Data")
    FAnim_MotionData PreviousMotionData;

    // Tribal Animation Sets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Animations")
    FAnim_TribalAnimSet TribalAnimations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Animations")
    TMap<EAnim_TribalRole, FAnim_TribalAnimSet> RoleAnimationSets;

    // Motion Matching Parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float MotionMatchingThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float BlendTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    bool bUseMotionMatching;

    // Foot IK Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    bool bEnableFootIK;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    float FootIKTraceDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    float FootIKInterpSpeed;

    // Component References
    UPROPERTY()
    class USkeletalMeshComponent* SkeletalMeshComponent;

    UPROPERTY()
    class UAnimInstance* AnimInstance;

    UPROPERTY()
    class ACharacter* OwnerCharacter;

private:
    // Internal Animation Management
    void InitializeTribalAnimations();
    void UpdateMovementParameters(float DeltaTime);
    void CalculateMotionData(float DeltaTime);
    void PerformFootIK();
    
    // Animation Blending
    float CalculateAnimationScore(class UAnimMontage* Animation);
    void SmoothTransition(EAnim_MovementState NewState);
    
    // Tribal Behavior Integration
    void UpdateTribalBehaviorAnimations();
    void HandleRoleSpecificAnimations();
};