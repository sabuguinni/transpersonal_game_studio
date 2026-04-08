#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "AnimationSystemCore.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogAnimationSystem, Log, All);

/**
 * Core Animation System for Transpersonal Game
 * Handles Motion Matching, IK, and character-specific animation logic
 * Built around the principle: every movement tells a story
 */

UENUM(BlueprintType)
enum class ECharacterEmotionalState : uint8
{
    Calm        UMETA(DisplayName = "Calm"),
    Alert       UMETA(DisplayName = "Alert"), 
    Afraid      UMETA(DisplayName = "Afraid"),
    Terrified   UMETA(DisplayName = "Terrified"),
    Exhausted   UMETA(DisplayName = "Exhausted"),
    Injured     UMETA(DisplayName = "Injured")
};

UENUM(BlueprintType)
enum class EMovementContext : uint8
{
    Exploration     UMETA(DisplayName = "Exploration"),
    Stealth         UMETA(DisplayName = "Stealth"),
    Escape          UMETA(DisplayName = "Escape"),
    Interaction     UMETA(DisplayName = "Interaction"),
    Combat          UMETA(DisplayName = "Combat"),
    Observation     UMETA(DisplayName = "Observation")
};

USTRUCT(BlueprintType)
struct FCharacterAnimationProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Profile")
    FString CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Profile")
    float FearThreshold = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Profile")
    float ExhaustionRate = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Profile")
    float RecoveryRate = 0.05f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Profile")
    TMap<EMovementContext, float> ContextualSpeedModifiers;

    FCharacterAnimationProfile()
    {
        ContextualSpeedModifiers.Add(EMovementContext::Exploration, 1.0f);
        ContextualSpeedModifiers.Add(EMovementContext::Stealth, 0.3f);
        ContextualSpeedModifiers.Add(EMovementContext::Escape, 1.5f);
        ContextualSpeedModifiers.Add(EMovementContext::Interaction, 0.8f);
        ContextualSpeedModifiers.Add(EMovementContext::Combat, 1.2f);
        ContextualSpeedModifiers.Add(EMovementContext::Observation, 0.1f);
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAnimationSystemCore : public UObject
{
    GENERATED_BODY()

public:
    UAnimationSystemCore();

    // Core system functions
    UFUNCTION(BlueprintCallable, Category = "Animation System")
    void InitializeForCharacter(class ACharacter* Character);

    UFUNCTION(BlueprintCallable, Category = "Animation System")
    void UpdateAnimationContext(EMovementContext NewContext, ECharacterEmotionalState EmotionalState);

    UFUNCTION(BlueprintCallable, Category = "Animation System")
    float CalculateBlendTime(EMovementContext FromContext, EMovementContext ToContext);

    UFUNCTION(BlueprintCallable, Category = "Animation System")
    FVector CalculateFootPlantingOffset(const FVector& GroundNormal, const FVector& FootLocation);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Profiles")
    TMap<FString, FCharacterAnimationProfile> CharacterProfiles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    class UPoseSearchDatabase* LocomotionDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching") 
    class UPoseSearchDatabase* StealthDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    class UPoseSearchDatabase* EscapeDatabase;

private:
    EMovementContext CurrentContext;
    ECharacterEmotionalState CurrentEmotionalState;
    float CurrentFearLevel;
    float CurrentExhaustion;
};