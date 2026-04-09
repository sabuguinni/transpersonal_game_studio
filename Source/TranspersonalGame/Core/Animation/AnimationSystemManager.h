#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimBlueprint.h"
#include "Animation/AnimSequence.h"
#include "Animation/BlendSpace.h"
#include "Animation/PoseAsset.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "Engine/DataTable.h"
#include "AnimationSystemManager.generated.h"

USTRUCT(BlueprintType)
struct FCharacterAnimationProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Profile")
    FString CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Profile")
    TSubclassOf<UAnimInstance> AnimBlueprintClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion")
    UBlendSpace* LocomotionBlendSpace;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion")
    UAnimSequence* IdleAnimation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion")
    UAnimSequence* WalkAnimation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion")
    UAnimSequence* RunAnimation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    TArray<UAnimSequence*> CombatAnimations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    TArray<UAnimSequence*> InteractionAnimations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional")
    TArray<UAnimSequence*> EmotionalAnimations;

    FCharacterAnimationProfile()
    {
        CharacterName = TEXT("Default");
        AnimBlueprintClass = nullptr;
        LocomotionBlendSpace = nullptr;
        IdleAnimation = nullptr;
        WalkAnimation = nullptr;
        RunAnimation = nullptr;
    }
};

USTRUCT(BlueprintType)
struct FMotionMatchingConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    bool bUseMotionMatching;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float BlendTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float SearchRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    int32 MaxCandidates;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    TArray<FString> FeatureChannels;

    FMotionMatchingConfig()
    {
        bUseMotionMatching = true;
        BlendTime = 0.2f;
        SearchRadius = 100.0f;
        MaxCandidates = 10;
    }
};

/**
 * Sistema central de gestão de animações para o jogo transpersonal
 * Implementa Motion Matching, IK procedural e sistemas de animação contextual
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAnimationSystemManager : public UObject
{
    GENERATED_BODY()

public:
    UAnimationSystemManager();

    // Core Animation Management
    UFUNCTION(BlueprintCallable, Category = "Animation System")
    void InitializeAnimationSystem();

    UFUNCTION(BlueprintCallable, Category = "Animation System")
    void RegisterCharacterAnimationProfile(const FCharacterAnimationProfile& Profile);

    UFUNCTION(BlueprintCallable, Category = "Animation System")
    FCharacterAnimationProfile GetAnimationProfileForCharacter(const FString& CharacterName);

    UFUNCTION(BlueprintCallable, Category = "Animation System")
    void ApplyAnimationProfileToCharacter(ACharacter* Character, const FString& ProfileName);

    // Motion Matching System
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void ConfigureMotionMatching(const FMotionMatchingConfig& Config);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void UpdateMotionMatchingDatabase(const TArray<UAnimSequence*>& Animations);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    UAnimSequence* FindBestMatchingAnimation(ACharacter* Character, const FVector& Velocity, const FVector& Acceleration);

    // IK System Management
    UFUNCTION(BlueprintCallable, Category = "IK System")
    void EnableFootIK(ACharacter* Character, bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "IK System")
    void SetIKTargets(ACharacter* Character, const FVector& LeftFootTarget, const FVector& RightFootTarget);

    UFUNCTION(BlueprintCallable, Category = "IK System")
    void UpdateTerrainAdaptation(ACharacter* Character, float DeltaTime);

    // Animation State Management
    UFUNCTION(BlueprintCallable, Category = "Animation States")
    void TransitionToAnimationState(ACharacter* Character, const FString& StateName, float BlendTime = 0.3f);

    UFUNCTION(BlueprintCallable, Category = "Animation States")
    FString GetCurrentAnimationState(ACharacter* Character);

    UFUNCTION(BlueprintCallable, Category = "Animation States")
    void RegisterAnimationStateCallback(const FString& StateName, const FString& CallbackFunction);

    // Emotional Animation System
    UFUNCTION(BlueprintCallable, Category = "Emotional Animation")
    void PlayEmotionalAnimation(ACharacter* Character, const FString& EmotionType, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Emotional Animation")
    void BlendEmotionalLayers(ACharacter* Character, const TMap<FString, float>& EmotionWeights);

    UFUNCTION(BlueprintCallable, Category = "Emotional Animation")
    void SetCharacterPersonality(ACharacter* Character, const FString& PersonalityType);

    // Performance and Quality Management
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetAnimationQualityLevel(int32 QualityLevel);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeAnimationsForDistance(ACharacter* Character, float DistanceToPlayer);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void EnableAnimationLOD(bool bEnable);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Profiles")
    TMap<FString, FCharacterAnimationProfile> CharacterProfiles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    FMotionMatchingConfig MotionMatchingSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Database")
    TArray<UAnimSequence*> MotionMatchingDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    bool bFootIKEnabled;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float FootIKTraceDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float FootIKInterpSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 CurrentQualityLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bAnimationLODEnabled;

private:
    // Internal animation state tracking
    TMap<ACharacter*, FString> CharacterAnimationStates;
    TMap<FString, FString> StateCallbacks;
    
    // Performance optimization
    TMap<ACharacter*, float> CharacterDistances;
    TMap<ACharacter*, int32> CharacterLODLevels;

    // Helper functions
    void InitializeDefaultProfiles();
    void SetupMotionMatchingDefaults();
    bool ValidateAnimationProfile(const FCharacterAnimationProfile& Profile);
    void ApplyPerformanceOptimizations(ACharacter* Character);
    void UpdateAnimationLOD(ACharacter* Character, float Distance);
};