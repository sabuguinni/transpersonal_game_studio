#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Engine/TriggerVolume.h"
#include "Components/ActorComponent.h"
#include "../SharedTypes.h"
#include "StoryEventManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_StoryEvent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString EventID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    ENarr_StoryTriggerType TriggerType;

    FNarr_StoryEvent()
    {
        EventID = TEXT("");
        DialogueText = TEXT("");
        CharacterName = TEXT("");
        bIsCompleted = false;
        TriggerType = ENarr_StoryTriggerType::ProximityTrigger;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UStoryEventManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UStoryEventManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Events")
    TArray<FNarr_StoryEvent> ActiveStoryEvents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Events")
    TArray<FNarr_StoryEvent> CompletedStoryEvents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Events")
    float TriggerDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Events")
    class ACharacter* PlayerCharacter;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Story Events")
    void TriggerStoryEvent(const FString& EventID);

    UFUNCTION(BlueprintCallable, Category = "Story Events")
    void AddStoryEvent(const FNarr_StoryEvent& NewEvent);

    UFUNCTION(BlueprintCallable, Category = "Story Events")
    bool IsEventCompleted(const FString& EventID) const;

    UFUNCTION(BlueprintCallable, Category = "Story Events")
    FNarr_StoryEvent GetStoryEvent(const FString& EventID) const;

    UFUNCTION(BlueprintCallable, Category = "Story Events")
    void InitializeDefaultEvents();

private:
    void CheckProximityTriggers();
    void ProcessStoryEvent(FNarr_StoryEvent& Event);
};