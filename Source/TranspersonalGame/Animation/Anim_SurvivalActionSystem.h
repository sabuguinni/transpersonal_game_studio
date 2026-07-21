#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimInstance.h"
#include "TimerManager.h"
#include "Anim_SurvivalActionSystem.generated.h"

UENUM(BlueprintType)
enum class EAnim_SurvivalAction : uint8
{
    None            UMETA(DisplayName = "None"),
    Crafting        UMETA(DisplayName = "Crafting Tool"),
    Gathering       UMETA(DisplayName = "Gathering Berries"),
    Building        UMETA(DisplayName = "Building Shelter"),
    Hunting         UMETA(DisplayName = "Spear Throwing"),
    Climbing        UMETA(DisplayName = "Rock Climbing"),
    Swimming        UMETA(DisplayName = "Swimming"),
    Cooking         UMETA(DisplayName = "Cooking Food"),
    Fishing         UMETA(DisplayName = "Fishing"),
    Healing         UMETA(DisplayName = "Applying Medicine")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_SurvivalActionData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Action")
    EAnim_SurvivalAction ActionType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Action")
    TSoftObjectPtr<UAnimMontage> ActionMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Action")
    float ActionDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Action")
    float StaminaCost;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Action")
    bool bRequiresTool;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Action")
    FString RequiredToolName;

    FAnim_SurvivalActionData()
    {
        ActionType = EAnim_SurvivalAction::None;
        ActionDuration = 2.0f;
        StaminaCost = 10.0f;
        bRequiresTool = false;
        RequiredToolName = TEXT("");
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FAnim_OnSurvivalActionComplete, EAnim_SurvivalAction, ActionType, bool, bSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FAnim_OnSurvivalActionStart, EAnim_SurvivalAction, ActionType, float, Duration);

UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_SurvivalActionSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_SurvivalActionSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Action Management
    UFUNCTION(BlueprintCallable, Category = "Survival Actions")
    bool StartSurvivalAction(EAnim_SurvivalAction ActionType);

    UFUNCTION(BlueprintCallable, Category = "Survival Actions")
    void StopCurrentAction();

    UFUNCTION(BlueprintCallable, Category = "Survival Actions")
    bool CanPerformAction(EAnim_SurvivalAction ActionType) const;

    UFUNCTION(BlueprintPure, Category = "Survival Actions")
    EAnim_SurvivalAction GetCurrentAction() const { return CurrentAction; }

    UFUNCTION(BlueprintPure, Category = "Survival Actions")
    bool IsPerformingAction() const { return CurrentAction != EAnim_SurvivalAction::None; }

    UFUNCTION(BlueprintPure, Category = "Survival Actions")
    float GetActionProgress() const;

    // Configuration
    UFUNCTION(BlueprintCallable, Category = "Survival Actions")
    void SetActionData(EAnim_SurvivalAction ActionType, const FAnim_SurvivalActionData& ActionData);

    UFUNCTION(BlueprintCallable, Category = "Survival Actions")
    FAnim_SurvivalActionData GetActionData(EAnim_SurvivalAction ActionType) const;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Survival Actions")
    FAnim_OnSurvivalActionComplete OnActionComplete;

    UPROPERTY(BlueprintAssignable, Category = "Survival Actions")
    FAnim_OnSurvivalActionStart OnActionStart;

protected:
    // Current action state
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Survival Actions")
    EAnim_SurvivalAction CurrentAction;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Survival Actions")
    float ActionStartTime;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Survival Actions")
    float ActionDuration;

    // Action configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Actions")
    TMap<EAnim_SurvivalAction, FAnim_SurvivalActionData> ActionDataMap;

    // Animation references
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation")
    class USkeletalMeshComponent* CharacterMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation")
    class UAnimInstance* AnimInstance;

    // Timer handle for action completion
    FTimerHandle ActionTimerHandle;

private:
    void InitializeActionData();
    void OnActionTimerComplete();
    bool PlayActionMontage(UAnimMontage* Montage);
    void StopActionMontage();
};