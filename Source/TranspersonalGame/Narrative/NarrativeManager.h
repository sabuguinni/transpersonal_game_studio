#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/DataTable.h"
#include "../SharedTypes.h"
#include "DialogueSystem.h"
#include "NarrativeManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_StoryEvent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString EventID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString EventName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString EventDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    TArray<FString> Prerequisites;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    TArray<FString> UnlockedEvents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString AssociatedDialogue;

    FNarr_StoryEvent()
    {
        EventID = TEXT("");
        EventName = TEXT("");
        EventDescription = TEXT("");
        bIsCompleted = false;
        AssociatedDialogue = TEXT("");
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_CharacterProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    FString CharacterID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    FString CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    FString CharacterRole;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    FString CharacterBackground;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    TArray<FString> AvailableDialogues;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    FVector SpawnLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    bool bIsAlive;

    FNarr_CharacterProfile()
    {
        CharacterID = TEXT("");
        CharacterName = TEXT("");
        CharacterRole = TEXT("");
        CharacterBackground = TEXT("");
        SpawnLocation = FVector::ZeroVector;
        bIsAlive = true;
    }
};

UCLASS()
class TRANSPERSONALGAME_API UNarrativeManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    TArray<FNarr_StoryEvent> StoryEvents;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    TArray<FNarr_CharacterProfile> Characters;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    FString CurrentStoryPhase;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    int32 DaysSurvived;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    TArray<FString> CompletedEvents;

public:
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TriggerStoryEvent(const FString& EventID);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool IsEventCompleted(const FString& EventID);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    TArray<FNarr_StoryEvent> GetAvailableEvents();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void AdvanceStoryPhase(const FString& NewPhase);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    FNarr_CharacterProfile GetCharacterProfile(const FString& CharacterID);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void RegisterCharacter(const FNarr_CharacterProfile& NewCharacter);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    TArray<FString> GetCharacterDialogues(const FString& CharacterID);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void IncrementDaysSurvived();

    UFUNCTION(BlueprintPure, Category = "Narrative")
    int32 GetDaysSurvived() const { return DaysSurvived; }

    UFUNCTION(BlueprintPure, Category = "Narrative")
    FString GetCurrentStoryPhase() const { return CurrentStoryPhase; }

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void SaveNarrativeProgress();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void LoadNarrativeProgress();

private:
    void InitializeStoryEvents();
    void InitializeCharacters();
    bool CheckEventPrerequisites(const FNarr_StoryEvent& Event);
};