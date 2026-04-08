#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Animation/AnimInstance.h"
#include "PoseSearch/PoseSearchDatabase.h"
#include "PoseSearch/PoseSearchSchema.h"
#include "AnimationSystemManager.generated.h"

/**
 * Central manager for the animation system
 * Handles Motion Matching databases, IK systems, and character-specific animation logic
 * Based on principles from RDR2's character animation system
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAnimationSystemManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnimationSystemManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Motion Matching Database Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    TMap<FString, TSoftObjectPtr<UPoseSearchDatabase>> MotionDatabases;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    TSoftObjectPtr<UPoseSearchSchema> DefaultLocomotionSchema;

    // Character Animation States
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character States")
    TMap<FString, float> CharacterEmotionalStates;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character States")
    float FearLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character States")
    float ExhaustionLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character States")
    float ConfidenceLevel;

    // Terrain Adaptation
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK System")
    bool bEnableFootIK;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK System")
    float FootIKInterpSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK System")
    float MaxFootIKOffset;

    // Animation Quality Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quality")
    int32 AnimationLODLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quality")
    bool bUseHighQualityMotionMatching;

    // Functions
    UFUNCTION(BlueprintCallable, Category = "Animation System")
    void SetCharacterEmotionalState(const FString& StateName, float Value);

    UFUNCTION(BlueprintCallable, Category = "Animation System")
    float GetCharacterEmotionalState(const FString& StateName) const;

    UFUNCTION(BlueprintCallable, Category = "Animation System")
    void UpdateFearLevel(float NewFearLevel);

    UFUNCTION(BlueprintCallable, Category = "Animation System")
    UPoseSearchDatabase* GetMotionDatabaseForContext(const FString& Context) const;

    UFUNCTION(BlueprintCallable, Category = "Animation System")
    void InitializeCharacterAnimationProfile(const FString& CharacterType);

private:
    void LoadDefaultAnimationDatabases();
    void SetupCharacterEmotionalDefaults();
};