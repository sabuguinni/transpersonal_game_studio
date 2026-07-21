#pragma once

#include "CoreMinimal.h"
#include "Engine/TriggerBox.h"
#include "Components/BoxComponent.h"
#include "Quest_QuestObjective.h"
#include "Quest_QuestTrigger.generated.h"

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API AQuest_QuestTrigger : public ATriggerBox
{
    GENERATED_BODY()

public:
    AQuest_QuestTrigger();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Trigger")
    FString QuestID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Trigger")
    FString ObjectiveID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Trigger")
    EQuest_ObjectiveType TriggerType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Trigger")
    FString RequiredItemID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Trigger")
    int32 TriggerCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Trigger")
    bool bOneTimeUse;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Trigger")
    bool bIsActivated;

    UFUNCTION()
    void OnOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, 
                       class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, 
                       bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnOverlapEnd(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, 
                     class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

public:
    UFUNCTION(BlueprintCallable, Category = "Quest Trigger")
    void ActivateTrigger(AActor* TriggeringActor);

    UFUNCTION(BlueprintCallable, Category = "Quest Trigger")
    bool CanTrigger(AActor* TriggeringActor) const;

    UFUNCTION(BlueprintCallable, Category = "Quest Trigger")
    void ResetTrigger();

    UFUNCTION(BlueprintImplementableEvent, Category = "Quest Trigger")
    void OnQuestTriggerActivated(const FString& QuestID, const FString& ObjectiveID);

    UFUNCTION(BlueprintImplementableEvent, Category = "Quest Trigger")
    void OnQuestObjectiveCompleted(const FString& QuestID, const FString& ObjectiveID);
};

#include "Quest_QuestTrigger.generated.h"