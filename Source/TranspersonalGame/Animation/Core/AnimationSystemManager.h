#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "PoseSearch/PoseSearchDatabase.h"
#include "AnimationSystemManager.generated.h"

UENUM(BlueprintType)
enum class ECharacterEmotionalState : uint8
{
    Calm = 0,
    Cautious,
    Fearful,
    Panicked,
    Confident,
    Aggressive
};

UENUM(BlueprintType)
enum class ECreaturePersonality : uint8
{
    Timid = 0,
    Curious,
    Aggressive,
    Protective,
    Playful,
    Territorial
};

USTRUCT(BlueprintType)
struct FCharacterAnimationProfile
{
    GENERATED_BODY()

    // Base locomotion parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float WalkSpeed = 150.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float RunSpeed = 400.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CrouchSpeed = 80.0f;
    
    // Emotional modifiers
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FearSpeedMultiplier = 1.3f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ConfidencePostureOffset = 0.1f;
    
    // Individual variation parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float GaitVariation = 0.15f; // 15% variation in step timing
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PosturalTension = 0.5f; // How tense the character naturally is
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ReactionSpeed = 1.0f; // How quickly they respond to stimuli
};

USTRUCT(BlueprintType)
struct FDinosaurAnimationProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ECreaturePersonality Personality = ECreaturePersonality::Curious;
    
    // Physical variation parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SizeVariation = 1.0f; // 0.8 to 1.2 typical range
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LimbLengthVariation = 1.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MovementAsymmetry = 0.02f; // Slight limp or favoring
    
    // Behavioral parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TrustLevel = 0.0f; // 0 = wild, 1 = fully domesticated
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AlertnessLevel = 0.7f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SocialComfort = 0.3f; // Comfort around other creatures
};

/**
 * Central manager for all animation systems in the game
 * Handles Motion Matching databases, IK systems, and procedural variations
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
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Motion Matching Database Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    TMap<FString, TSoftObjectPtr<UPoseSearchDatabase>> MotionDatabases;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    TSoftObjectPtr<UPoseSearchDatabase> HumanLocomotionDatabase;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    TSoftObjectPtr<UPoseSearchDatabase> HumanEmotionalDatabase;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    TMap<FString, TSoftObjectPtr<UPoseSearchDatabase>> DinosaurDatabases;

    // Character Animation Profiles
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Profiles")
    FCharacterAnimationProfile PlayerProfile;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Profiles")
    TMap<FString, FDinosaurAnimationProfile> DinosaurProfiles;

    // Animation State Management
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetCharacterEmotionalState(AActor* Character, ECharacterEmotionalState NewState);
    
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateDinosaurTrustLevel(AActor* Dinosaur, float NewTrustLevel);
    
    UFUNCTION(BlueprintCallable, Category = "Animation")
    FDinosaurAnimationProfile GenerateUniqueProfile(const FString& Species, int32 IndividualSeed);
    
    // IK System Integration
    UFUNCTION(BlueprintCallable, Category = "IK")
    void EnableFootIK(AActor* Character, bool bEnable);
    
    UFUNCTION(BlueprintCallable, Category = "IK")
    void SetIKTargets(AActor* Character, const TArray<FVector>& FootTargets);

private:
    // Internal state tracking
    TMap<AActor*, ECharacterEmotionalState> CharacterStates;
    TMap<AActor*, FDinosaurAnimationProfile> RuntimeProfiles;
    
    // Performance optimization
    float LastUpdateTime = 0.0f;
    static constexpr float UPDATE_FREQUENCY = 0.1f; // 10Hz updates
    
    void UpdateCharacterAnimationState(AActor* Character, float DeltaTime);
    void ApplyProceduralVariations(AActor* Character, const FDinosaurAnimationProfile& Profile);
};