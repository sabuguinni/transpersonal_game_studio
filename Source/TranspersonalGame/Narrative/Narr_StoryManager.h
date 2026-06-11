#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "Narr_StoryManager.generated.h"

class UNarr_CharacterDialogue;

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_StoryEvent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story")
    FString EventID;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story")
    FText EventTitle;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story")
    FText EventDescription;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story")
    TArray<FString> PrerequisiteEvents;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story")
    ESurvivalContext TriggerContext;

    FNarr_StoryEvent()
    {
        EventID = TEXT("");
        EventTitle = FText::FromString(TEXT("Unknown Event"));
        EventDescription = FText::FromString(TEXT(""));
        bIsCompleted = false;
        TriggerContext = ESurvivalContext::Normal;
    }
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UNarr_StoryManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UNarr_StoryManager();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story")
    TArray<FNarr_StoryEvent> StoryEvents;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story")
    TMap<FString, UNarr_CharacterDialogue*> CharacterDialogues;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story")
    int32 CurrentStoryPhase;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story")
    float SurvivalDaysElapsed;

    UFUNCTION(BlueprintCallable, Category = "Story")
    void TriggerStoryEvent(const FString& EventID);

    UFUNCTION(BlueprintCallable, Category = "Story")
    bool IsEventCompleted(const FString& EventID) const;

    UFUNCTION(BlueprintCallable, Category = "Story")
    TArray<FNarr_StoryEvent> GetAvailableEvents() const;

    UFUNCTION(BlueprintCallable, Category = "Story")
    UNarr_CharacterDialogue* GetCharacterDialogue(const FString& CharacterName) const;

    UFUNCTION(BlueprintCallable, Category = "Story")
    void RegisterCharacterDialogue(const FString& CharacterName, UNarr_CharacterDialogue* Dialogue);

    UFUNCTION(BlueprintCallable, Category = "Story")
    void AdvanceStoryPhase();

    UFUNCTION(BlueprintCallable, Category = "Story")
    void UpdateSurvivalTime(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Story")
    FText GetCurrentStoryContext() const;

private:
    void InitializeStoryEvents();
    void CheckEventTriggers();
    
    UPROPERTY()
    TArray<FString> CompletedEvents;
};