#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimInstance.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "TranspersonalGame.h"
#include "Anim_SurvivalActionController.generated.h"

UENUM(BlueprintType)
enum class EAnim_SurvivalAction : uint8
{
    None            UMETA(DisplayName = "None"),
    Gathering       UMETA(DisplayName = "Gathering"),
    Crafting        UMETA(DisplayName = "Crafting"),
    Building        UMETA(DisplayName = "Building"),
    Hunting         UMETA(DisplayName = "Hunting"),
    Eating          UMETA(DisplayName = "Eating"),
    Drinking        UMETA(DisplayName = "Drinking"),
    Resting         UMETA(DisplayName = "Resting"),
    Climbing        UMETA(DisplayName = "Climbing"),
    Swimming        UMETA(DisplayName = "Swimming"),
    Hiding          UMETA(DisplayName = "Hiding"),
    Exploring       UMETA(DisplayName = "Exploring")
};

UENUM(BlueprintType)
enum class EAnim_ActionPriority : uint8
{
    Low             UMETA(DisplayName = "Low"),
    Normal          UMETA(DisplayName = "Normal"),
    High            UMETA(DisplayName = "High"),
    Critical        UMETA(DisplayName = "Critical")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_SurvivalActionData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action")
    EAnim_SurvivalAction ActionType = EAnim_SurvivalAction::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action")
    UAnimMontage* ActionMontage = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action")
    float Duration = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action")
    EAnim_ActionPriority Priority = EAnim_ActionPriority::Normal;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action")
    bool bCanBeInterrupted = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action")
    bool bRequiresTarget = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action")
    FString ActionDescription = TEXT("");

    FAnim_SurvivalActionData()
    {
        ActionType = EAnim_SurvivalAction::None;
        ActionMontage = nullptr;
        Duration = 2.0f;
        Priority = EAnim_ActionPriority::Normal;
        bCanBeInterrupted = true;
        bRequiresTarget = false;
        ActionDescription = TEXT("");
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSurvivalActionStarted, EAnim_SurvivalAction, ActionType, float, Duration);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSurvivalActionCompleted, EAnim_SurvivalAction, ActionType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSurvivalActionInterrupted, EAnim_SurvivalAction, ActionType);

/**
 * Component that handles survival action animations
 * Manages montages for gathering, crafting, building, hunting, etc.
 * Provides priority system and interruption handling
 */
UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_SurvivalActionController : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_SurvivalActionController();

protected:
    virtual void BeginPlay() override;

    // Action database
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Actions")
    TMap<EAnim_SurvivalAction, FAnim_SurvivalActionData> ActionDatabase;

    // Current action state
    UPROPERTY(BlueprintReadOnly, Category = "Current Action", meta = (AllowPrivateAccess = "true"))
    EAnim_SurvivalAction CurrentAction;

    UPROPERTY(BlueprintReadOnly, Category = "Current Action", meta = (AllowPrivateAccess = "true"))
    float CurrentActionTimeRemaining;

    UPROPERTY(BlueprintReadOnly, Category = "Current Action", meta = (AllowPrivateAccess = "true"))
    bool bIsPerformingAction;

    // Component references
    UPROPERTY(BlueprintReadOnly, Category = "References", meta = (AllowPrivateAccess = "true"))
    class ACharacter* OwnerCharacter;

    UPROPERTY(BlueprintReadOnly, Category = "References", meta = (AllowPrivateAccess = "true"))
    class USkeletalMeshComponent* SkeletalMeshComponent;

    UPROPERTY(BlueprintReadOnly, Category = "References", meta = (AllowPrivateAccess = "true"))
    class UAnimInstance* AnimInstance;

    // Action queue for chaining actions
    UPROPERTY(BlueprintReadOnly, Category = "Action Queue", meta = (AllowPrivateAccess = "true"))
    TArray<EAnim_SurvivalAction> ActionQueue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    int32 MaxQueueSize = 5;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    bool bAllowActionQueuing = true;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnSurvivalActionStarted OnSurvivalActionStarted;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnSurvivalActionCompleted OnSurvivalActionCompleted;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnSurvivalActionInterrupted OnSurvivalActionInterrupted;

    // Main action functions
    UFUNCTION(BlueprintCallable, Category = "Survival Actions")
    bool StartSurvivalAction(EAnim_SurvivalAction ActionType, bool bForceStart = false);

    UFUNCTION(BlueprintCallable, Category = "Survival Actions")
    bool StopCurrentAction(bool bInterrupt = false);

    UFUNCTION(BlueprintCallable, Category = "Survival Actions")
    bool QueueSurvivalAction(EAnim_SurvivalAction ActionType);

    UFUNCTION(BlueprintCallable, Category = "Survival Actions")
    void ClearActionQueue();

    // Action state queries
    UFUNCTION(BlueprintPure, Category = "Survival Actions")
    bool IsPerformingAction() const { return bIsPerformingAction; }

    UFUNCTION(BlueprintPure, Category = "Survival Actions")
    EAnim_SurvivalAction GetCurrentAction() const { return CurrentAction; }

    UFUNCTION(BlueprintPure, Category = "Survival Actions")
    float GetCurrentActionTimeRemaining() const { return CurrentActionTimeRemaining; }

    UFUNCTION(BlueprintPure, Category = "Survival Actions")
    float GetCurrentActionProgress() const;

    UFUNCTION(BlueprintPure, Category = "Survival Actions")
    bool CanStartAction(EAnim_SurvivalAction ActionType) const;

    UFUNCTION(BlueprintPure, Category = "Survival Actions")
    bool CanInterruptCurrentAction() const;

    // Action database management
    UFUNCTION(BlueprintCallable, Category = "Action Database")
    void RegisterSurvivalAction(EAnim_SurvivalAction ActionType, const FAnim_SurvivalActionData& ActionData);

    UFUNCTION(BlueprintCallable, Category = "Action Database")
    bool GetSurvivalActionData(EAnim_SurvivalAction ActionType, FAnim_SurvivalActionData& OutActionData) const;

    UFUNCTION(BlueprintCallable, Category = "Action Database")
    void SetActionMontage(EAnim_SurvivalAction ActionType, UAnimMontage* NewMontage);

    // Utility functions
    UFUNCTION(BlueprintPure, Category = "Survival Actions")
    TArray<EAnim_SurvivalAction> GetAvailableActions() const;

    UFUNCTION(BlueprintPure, Category = "Survival Actions")
    TArray<EAnim_SurvivalAction> GetQueuedActions() const { return ActionQueue; }

    UFUNCTION(BlueprintCallable, Category = "Survival Actions")
    void InitializeDefaultActions();

private:
    // Internal functions
    void InitializeReferences();
    void ProcessActionQueue();
    void CompleteCurrentAction();
    void InterruptCurrentAction();
    bool PlayActionMontage(const FAnim_SurvivalActionData& ActionData);
    void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);
    bool HasHigherPriority(EAnim_SurvivalAction NewAction, EAnim_SurvivalAction CurrentAction) const;
    
    // Timer handle for action completion
    FTimerHandle ActionTimerHandle;
    
    // Default action data
    void SetupDefaultActionDatabase();
};