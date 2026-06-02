#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/Engine.h"
#include "Animation/BlendSpace.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "Anim_BlendSpaceManager.generated.h"

UENUM(BlueprintType)
enum class EAnim_LocomotionState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walking     UMETA(DisplayName = "Walking"), 
    Running     UMETA(DisplayName = "Running"),
    Sprinting   UMETA(DisplayName = "Sprinting"),
    Crouching   UMETA(DisplayName = "Crouching"),
    Crawling    UMETA(DisplayName = "Crawling")
};

UENUM(BlueprintType)
enum class EAnim_CombatState : uint8
{
    Unarmed     UMETA(DisplayName = "Unarmed"),
    SpearReady  UMETA(DisplayName = "Spear Ready"),
    BowReady    UMETA(DisplayName = "Bow Ready"),
    ClubReady   UMETA(DisplayName = "Club Ready"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Blocking    UMETA(DisplayName = "Blocking"),
    Dodging     UMETA(DisplayName = "Dodging")
};

UENUM(BlueprintType)
enum class EAnim_SurvivalState : uint8
{
    None        UMETA(DisplayName = "None"),
    Gathering   UMETA(DisplayName = "Gathering"),
    Crafting    UMETA(DisplayName = "Crafting"),
    Building    UMETA(DisplayName = "Building"),
    Cooking     UMETA(DisplayName = "Cooking"),
    Healing     UMETA(DisplayName = "Healing"),
    Resting     UMETA(DisplayName = "Resting")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_BlendSpaceData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    UBlendSpace* BlendSpace;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    float XAxisValue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    float YAxisValue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    float BlendWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    bool bIsActive;

    FAnim_BlendSpaceData()
    {
        BlendSpace = nullptr;
        XAxisValue = 0.0f;
        YAxisValue = 0.0f;
        BlendWeight = 1.0f;
        bIsActive = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MontagePlayback
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    UAnimMontage* Montage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    float PlayRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    float StartPosition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    FName StartSection;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    bool bLooping;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    int32 Priority;

    FAnim_MontagePlayback()
    {
        Montage = nullptr;
        PlayRate = 1.0f;
        StartPosition = 0.0f;
        StartSection = NAME_None;
        bLooping = false;
        Priority = 0;
    }
};

/**
 * Advanced blend space management system for tribal character animations
 * Handles locomotion, combat, and survival animation states with smooth transitions
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAnim_BlendSpaceManager : public UObject
{
    GENERATED_BODY()

public:
    UAnim_BlendSpaceManager();

    // Core blend space management
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void InitializeBlendSpaces(USkeletalMeshComponent* TargetMesh);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateLocomotionBlendSpace(float Speed, float Direction, bool bIsCrouching);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateCombatBlendSpace(EAnim_CombatState CombatState, float AttackDirection);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateSurvivalBlendSpace(EAnim_SurvivalState SurvivalState, float ActionProgress);

    // Montage playback system
    UFUNCTION(BlueprintCallable, Category = "Animation")
    bool PlayAnimationMontage(UAnimMontage* Montage, float PlayRate = 1.0f, FName StartSection = NAME_None);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void StopAnimationMontage(UAnimMontage* Montage, float BlendOutTime = 0.25f);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void StopAllMontages(float BlendOutTime = 0.25f);

    // State management
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetLocomotionState(EAnim_LocomotionState NewState);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetCombatState(EAnim_CombatState NewState);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetSurvivalState(EAnim_SurvivalState NewState);

    // Getters
    UFUNCTION(BlueprintPure, Category = "Animation")
    EAnim_LocomotionState GetLocomotionState() const { return CurrentLocomotionState; }

    UFUNCTION(BlueprintPure, Category = "Animation")
    EAnim_CombatState GetCombatState() const { return CurrentCombatState; }

    UFUNCTION(BlueprintPure, Category = "Animation")
    EAnim_SurvivalState GetSurvivalState() const { return CurrentSurvivalState; }

    UFUNCTION(BlueprintPure, Category = "Animation")
    bool IsPlayingMontage(UAnimMontage* Montage) const;

    UFUNCTION(BlueprintPure, Category = "Animation")
    float GetMontagePosition(UAnimMontage* Montage) const;

    // Blend space configuration
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetBlendSpaceWeight(UBlendSpace* BlendSpace, float Weight);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetBlendSpaceParameters(UBlendSpace* BlendSpace, float XValue, float YValue);

    // Advanced features
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void EnableRootMotion(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetAnimationSpeed(float SpeedMultiplier);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void BlendToState(EAnim_LocomotionState TargetState, float BlendTime = 0.5f);

protected:
    // Core references
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation")
    USkeletalMeshComponent* OwnerMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation")
    UAnimInstance* AnimInstance;

    // Current states
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation")
    EAnim_LocomotionState CurrentLocomotionState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation")
    EAnim_CombatState CurrentCombatState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation")
    EAnim_SurvivalState CurrentSurvivalState;

    // Blend space assets
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation Assets")
    UBlendSpace* LocomotionBlendSpace;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation Assets")
    UBlendSpace* CombatBlendSpace;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation Assets")
    UBlendSpace* SurvivalBlendSpace;

    // Montage assets
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation Assets")
    TMap<EAnim_CombatState, UAnimMontage*> CombatMontages;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation Assets")
    TMap<EAnim_SurvivalState, UAnimMontage*> SurvivalMontages;

    // Blend space data
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation Data")
    TMap<UBlendSpace*, FAnim_BlendSpaceData> BlendSpaceData;

    // Active montages
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation Data")
    TArray<FAnim_MontagePlayback> ActiveMontages;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation Config")
    float DefaultBlendTime;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation Config")
    float MaxLocomotionSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation Config")
    bool bUseRootMotion;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation Config")
    float AnimationSpeedMultiplier;

private:
    // Internal helpers
    void UpdateBlendSpaceInternal(UBlendSpace* BlendSpace, float XValue, float YValue, float Weight);
    void CleanupFinishedMontages();
    bool ValidateAnimationAssets() const;
    void LoadDefaultAnimationAssets();
};