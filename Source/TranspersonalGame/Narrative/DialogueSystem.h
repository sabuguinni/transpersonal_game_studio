#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "SharedTypes.h"
#include "DialogueSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FText DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString AudioURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FString> ResponseOptions;

    FNarr_DialogueEntry()
    {
        SpeakerName = TEXT("");
        DialogueText = FText::GetEmpty();
        AudioURL = TEXT("");
        Duration = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_NarrativeEvent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString EventID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FText EventDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_DialogueEntry> DialogueSequence;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bIsTriggered;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float TriggerRadius;

    FNarr_NarrativeEvent()
    {
        EventID = TEXT("");
        EventDescription = FText::GetEmpty();
        bIsTriggered = false;
        TriggerRadius = 500.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UDialogueSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UDialogueSystem();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FNarr_DialogueEntry> DialogueEntries;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FNarr_NarrativeEvent> NarrativeEvents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    int32 CurrentDialogueIndex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bIsDialogueActive;

public:
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void StartDialogue(const FString& EventID);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void NextDialogue();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void EndDialogue();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    FNarr_DialogueEntry GetCurrentDialogue() const;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void TriggerNarrativeEvent(const FString& EventID, const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void AddDialogueEntry(const FNarr_DialogueEntry& NewEntry);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void LoadNarrativeData();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool IsDialogueActive() const { return bIsDialogueActive; }

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void RegisterNarrativeEvent(const FNarr_NarrativeEvent& Event);
};