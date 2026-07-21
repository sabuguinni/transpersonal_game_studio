#pragma once

#include "CoreMinimal.h"
#include "Engine/TriggerBox.h"
#include "Components/BoxComponent.h"
#include "Narr_DialogueSystem.h"
#include "Narr_StoryTrigger.generated.h"

UENUM(BlueprintType)
enum class ENarr_TriggerEvent : uint8
{
    FirstEncounter,
    DangerWarning,
    ResourceDiscovery,
    TerritoryEntry,
    CombatStart
};

UCLASS()
class TRANSPERSONALGAME_API ANarr_StoryTrigger : public ATriggerBox
{
    GENERATED_BODY()

public:
    ANarr_StoryTrigger();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UNarr_DialogueSystem* DialogueComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    ENarr_TriggerEvent TriggerEventType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString TriggerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    bool bOneTimeUse;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    bool bHasBeenTriggered;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    float DelayBeforeDialogue;

public:
    UFUNCTION()
    void OnOverlapBegin(class AActor* OverlappedActor, class AActor* OtherActor);

    UFUNCTION()
    void OnOverlapEnd(class AActor* OverlappedActor, class AActor* OtherActor);

    UFUNCTION(BlueprintCallable, Category = "Story")
    void TriggerStoryEvent();

    UFUNCTION(BlueprintCallable, Category = "Story")
    void SetupDialogueForEvent(ENarr_TriggerEvent EventType);

    UFUNCTION(BlueprintCallable, Category = "Story")
    void ResetTrigger();
};