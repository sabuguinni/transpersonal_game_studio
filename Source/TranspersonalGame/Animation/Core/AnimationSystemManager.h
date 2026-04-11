#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/ActorComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "PoseSearch/PoseSearchDatabase.h"
#include "AnimationSystemManager.generated.h"

class UMotionMatchingComponent;
class UFootIKComponent;
class UCharacterMovementComponent;

/**
 * Core animation system manager for Transpersonal Game
 * Handles Motion Matching, procedural IK, and character animation coordination
 * Based on RDR2-style character movement with prehistoric authenticity
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(Animation))
class TRANSPERSONALGAME_API UAnimationSystemManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnimationSystemManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Motion Matching System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    class UPoseSearchDatabase* LocomotionDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    class UPoseSearchDatabase* CombatDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    class UPoseSearchDatabase* InteractionDatabase;

    // IK System Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK System")
    bool bEnableFootIK = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK System")
    bool bEnableHandIK = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK System")
    float FootIKRange = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK System")
    float HandIKRange = 30.0f;

    // Animation State Management
    UPROPERTY(BlueprintReadOnly, Category = "Animation State")
    bool bIsInCombat = false;

    UPROPERTY(BlueprintReadOnly, Category = "Animation State")
    bool bIsClimbing = false;

    UPROPERTY(BlueprintReadOnly, Category = "Animation State")
    bool bIsSwimming = false;

    UPROPERTY(BlueprintReadOnly, Category = "Animation State")
    bool bIsCrouching = false;

    // Character Personality Traits (affects animation style)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Personality")
    float ConfidenceLevel = 0.5f; // 0.0 = timid, 1.0 = confident

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Personality")
    float AgilityLevel = 0.5f; // 0.0 = clumsy, 1.0 = graceful

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Personality")
    float StrengthLevel = 0.5f; // 0.0 = weak, 1.0 = powerful

    // Animation Blending
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Blending")
    float BlendInTime = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Blending")
    float BlendOutTime = 0.2f;

public:
    // Core Animation Functions
    UFUNCTION(BlueprintCallable, Category = "Animation System")
    void InitializeAnimationSystem();

    UFUNCTION(BlueprintCallable, Category = "Animation System")
    void UpdateAnimationState(float DeltaTime);

    // Motion Matching Functions
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void SetMotionMatchingDatabase(class UPoseSearchDatabase* Database);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    FVector GetDesiredVelocity() const;

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    float GetMovementSpeed() const;

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    FVector GetMovementDirection() const;

    // IK System Functions
    UFUNCTION(BlueprintCallable, Category = "IK System")
    void UpdateFootIK(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "IK System")
    void UpdateHandIK(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "IK System")
    FVector GetFootIKOffset(bool bIsLeftFoot) const;

    UFUNCTION(BlueprintCallable, Category = "IK System")
    FVector GetHandIKOffset(bool bIsLeftHand) const;

    // Animation State Functions
    UFUNCTION(BlueprintCallable, Category = "Animation State")
    void SetCombatMode(bool bInCombat);

    UFUNCTION(BlueprintCallable, Category = "Animation State")
    void SetClimbingMode(bool bClimbing);

    UFUNCTION(BlueprintCallable, Category = "Animation State")
    void SetSwimmingMode(bool bSwimming);

    UFUNCTION(BlueprintCallable, Category = "Animation State")
    void SetCrouchingMode(bool bCrouching);

    // Character Personality Functions
    UFUNCTION(BlueprintCallable, Category = "Character Personality")
    void SetCharacterPersonality(float Confidence, float Agility, float Strength);

    UFUNCTION(BlueprintCallable, Category = "Character Personality")
    float GetPersonalityBlendWeight(const FString& AnimationType) const;

    // Utility Functions
    UFUNCTION(BlueprintCallable, Category = "Animation System")
    class UAnimInstance* GetAnimInstance() const;

    UFUNCTION(BlueprintCallable, Category = "Animation System")
    class USkeletalMeshComponent* GetSkeletalMeshComponent() const;

    UFUNCTION(BlueprintCallable, Category = "Animation System")
    class UCharacterMovementComponent* GetCharacterMovement() const;

private:
    // Internal state tracking
    FVector LastVelocity;
    FVector LastPosition;
    float LastUpdateTime;

    // Component references
    UPROPERTY()
    class UMotionMatchingComponent* MotionMatchingComponent;

    UPROPERTY()
    class UFootIKComponent* FootIKComponent;

    UPROPERTY()
    class USkeletalMeshComponent* SkeletalMeshComponent;

    UPROPERTY()
    class UCharacterMovementComponent* CharacterMovementComponent;

    // Internal helper functions
    void CacheComponentReferences();
    void UpdateMovementData(float DeltaTime);
    float CalculatePersonalityInfluence(const FString& AnimationType) const;
    void ValidateAnimationDatabases();
};