#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "PoseSearch/PoseSearchDatabase.h"
#include "PoseSearch/PoseSearchSchema.h"
#include "Components/SkeletalMeshComponent.h"
#include "AnimationSystemCore.generated.h"

/**
 * Core Animation System for Transpersonal Game
 * Handles Motion Matching, IK systems, and character-specific animations
 * Based on RDR2 principles: every movement tells a story
 */

UENUM(BlueprintType)
enum class ECharacterArchetype : uint8
{
    PlayerPaleontologist    UMETA(DisplayName = "Player - Paleontologist"),
    DinosaurHerbivore      UMETA(DisplayName = "Dinosaur - Herbivore"),
    DinosaurCarnivore      UMETA(DisplayName = "Dinosaur - Carnivore"),
    DinosaurFlying         UMETA(DisplayName = "Dinosaur - Flying"),
    DinosaurAquatic        UMETA(DisplayName = "Dinosaur - Aquatic")
};

UENUM(BlueprintType)
enum class EMovementState : uint8
{
    Idle                   UMETA(DisplayName = "Idle"),
    Walking                UMETA(DisplayName = "Walking"),
    Running                UMETA(DisplayName = "Running"),
    Sprinting              UMETA(DisplayName = "Sprinting"),
    Crouching              UMETA(DisplayName = "Crouching"),
    Crawling               UMETA(DisplayName = "Crawling"),
    Climbing               UMETA(DisplayName = "Climbing"),
    Swimming               UMETA(DisplayName = "Swimming"),
    Falling                UMETA(DisplayName = "Falling"),
    Landing                UMETA(DisplayName = "Landing"),
    Jumping                UMETA(DisplayName = "Jumping"),
    Dodging                UMETA(DisplayName = "Dodging"),
    Hiding                 UMETA(DisplayName = "Hiding"),
    Interacting            UMETA(DisplayName = "Interacting"),
    Crafting               UMETA(DisplayName = "Crafting"),
    Gathering              UMETA(DisplayName = "Gathering"),
    Combat                 UMETA(DisplayName = "Combat"),
    Injured                UMETA(DisplayName = "Injured"),
    Dying                  UMETA(DisplayName = "Dying"),
    Dead                   UMETA(DisplayName = "Dead")
};

UENUM(BlueprintType)
enum class EEmotionalState : uint8
{
    Neutral                UMETA(DisplayName = "Neutral"),
    Fearful                UMETA(DisplayName = "Fearful"),
    Terrified              UMETA(DisplayName = "Terrified"),
    Cautious               UMETA(DisplayName = "Cautious"),
    Alert                  UMETA(DisplayName = "Alert"),
    Curious                UMETA(DisplayName = "Curious"),
    Confident              UMETA(DisplayName = "Confident"),
    Exhausted              UMETA(DisplayName = "Exhausted"),
    Injured                UMETA(DisplayName = "Injured"),
    Aggressive             UMETA(DisplayName = "Aggressive"),
    Peaceful               UMETA(DisplayName = "Peaceful"),
    Domesticated           UMETA(DisplayName = "Domesticated")
};

USTRUCT(BlueprintType)
struct FCharacterAnimationProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Identity")
    ECharacterArchetype Archetype;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Identity")
    FString CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Identity")
    int32 UniqueID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float BaseMovementSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float SprintMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float CrouchSpeedMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical Traits")
    float Height;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical Traits")
    float Weight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical Traits")
    float LegLength;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    float Nervousness; // 0.0 = Calm, 1.0 = Extremely nervous

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    float Confidence; // 0.0 = No confidence, 1.0 = Very confident

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    float Aggression; // 0.0 = Peaceful, 1.0 = Highly aggressive

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Databases")
    TObjectPtr<UPoseSearchDatabase> LocomotionDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Databases")
    TObjectPtr<UPoseSearchDatabase> InteractionDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Databases")
    TObjectPtr<UPoseSearchDatabase> CombatDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Databases")
    TObjectPtr<UPoseSearchDatabase> EmotionalDatabase;

    FCharacterAnimationProfile()
    {
        Archetype = ECharacterArchetype::PlayerPaleontologist;
        CharacterName = TEXT("Unknown");
        UniqueID = 0;
        BaseMovementSpeed = 400.0f;
        SprintMultiplier = 2.0f;
        CrouchSpeedMultiplier = 0.5f;
        Height = 180.0f;
        Weight = 75.0f;
        LegLength = 90.0f;
        Nervousness = 0.5f;
        Confidence = 0.5f;
        Aggression = 0.1f;
        LocomotionDatabase = nullptr;
        InteractionDatabase = nullptr;
        CombatDatabase = nullptr;
        EmotionalDatabase = nullptr;
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
    void InitializeCharacterAnimation(AActor* Character, const FCharacterAnimationProfile& Profile);

    UFUNCTION(BlueprintCallable, Category = "Animation System")
    void UpdateMotionMatchingQuery(AActor* Character, FVector Velocity, EMovementState State, EEmotionalState Emotion);

    UFUNCTION(BlueprintCallable, Category = "Animation System")
    void ApplyFootIK(USkeletalMeshComponent* SkeletalMesh, float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Animation System")
    FCharacterAnimationProfile GetCharacterProfile(int32 UniqueID);

    UFUNCTION(BlueprintCallable, Category = "Animation System")
    void RegisterCharacterProfile(const FCharacterAnimationProfile& Profile);

    // Motion Matching utilities
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    UPoseSearchDatabase* SelectOptimalDatabase(ECharacterArchetype Archetype, EMovementState State, EEmotionalState Emotion);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    float CalculateBlendTime(EMovementState FromState, EMovementState ToState, EEmotionalState Emotion);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Profiles")
    TMap<int32, FCharacterAnimationProfile> RegisteredProfiles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    TObjectPtr<UPoseSearchSchema> DefaultLocomotionSchema;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    TObjectPtr<UPoseSearchSchema> DefaultInteractionSchema;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK System")
    float FootIKTraceDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK System")
    float FootIKInterpSpeed;

private:
    // Internal animation logic
    void SetupMotionMatchingForCharacter(AActor* Character, const FCharacterAnimationProfile& Profile);
    void ConfigureIKRigForCharacter(USkeletalMeshComponent* SkeletalMesh, ECharacterArchetype Archetype);
    
    // Foot IK implementation
    FVector PerformFootTrace(USkeletalMeshComponent* SkeletalMesh, FName FootBoneName, FVector FootLocation);
    void ApplyFootIKToLimb(USkeletalMeshComponent* SkeletalMesh, FName FootBone, FName KneeBone, FVector TargetLocation, float Alpha);
};