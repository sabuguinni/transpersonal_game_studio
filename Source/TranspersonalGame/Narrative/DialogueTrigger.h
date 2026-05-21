#pragma once

#include "CoreMinimal.h"
#include "Engine/TriggerBox.h"
#include "Components/StaticMeshComponent.h"
#include "NarrativeManager.h"
#include "DialogueTrigger.generated.h"

UENUM(BlueprintType)
enum class ENarr_TriggerType : uint8
{
    OnEnter     UMETA(DisplayName = "On Enter"),
    OnExit      UMETA(DisplayName = "On Exit"),
    OnStay      UMETA(DisplayName = "On Stay"),
    OnInteract  UMETA(DisplayName = "On Interact")
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ADialogueTrigger : public ATriggerBox
{
    GENERATED_BODY()

public:
    ADialogueTrigger();

protected:
    virtual void BeginPlay() override;

    UFUNCTION()
    void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, 
                       UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, 
                       bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, 
                     UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DialogueID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString StoryBeatID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ENarr_TriggerType TriggerType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bOneTimeOnly;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bRequiresPlayerOnly;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float DelayBeforeTrigger;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FString> RequiredConditions;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* VisualIndicator;

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    bool bHasTriggered;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    UNarrativeManager* NarrativeManager;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    FTimerHandle DelayTimerHandle;

private:
    void ExecuteTrigger(AActor* TriggeringActor);
    void DelayedTriggerExecution();
    bool CheckTriggerConditions(AActor* TriggeringActor) const;
    void SetupVisualIndicator();
};

#include "DialogueTrigger.generated.h"