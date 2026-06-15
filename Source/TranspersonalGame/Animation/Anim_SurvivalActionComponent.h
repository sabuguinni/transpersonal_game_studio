#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Animation/AnimMontage.h"
#include "Engine/DataTable.h"
#include "Anim_SurvivalActionComponent.generated.h"

UENUM(BlueprintType)
enum class EAnim_SurvivalAction : uint8
{
    None UMETA(DisplayName = "None"),
    Crafting UMETA(DisplayName = "Crafting"),
    Gathering UMETA(DisplayName = "Gathering"),
    Hunting UMETA(DisplayName = "Hunting"),
    Building UMETA(DisplayName = "Building"),
    Eating UMETA(DisplayName = "Eating"),
    Drinking UMETA(DisplayName = "Drinking"),
    Sleeping UMETA(DisplayName = "Sleeping"),
    Climbing UMETA(DisplayName = "Climbing"),
    Swimming UMETA(DisplayName = "Swimming"),
    Fighting UMETA(DisplayName = "Fighting")
};

USTRUCT(BlueprintType)
struct FAnim_SurvivalActionData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    class UAnimMontage* ActionMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    float StaminaCost;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    bool bRequiresTools;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    FString RequiredToolType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    class USoundBase* ActionSound;

    FAnim_SurvivalActionData()
    {
        ActionMontage = nullptr;
        Duration = 2.0f;
        StaminaCost = 10.0f;
        bRequiresTools = false;
        RequiredToolType = TEXT("");
        ActionSound = nullptr;
    }
};

UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_SurvivalActionComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_SurvivalActionComponent();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    class UDataTable* SurvivalActionsTable;

    UPROPERTY(BlueprintReadOnly, Category = "Animation")
    EAnim_SurvivalAction CurrentAction;

    UPROPERTY(BlueprintReadOnly, Category = "Animation")
    bool bIsPerformingAction;

    UPROPERTY(BlueprintReadOnly, Category = "Animation")
    float ActionProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    float ActionSpeedMultiplier;

    UPROPERTY(BlueprintReadOnly, Category = "Animation")
    class UAnimMontage* CurrentMontage;

    FTimerHandle ActionTimerHandle;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Animation")
    bool StartSurvivalAction(EAnim_SurvivalAction Action, AActor* TargetActor = nullptr);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void StopCurrentAction();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    bool CanPerformAction(EAnim_SurvivalAction Action) const;

    UFUNCTION(BlueprintPure, Category = "Animation")
    EAnim_SurvivalAction GetCurrentAction() const { return CurrentAction; }

    UFUNCTION(BlueprintPure, Category = "Animation")
    bool IsPerformingAction() const { return bIsPerformingAction; }

    UFUNCTION(BlueprintPure, Category = "Animation")
    float GetActionProgress() const { return ActionProgress; }

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSurvivalActionStarted, EAnim_SurvivalAction, Action, float, Duration);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSurvivalActionCompleted, EAnim_SurvivalAction, Action);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSurvivalActionCancelled, EAnim_SurvivalAction, Action);

    UPROPERTY(BlueprintAssignable, Category = "Animation")
    FOnSurvivalActionStarted OnSurvivalActionStarted;

    UPROPERTY(BlueprintAssignable, Category = "Animation")
    FOnSurvivalActionCompleted OnSurvivalActionCompleted;

    UPROPERTY(BlueprintAssignable, Category = "Animation")
    FOnSurvivalActionCancelled OnSurvivalActionCancelled;

protected:
    UFUNCTION()
    void OnActionCompleted();

    UFUNCTION()
    void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);

    FAnim_SurvivalActionData* GetActionData(EAnim_SurvivalAction Action) const;

    class UAnimInstance* GetOwnerAnimInstance() const;
};