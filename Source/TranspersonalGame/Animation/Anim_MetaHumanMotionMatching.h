#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "Engine/DataAsset.h"
#include "Anim_MetaHumanMotionMatching.generated.h"

UENUM(BlueprintType)
enum class EAnim_MetaHumanCharacterType : uint8
{
    TribalElder     UMETA(DisplayName = "Tribal Elder"),
    Hunter          UMETA(DisplayName = "Hunter"),
    Gatherer        UMETA(DisplayName = "Gatherer"),
    Crafter         UMETA(DisplayName = "Crafter"),
    Scout           UMETA(DisplayName = "Scout"),
    Player          UMETA(DisplayName = "Player Character")
};

UENUM(BlueprintType)
enum class EAnim_MotionMatchingState : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Walking         UMETA(DisplayName = "Walking"),
    Running         UMETA(DisplayName = "Running"),
    Crouching       UMETA(DisplayName = "Crouching"),
    Jumping         UMETA(DisplayName = "Jumping"),
    Climbing        UMETA(DisplayName = "Climbing"),
    Combat          UMETA(DisplayName = "Combat"),
    Gathering       UMETA(DisplayName = "Gathering"),
    Crafting        UMETA(DisplayName = "Crafting"),
    Interacting     UMETA(DisplayName = "Interacting")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MotionMatchingConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float IdleWeight = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float WalkSpeed = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float RunSpeed = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float GestureFrequency = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float BlendTime = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    bool bEnableRootMotion = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    bool bEnableIKFootPlacement = true;

    FAnim_MotionMatchingConfig()
    {
        IdleWeight = 1.0f;
        WalkSpeed = 1.0f;
        RunSpeed = 1.0f;
        GestureFrequency = 0.5f;
        BlendTime = 0.2f;
        bEnableRootMotion = true;
        bEnableIKFootPlacement = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_CharacterAnimationSet
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Set")
    TObjectPtr<UAnimMontage> IdleMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Set")
    TObjectPtr<UAnimMontage> WalkMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Set")
    TObjectPtr<UAnimMontage> RunMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Set")
    TObjectPtr<UAnimMontage> JumpMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Set")
    TObjectPtr<UAnimMontage> CombatMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Set")
    TObjectPtr<UBlendSpace> LocomotionBlendSpace;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Set")
    TObjectPtr<UBlendSpace> DirectionalBlendSpace;

    FAnim_CharacterAnimationSet()
    {
        IdleMontage = nullptr;
        WalkMontage = nullptr;
        RunMontage = nullptr;
        JumpMontage = nullptr;
        CombatMontage = nullptr;
        LocomotionBlendSpace = nullptr;
        DirectionalBlendSpace = nullptr;
    }
};

UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_MetaHumanMotionMatching : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_MetaHumanMotionMatching();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Setup")
    EAnim_MetaHumanCharacterType CharacterType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    FAnim_MotionMatchingConfig MotionConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Sets")
    FAnim_CharacterAnimationSet AnimationSet;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    EAnim_MotionMatchingState CurrentState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float CurrentSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    FVector MovementDirection;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    bool bIsInCombat;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    bool bIsInteracting;

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void SetCharacterType(EAnim_MetaHumanCharacterType NewType);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void UpdateMotionMatchingState(float Speed, FVector Direction);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void PlayAnimationMontage(EAnim_MotionMatchingState State);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void SetCombatMode(bool bInCombat);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void SetInteractionMode(bool bInteracting);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void InitializeCharacterPreset();

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    FAnim_MotionMatchingConfig GetMotionConfigForCharacterType(EAnim_MetaHumanCharacterType Type);

private:
    UPROPERTY()
    TObjectPtr<USkeletalMeshComponent> SkeletalMeshComponent;

    UPROPERTY()
    TObjectPtr<UAnimInstance> AnimInstance;

    float LastStateChangeTime;
    float GestureTimer;

    void UpdateAnimationBlending(float DeltaTime);
    void HandleGestureSystem(float DeltaTime);
    void ApplyIKFootPlacement();
    EAnim_MotionMatchingState DetermineStateFromMovement(float Speed, FVector Direction);
};