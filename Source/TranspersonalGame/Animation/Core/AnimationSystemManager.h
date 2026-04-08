#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "PoseSearch/PoseSearch.h"
#include "Components/ActorComponent.h"
#include "AnimationSystemManager.generated.h"

UENUM(BlueprintType)
enum class ECharacterType : uint8
{
    Player          UMETA(DisplayName = "Player Character"),
    DinosaurSmall   UMETA(DisplayName = "Small Dinosaur"),
    DinosaurMedium  UMETA(DisplayName = "Medium Dinosaur"),
    DinosaurLarge   UMETA(DisplayName = "Large Dinosaur"),
    DinosaurFlying  UMETA(DisplayName = "Flying Dinosaur")
};

UENUM(BlueprintType)
enum class EMovementState : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Walking         UMETA(DisplayName = "Walking"),
    Running         UMETA(DisplayName = "Running"),
    Sneaking        UMETA(DisplayName = "Sneaking"),
    Climbing        UMETA(DisplayName = "Climbing"),
    Swimming        UMETA(DisplayName = "Swimming"),
    Falling         UMETA(DisplayName = "Falling"),
    Landing         UMETA(DisplayName = "Landing"),
    Jumping         UMETA(DisplayName = "Jumping"),
    Crouching       UMETA(DisplayName = "Crouching"),
    Injured         UMETA(DisplayName = "Injured"),
    Exhausted       UMETA(DisplayName = "Exhausted"),
    Afraid          UMETA(DisplayName = "Afraid"),
    Alert           UMETA(DisplayName = "Alert")
};

UENUM(BlueprintType)
enum class EEmotionalState : uint8
{
    Neutral         UMETA(DisplayName = "Neutral"),
    Fearful         UMETA(DisplayName = "Fearful"),
    Cautious        UMETA(DisplayName = "Cautious"),
    Confident       UMETA(DisplayName = "Confident"),
    Exhausted       UMETA(DisplayName = "Exhausted"),
    Injured         UMETA(DisplayName = "Injured"),
    Curious         UMETA(DisplayName = "Curious"),
    Aggressive      UMETA(DisplayName = "Aggressive")
};

USTRUCT(BlueprintType)
struct FAnimationProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ECharacterType CharacterType = ECharacterType::Player;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MovementSpeed = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AnimationIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FearLevel = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ConfidenceLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float InjuryLevel = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ExhaustionLevel = 0.0f;
};

/**
 * Sistema central de gestão de animações para o jogo pré-histórico
 * Implementa Motion Matching, IK de pés e estados emocionais
 */
UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnimationSystemManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnimationSystemManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, 
                              FActorComponentTickFunction* ThisTickFunction) override;

    // Motion Matching Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    class UPoseSearchDatabase* PlayerLocomotionDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    class UPoseSearchDatabase* DinosaurLocomotionDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    class UPoseSearchSchema* DefaultSchema;

    // IK Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK System")
    float FootIKRange = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK System")
    float FootIKInterpSpeed = 15.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK System")
    bool bEnableFootIK = true;

    // Animation Profiles
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Profiles")
    TMap<ECharacterType, FAnimationProfile> CharacterProfiles;

    // Current State
    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    EMovementState CurrentMovementState = EMovementState::Idle;

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    EEmotionalState CurrentEmotionalState = EEmotionalState::Neutral;

    // Animation Functions
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetMovementState(EMovementState NewState);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetEmotionalState(EEmotionalState NewState);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateAnimationProfile(const FAnimationProfile& NewProfile);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    FAnimationProfile GetCurrentAnimationProfile() const;

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void TriggerFearResponse(float FearIntensity, float Duration);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void TriggerInjuryAnimation(float InjurySeverity);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateExhaustion(float ExhaustionLevel);

private:
    FAnimationProfile CurrentProfile;
    float FearResponseTimer = 0.0f;
    float FearResponseDuration = 0.0f;
    float OriginalFearLevel = 0.0f;

    void UpdateFearResponse(float DeltaTime);
    void ApplyEmotionalStateToProfile();
};