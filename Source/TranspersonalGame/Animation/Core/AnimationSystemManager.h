#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/ActorComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "AnimationSystemManager.generated.h"

// Forward declarations
class UAnimationBlueprint;
class UPoseSearchDatabase;
class UIKRigDefinition;
class UControlRig;

UENUM(BlueprintType)
enum class EAnim_MovementType : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Walking         UMETA(DisplayName = "Walking"),
    Running         UMETA(DisplayName = "Running"),
    Sprinting       UMETA(DisplayName = "Sprinting"),
    Crouching       UMETA(DisplayName = "Crouching"),
    Crawling        UMETA(DisplayName = "Crawling"),
    Swimming        UMETA(DisplayName = "Swimming"),
    Climbing        UMETA(DisplayName = "Climbing")
};

UENUM(BlueprintType)
enum class EAnim_CombatState : uint8
{
    None            UMETA(DisplayName = "None"),
    CombatIdle      UMETA(DisplayName = "Combat Idle"),
    Attacking       UMETA(DisplayName = "Attacking"),
    Blocking        UMETA(DisplayName = "Blocking"),
    Dodging         UMETA(DisplayName = "Dodging"),
    Stunned         UMETA(DisplayName = "Stunned"),
    Dead            UMETA(DisplayName = "Dead")
};

UENUM(BlueprintType)
enum class EAnim_InteractionType : uint8
{
    None            UMETA(DisplayName = "None"),
    Gathering       UMETA(DisplayName = "Gathering"),
    Crafting        UMETA(DisplayName = "Crafting"),
    Eating          UMETA(DisplayName = "Eating"),
    Drinking        UMETA(DisplayName = "Drinking"),
    Sleeping        UMETA(DisplayName = "Sleeping"),
    Storytelling    UMETA(DisplayName = "Storytelling")
};

USTRUCT(BlueprintType)
struct FAnim_MotionMatchingData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float Velocity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    FVector MovementDirection;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float AngularVelocity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    FVector FuturePosition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    FVector FutureVelocity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    bool bIsInCombat;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    EAnim_MovementType MovementType;

    FAnim_MotionMatchingData()
    {
        Velocity = 0.0f;
        MovementDirection = FVector::ZeroVector;
        AngularVelocity = 0.0f;
        FuturePosition = FVector::ZeroVector;
        FutureVelocity = FVector::ZeroVector;
        bIsInCombat = false;
        MovementType = EAnim_MovementType::Idle;
    }
};

USTRUCT(BlueprintType)
struct FAnim_IKFootData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    FVector FootLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    FRotator FootRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    float FootOffset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    bool bIsGrounded;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    float GroundDistance;

    FAnim_IKFootData()
    {
        FootLocation = FVector::ZeroVector;
        FootRotation = FRotator::ZeroRotator;
        FootOffset = 0.0f;
        bIsGrounded = true;
        GroundDistance = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FAnim_EmotionalState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotion", meta = (ClampMin = "-1.0", ClampMax = "1.0"))
    float Arousal; // -1 (calm) to 1 (excited)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotion", meta = (ClampMin = "-1.0", ClampMax = "1.0"))
    float Valence; // -1 (negative) to 1 (positive)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotion", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Intensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotion")
    float Duration;

    FAnim_EmotionalState()
    {
        Arousal = 0.0f;
        Valence = 0.0f;
        Intensity = 0.0f;
        Duration = 0.0f;
    }
};

/**
 * Core animation system manager that handles Motion Matching, IK, and procedural animations
 * for prehistoric characters. Provides realistic movement with terrain adaptation.
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnimationSystemManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnimationSystemManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // === MOTION MATCHING SYSTEM ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    TObjectPtr<UPoseSearchDatabase> LocomotionDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    TObjectPtr<UPoseSearchDatabase> CombatDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    TObjectPtr<UPoseSearchDatabase> InteractionDatabase;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Motion Matching")
    FAnim_MotionMatchingData CurrentMotionData;

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void UpdateMotionMatchingData(const FVector& Velocity, const FVector& AngularVelocity, bool bInCombat);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void SetMovementType(EAnim_MovementType NewMovementType);

    // === IK SYSTEM ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    TObjectPtr<UIKRigDefinition> CharacterIKRig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    TObjectPtr<UControlRig> FootIKControlRig;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "IK")
    FAnim_IKFootData LeftFootData;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "IK")
    FAnim_IKFootData RightFootData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    float IKTraceDistance = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    float FootOffsetInterpSpeed = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    float PelvisOffsetInterpSpeed = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    float MaxFootOffset = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    float MinFootOffset = -20.0f;

    UFUNCTION(BlueprintCallable, Category = "IK")
    void UpdateFootIK(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "IK")
    FAnim_IKFootData TraceFootPlacement(const FVector& FootLocation, const FName& SocketName);

    // === ANIMATION MONTAGES ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montages")
    TMap<FString, TObjectPtr<UAnimMontage>> CombatMontages;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montages")
    TMap<FString, TObjectPtr<UAnimMontage>> SurvivalMontages;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montages")
    TMap<FString, TObjectPtr<UAnimMontage>> SocialMontages;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montages")
    TMap<FString, TObjectPtr<UAnimMontage>> EnvironmentalMontages;

    UFUNCTION(BlueprintCallable, Category = "Montages")
    bool PlayAnimationMontage(const FString& MontageName, const FString& Category, float PlayRate = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Montages")
    void StopAnimationMontage(const FString& MontageName);

    // === EMOTIONAL ANIMATION ===

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Emotion")
    FAnim_EmotionalState CurrentEmotionalState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotion")
    TObjectPtr<UBlendSpace> EmotionBlendSpace;

    UFUNCTION(BlueprintCallable, Category = "Emotion")
    void SetEmotionalState(float Arousal, float Valence, float Intensity, float Duration = 5.0f);

    UFUNCTION(BlueprintCallable, Category = "Emotion")
    void UpdateEmotionalAnimation(float DeltaTime);

    // === PROCEDURAL ANIMATION ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural")
    bool bEnableBreathingAnimation = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural")
    bool bEnableIdleVariations = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural")
    bool bEnablePhysicsSecondary = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural")
    float BreathingRate = 1.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural")
    float IdleVariationFrequency = 8.0f;

    UFUNCTION(BlueprintCallable, Category = "Procedural")
    void UpdateProceduralAnimations(float DeltaTime);

    // === UTILITY FUNCTIONS ===

    UFUNCTION(BlueprintCallable, Category = "Animation System")
    void InitializeAnimationSystem(USkeletalMeshComponent* SkeletalMeshComp);

    UFUNCTION(BlueprintCallable, Category = "Animation System")
    void SetCombatState(EAnim_CombatState NewCombatState);

    UFUNCTION(BlueprintCallable, Category = "Animation System")
    void SetInteractionType(EAnim_InteractionType NewInteractionType);

    UFUNCTION(BlueprintPure, Category = "Animation System")
    bool IsInCombat() const { return CurrentCombatState != EAnim_CombatState::None; }

    UFUNCTION(BlueprintPure, Category = "Animation System")
    bool IsInteracting() const { return CurrentInteractionType != EAnim_InteractionType::None; }

private:
    UPROPERTY()
    TObjectPtr<USkeletalMeshComponent> OwnerSkeletalMesh;

    UPROPERTY()
    EAnim_CombatState CurrentCombatState;

    UPROPERTY()
    EAnim_InteractionType CurrentInteractionType;

    // Internal timers
    float EmotionTimer;
    float BreathingTimer;
    float IdleVariationTimer;

    // Internal helper functions
    void UpdateMotionPrediction(float DeltaTime);
    void ProcessEmotionalTransitions(float DeltaTime);
    FVector PredictFuturePosition(float PredictionTime) const;
    FVector PredictFutureVelocity(float PredictionTime) const;
};