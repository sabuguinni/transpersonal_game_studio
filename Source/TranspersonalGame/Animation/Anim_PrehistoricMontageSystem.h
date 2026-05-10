#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/ActorComponent.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimInstance.h"
#include "Animation/SkeletalMeshActor.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Anim_PrehistoricMontageSystem.generated.h"

UENUM(BlueprintType)
enum class EAnim_PrehistoricAction : uint8
{
    None            UMETA(DisplayName = "None"),
    Crafting        UMETA(DisplayName = "Crafting Tools"),
    Gathering       UMETA(DisplayName = "Gathering Resources"),
    Hunting         UMETA(DisplayName = "Hunting"),
    ShelterBuilding UMETA(DisplayName = "Building Shelter"),
    FireMaking      UMETA(DisplayName = "Making Fire"),
    ToolCrafting    UMETA(DisplayName = "Crafting Tools"),
    FoodPrep        UMETA(DisplayName = "Preparing Food"),
    WaterGathering  UMETA(DisplayName = "Gathering Water"),
    StoneKnapping   UMETA(DisplayName = "Knapping Stone"),
    HideWorking     UMETA(DisplayName = "Working Hides")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_PrehistoricMontageData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prehistoric Animation")
    EAnim_PrehistoricAction ActionType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prehistoric Animation")
    UAnimMontage* Montage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prehistoric Animation")
    float PlayRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prehistoric Animation")
    float BlendInTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prehistoric Animation")
    float BlendOutTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prehistoric Animation")
    bool bLooping;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prehistoric Animation")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prehistoric Animation")
    int32 Priority;

    FAnim_PrehistoricMontageData()
    {
        ActionType = EAnim_PrehistoricAction::None;
        Montage = nullptr;
        PlayRate = 1.0f;
        BlendInTime = 0.25f;
        BlendOutTime = 0.25f;
        bLooping = false;
        Duration = 2.0f;
        Priority = 1;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_ActionSequence
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action Sequence")
    TArray<EAnim_PrehistoricAction> Actions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action Sequence")
    TArray<float> ActionDelays;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action Sequence")
    bool bLoopSequence;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action Sequence")
    float SequenceCooldown;

    FAnim_ActionSequence()
    {
        bLoopSequence = false;
        SequenceCooldown = 5.0f;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPrehistoricActionStarted, EAnim_PrehistoricAction, ActionType, float, Duration);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPrehistoricActionCompleted, EAnim_PrehistoricAction, ActionType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPrehistoricActionInterrupted, EAnim_PrehistoricAction, ActionType);

UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_PrehistoricMontageSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_PrehistoricMontageSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core montage data for prehistoric actions
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prehistoric Montages")
    TMap<EAnim_PrehistoricAction, FAnim_PrehistoricMontageData> PrehistoricMontages;

    // Action sequences for complex behaviors
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action Sequences")
    TMap<FString, FAnim_ActionSequence> ActionSequences;

    // Current state
    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    EAnim_PrehistoricAction CurrentAction;

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    bool bIsPlayingAction;

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    float CurrentActionTimeRemaining;

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    int32 CurrentSequenceIndex;

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    FString CurrentSequenceName;

    // Component references
    UPROPERTY()
    USkeletalMeshComponent* SkeletalMeshComponent;

    UPROPERTY()
    UAnimInstance* AnimInstance;

    // Timer handles
    FTimerHandle ActionTimerHandle;
    FTimerHandle SequenceTimerHandle;

public:
    // Event delegates
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnPrehistoricActionStarted OnActionStarted;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnPrehistoricActionCompleted OnActionCompleted;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnPrehistoricActionInterrupted OnActionInterrupted;

    // Core montage functions
    UFUNCTION(BlueprintCallable, Category = "Prehistoric Animation")
    bool PlayPrehistoricAction(EAnim_PrehistoricAction ActionType, float CustomPlayRate = 1.0f, bool bForceRestart = false);

    UFUNCTION(BlueprintCallable, Category = "Prehistoric Animation")
    void StopCurrentAction(float BlendOutTime = 0.25f);

    UFUNCTION(BlueprintCallable, Category = "Prehistoric Animation")
    void StopAllActions();

    UFUNCTION(BlueprintCallable, Category = "Prehistoric Animation")
    bool IsPlayingAction(EAnim_PrehistoricAction ActionType) const;

    UFUNCTION(BlueprintCallable, Category = "Prehistoric Animation")
    EAnim_PrehistoricAction GetCurrentAction() const { return CurrentAction; }

    UFUNCTION(BlueprintCallable, Category = "Prehistoric Animation")
    float GetCurrentActionProgress() const;

    // Action sequence functions
    UFUNCTION(BlueprintCallable, Category = "Action Sequences")
    bool PlayActionSequence(const FString& SequenceName, bool bLoopSequence = false);

    UFUNCTION(BlueprintCallable, Category = "Action Sequences")
    void StopActionSequence();

    UFUNCTION(BlueprintCallable, Category = "Action Sequences")
    void AddActionSequence(const FString& SequenceName, const FAnim_ActionSequence& Sequence);

    UFUNCTION(BlueprintCallable, Category = "Action Sequences")
    bool IsPlayingSequence() const { return !CurrentSequenceName.IsEmpty(); }

    // Montage management
    UFUNCTION(BlueprintCallable, Category = "Montage Management")
    void RegisterPrehistoricMontage(EAnim_PrehistoricAction ActionType, UAnimMontage* Montage, float Duration = 2.0f, int32 Priority = 1);

    UFUNCTION(BlueprintCallable, Category = "Montage Management")
    UAnimMontage* GetMontageForAction(EAnim_PrehistoricAction ActionType) const;

    UFUNCTION(BlueprintCallable, Category = "Montage Management")
    void SetMontagePlayRate(EAnim_PrehistoricAction ActionType, float NewPlayRate);

    // Utility functions
    UFUNCTION(BlueprintCallable, Category = "Utility")
    void InitializeDefaultMontages();

    UFUNCTION(BlueprintCallable, Category = "Utility")
    void CreateDefaultActionSequences();

    UFUNCTION(BlueprintCallable, Category = "Utility", CallInEditor = true)
    void TestPrehistoricActions();

protected:
    // Internal functions
    void OnActionCompleted_Internal();
    void OnSequenceStepCompleted();
    void PlayNextSequenceAction();
    bool CanPlayAction(EAnim_PrehistoricAction ActionType) const;
    void UpdateActionTimer(float DeltaTime);
    void FindSkeletalMeshComponent();
    void SetupDefaultMontageData();
    void SetupDefaultSequences();

    // Montage event callbacks
    UFUNCTION()
    void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);

    UFUNCTION()
    void OnMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted);
};