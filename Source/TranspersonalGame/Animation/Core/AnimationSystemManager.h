#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Components/ActorComponent.h"
#include "AnimationSystemManager.generated.h"

UENUM(BlueprintType)
enum class ECharacterMovementState : uint8
{
    Idle,
    Walking,
    Running,
    Crouching,
    Crawling,
    Climbing,
    Swimming,
    Falling,
    Landing,
    Hiding,
    Injured,
    Dying
};

UENUM(BlueprintType)
enum class EDinosaurBehaviorState : uint8
{
    Idle,
    Grazing,
    Drinking,
    Hunting,
    Stalking,
    Charging,
    Feeding,
    Sleeping,
    Patrolling,
    Fleeing,
    Fighting,
    Mating,
    Nesting
};

UENUM(BlueprintType)
enum class EAnimationPriority : uint8
{
    Low = 0,
    Normal = 1,
    High = 2,
    Critical = 3
};

USTRUCT(BlueprintType)
struct FCharacterAnimationProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Profile")
    FString CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Profile")
    float MovementSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Profile")
    float StepLength;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Profile")
    float BodyWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Profile")
    bool bIsInjured;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Profile")
    float FearLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Profile")
    float ExhaustionLevel;

    FCharacterAnimationProfile()
    {
        CharacterName = TEXT("Default");
        MovementSpeed = 1.0f;
        StepLength = 1.0f;
        BodyWeight = 1.0f;
        bIsInjured = false;
        FearLevel = 0.0f;
        ExhaustionLevel = 0.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnimationSystemManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnimationSystemManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Character Animation Management
    UFUNCTION(BlueprintCallable, Category = "Animation System")
    void SetCharacterMovementState(ECharacterMovementState NewState);

    UFUNCTION(BlueprintCallable, Category = "Animation System")
    ECharacterMovementState GetCharacterMovementState() const { return CurrentMovementState; }

    UFUNCTION(BlueprintCallable, Category = "Animation System")
    void UpdateCharacterProfile(const FCharacterAnimationProfile& NewProfile);

    // Dinosaur Animation Management
    UFUNCTION(BlueprintCallable, Category = "Animation System")
    void SetDinosaurBehaviorState(EDinosaurBehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "Animation System")
    EDinosaurBehaviorState GetDinosaurBehaviorState() const { return CurrentBehaviorState; }

    // Motion Matching Integration
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void InitializeMotionMatching();

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void UpdateMotionMatchingQuery(FVector Velocity, FVector Acceleration, float TurnRate);

    // IK System Integration
    UFUNCTION(BlueprintCallable, Category = "IK System")
    void EnableFootIK(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "IK System")
    void UpdateFootIKTargets(FVector LeftFootTarget, FVector RightFootTarget);

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation State")
    ECharacterMovementState CurrentMovementState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation State")
    EDinosaurBehaviorState CurrentBehaviorState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Profile")
    FCharacterAnimationProfile CharacterProfile;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    bool bUseMotionMatching;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK System")
    bool bUseFootIK;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK System")
    float FootIKInterpSpeed;

private:
    void InitializeAnimationSystems();
    void UpdateAnimationBlending(float DeltaTime);
    void ProcessMovementStateTransitions();
};