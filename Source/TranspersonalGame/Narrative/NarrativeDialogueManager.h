#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/DataTable.h"
#include "SharedTypes.h"
#include "NarrativeDialogueManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DialogueID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FText DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString AudioURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FString> RequiredFlags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FString> SetFlags;

    FNarr_DialogueEntry()
    {
        DialogueID = "";
        SpeakerName = "Unknown";
        DialogueText = FText::FromString("...");
        AudioURL = "";
        Duration = 3.0f;
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
    ENarr_EventTrigger TriggerType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FVector TriggerLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float TriggerRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_DialogueEntry> DialogueSequence;

    FNarr_NarrativeEvent()
    {
        EventID = "";
        EventDescription = FText::FromString("Narrative Event");
        TriggerType = ENarr_EventTrigger::LocationBased;
        TriggerLocation = FVector::ZeroVector;
        TriggerRadius = 500.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UNarrativeDialogueManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UNarrativeDialogueManager();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void RegisterNarrativeEvent(const FNarr_NarrativeEvent& Event);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TriggerNarrativeEvent(const FString& EventID);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void PlayDialogueSequence(const TArray<FNarr_DialogueEntry>& DialogueSequence);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void SetNarrativeFlag(const FString& FlagName, bool bValue);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool GetNarrativeFlag(const FString& FlagName) const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void CheckLocationTriggers(const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void LoadNarrativeData();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    TArray<FNarr_DialogueEntry> GetDialoguesByTags(const TArray<FString>& Tags);

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    TMap<FString, FNarr_NarrativeEvent> RegisteredEvents;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    TMap<FString, bool> NarrativeFlags;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    TArray<FNarr_DialogueEntry> CurrentDialogueQueue;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    bool bIsPlayingDialogue;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    float DialogueTimer;

    void ProcessDialogueQueue();
    void FinishCurrentDialogue();
    bool CheckDialogueConditions(const FNarr_DialogueEntry& Entry) const;
    void ApplyDialogueEffects(const FNarr_DialogueEntry& Entry);

public:
    virtual void Tick(float DeltaTime) override;
    virtual bool IsTickable() const override { return true; }
    virtual TStatId GetStatId() const override { RETURN_QUICK_DECLARE_CYCLE_STAT(UNarrativeDialogueManager, STATGROUP_Tickables); }
};