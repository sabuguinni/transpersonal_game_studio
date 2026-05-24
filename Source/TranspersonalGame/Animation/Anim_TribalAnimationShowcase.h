#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "Anim_TribalAnimationShowcase.generated.h"

UENUM(BlueprintType)
enum class EAnim_TribalAnimationType : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Walk            UMETA(DisplayName = "Walk"),
    Run             UMETA(DisplayName = "Run"),
    Sprint          UMETA(DisplayName = "Sprint"),
    Jump            UMETA(DisplayName = "Jump"),
    Crouch          UMETA(DisplayName = "Crouch"),
    CraftingAction  UMETA(DisplayName = "Crafting Action"),
    HuntingPose     UMETA(DisplayName = "Hunting Pose"),
    GatheringAction UMETA(DisplayName = "Gathering Action"),
    CombatStance    UMETA(DisplayName = "Combat Stance"),
    ThrowSpear      UMETA(DisplayName = "Throw Spear"),
    ClimbRock       UMETA(DisplayName = "Climb Rock")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_TribalAnimationData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    EAnim_TribalAnimationType AnimationType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    UAnimMontage* AnimationMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    float PlayRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    bool bLooping;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    float BlendInTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    float BlendOutTime;

    FAnim_TribalAnimationData()
    {
        AnimationType = EAnim_TribalAnimationType::Idle;
        AnimationMontage = nullptr;
        PlayRate = 1.0f;
        bLooping = false;
        BlendInTime = 0.25f;
        BlendOutTime = 0.25f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_TribalShowcaseSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Showcase")
    float AnimationDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Showcase")
    float TransitionTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Showcase")
    bool bAutoLoop;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Showcase")
    bool bRandomizeOrder;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Showcase")
    TArray<EAnim_TribalAnimationType> ShowcaseSequence;

    FAnim_TribalShowcaseSettings()
    {
        AnimationDuration = 3.0f;
        TransitionTime = 0.5f;
        bAutoLoop = true;
        bRandomizeOrder = false;
    }
};

/**
 * Tribal Animation Showcase Component
 * Manages and demonstrates various tribal character animations for the prehistoric survival game
 * Showcases realistic movement patterns and survival actions of primitive humans
 */
UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_TribalAnimationShowcase : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_TribalAnimationShowcase();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Animation Management
    UFUNCTION(BlueprintCallable, Category = "Tribal Animation")
    void StartShowcase();

    UFUNCTION(BlueprintCallable, Category = "Tribal Animation")
    void StopShowcase();

    UFUNCTION(BlueprintCallable, Category = "Tribal Animation")
    void PlayTribalAnimation(EAnim_TribalAnimationType AnimationType);

    UFUNCTION(BlueprintCallable, Category = "Tribal Animation")
    void PlayNextAnimation();

    UFUNCTION(BlueprintCallable, Category = "Tribal Animation")
    void SetShowcaseSettings(const FAnim_TribalShowcaseSettings& NewSettings);

    // Animation State
    UFUNCTION(BlueprintPure, Category = "Tribal Animation")
    bool IsShowcaseActive() const { return bShowcaseActive; }

    UFUNCTION(BlueprintPure, Category = "Tribal Animation")
    EAnim_TribalAnimationType GetCurrentAnimationType() const { return CurrentAnimationType; }

    UFUNCTION(BlueprintPure, Category = "Tribal Animation")
    float GetShowcaseProgress() const;

    // Configuration
    UFUNCTION(BlueprintCallable, Category = "Tribal Animation")
    void AddAnimationToShowcase(const FAnim_TribalAnimationData& AnimationData);

    UFUNCTION(BlueprintCallable, Category = "Tribal Animation")
    void RemoveAnimationFromShowcase(EAnim_TribalAnimationType AnimationType);

    UFUNCTION(BlueprintCallable, Category = "Tribal Animation")
    void ClearShowcaseAnimations();

protected:
    // Core Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Data")
    TArray<FAnim_TribalAnimationData> TribalAnimations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Showcase Settings")
    FAnim_TribalShowcaseSettings ShowcaseSettings;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    bool bShowcaseActive;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    EAnim_TribalAnimationType CurrentAnimationType;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    int32 CurrentAnimationIndex;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    float CurrentAnimationTimer;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    float TransitionTimer;

    // Component References
    UPROPERTY()
    USkeletalMeshComponent* TargetMeshComponent;

    UPROPERTY()
    UAnimInstance* TargetAnimInstance;

private:
    // Internal Methods
    void InitializeTribalAnimations();
    void UpdateShowcase(float DeltaTime);
    void TransitionToNextAnimation();
    bool PlayAnimationMontage(const FAnim_TribalAnimationData& AnimationData);
    void OnAnimationComplete();
    USkeletalMeshComponent* FindOwnerMeshComponent();
    void ValidateAnimationData();

    // State Management
    bool bIsTransitioning;
    float TotalShowcaseTime;
    int32 ShowcaseLoopCount;
};