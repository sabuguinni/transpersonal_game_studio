#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/BlendSpace.h"
#include "Animation/AnimMontage.h"
#include "Engine/Engine.h"
#include "../SharedTypes.h"
#include "Anim_MotionMatchingSystem.generated.h"

UENUM(BlueprintType)
enum class EAnim_MotionState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walking     UMETA(DisplayName = "Walking"),
    Running     UMETA(DisplayName = "Running"),
    Crouching   UMETA(DisplayName = "Crouching"),
    Jumping     UMETA(DisplayName = "Jumping"),
    Climbing    UMETA(DisplayName = "Climbing"),
    Swimming    UMETA(DisplayName = "Swimming"),
    Combat      UMETA(DisplayName = "Combat"),
    Gathering   UMETA(DisplayName = "Gathering"),
    Crafting    UMETA(DisplayName = "Crafting")
};

UENUM(BlueprintType)
enum class EAnim_TribalRole : uint8
{
    Hunter      UMETA(DisplayName = "Hunter"),
    Gatherer    UMETA(DisplayName = "Gatherer"),
    Crafter     UMETA(DisplayName = "Crafter"),
    Elder       UMETA(DisplayName = "Elder"),
    Child       UMETA(DisplayName = "Child"),
    Warrior     UMETA(DisplayName = "Warrior")
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
    bool bIsInAir;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    bool bIsCrouching;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    EAnim_MotionState CurrentState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    EAnim_TribalRole TribalRole;

    FAnim_MotionData()
    {
        Speed = 0.0f;
        Direction = 0.0f;
        Velocity = FVector::ZeroVector;
        bIsInAir = false;
        bIsCrouching = false;
        CurrentState = EAnim_MotionState::Idle;
        TribalRole = EAnim_TribalRole::Hunter;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_BlendSpaceConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    class UBlendSpace* LocomotionBlendSpace;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    class UBlendSpace* CombatBlendSpace;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    class UBlendSpace* WorkBlendSpace;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    float BlendSpacePlayRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    float TransitionSpeed;

    FAnim_BlendSpaceConfig()
    {
        LocomotionBlendSpace = nullptr;
        CombatBlendSpace = nullptr;
        WorkBlendSpace = nullptr;
        BlendSpacePlayRate = 1.0f;
        TransitionSpeed = 5.0f;
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

    // Core motion matching data
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Motion Matching")
    FAnim_MotionData CurrentMotionData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    FAnim_BlendSpaceConfig BlendSpaceConfig;

    // Animation assets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    TMap<EAnim_MotionState, class UAnimMontage*> StateMontages;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    TMap<EAnim_TribalRole, class UAnimSequence*> RoleIdleAnimations;

    // Motion matching parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float MotionMatchingThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float StateTransitionTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    bool bEnableMotionMatching;

    // Character references
    UPROPERTY()
    class ACharacter* OwnerCharacter;

    UPROPERTY()
    class USkeletalMeshComponent* SkeletalMeshComponent;

    UPROPERTY()
    class UAnimInstance* AnimInstance;

public:
    // Motion matching functions
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void UpdateMotionData(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void SetMotionState(EAnim_MotionState NewState);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void SetTribalRole(EAnim_TribalRole NewRole);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    EAnim_MotionState GetCurrentMotionState() const;

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    FAnim_MotionData GetMotionData() const;

    // Animation control functions
    UFUNCTION(BlueprintCallable, Category = "Animation Control")
    void PlayMontage(EAnim_MotionState State, float PlayRate = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Animation Control")
    void StopMontage(float BlendOutTime = 0.25f);

    UFUNCTION(BlueprintCallable, Category = "Animation Control")
    void SetBlendSpaceValues(float Speed, float Direction);

    UFUNCTION(BlueprintCallable, Category = "Animation Control")
    void EnableMotionMatching(bool bEnable);

    // Tribal-specific animation functions
    UFUNCTION(BlueprintCallable, Category = "Tribal Animation")
    void PlayTribalGesture(EAnim_TribalRole Role);

    UFUNCTION(BlueprintCallable, Category = "Tribal Animation")
    void PlayWorkAnimation(EAnim_TribalRole Role);

    UFUNCTION(BlueprintCallable, Category = "Tribal Animation")
    void PlayCommunicationGesture();

protected:
    // Internal motion matching logic
    void CalculateMotionState();
    void UpdateBlendSpaces();
    void HandleStateTransitions();
    bool ShouldTransitionToState(EAnim_MotionState NewState);
    float CalculateMotionScore(const FAnim_MotionData& TargetMotion);

    // Animation utility functions
    void InitializeAnimationAssets();
    void CacheCharacterReferences();
    UAnimMontage* GetMontageForState(EAnim_MotionState State);
    UAnimSequence* GetIdleAnimationForRole(EAnim_TribalRole Role);

private:
    // Internal state tracking
    EAnim_MotionState PreviousState;
    float StateTransitionTimer;
    bool bIsTransitioning;
    
    // Performance optimization
    float LastUpdateTime;
    float UpdateFrequency;
    bool bNeedsUpdate;
};