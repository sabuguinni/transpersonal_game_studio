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
    FString AudioPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FString> RequiredFlags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FString> SetFlags;

    FNarr_DialogueEntry()
    {
        DialogueID = "";
        SpeakerName = "";
        DialogueText = FText::GetEmpty();
        AudioPath = "";
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
    FText EventText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_NarrativeContext Context;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bIsRepeatable;

    FNarr_NarrativeEvent()
    {
        EventID = "";
        EventText = FText::GetEmpty();
        Context = ENarr_NarrativeContext::Exploration;
        Priority = 1.0f;
        bIsRepeatable = false;
    }
};

UCLASS()
class TRANSPERSONALGAME_API UNarrativeDialogueManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UNarrativeDialogueManager();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Dialogue system
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void PlayDialogue(const FString& DialogueID, AActor* Speaker = nullptr);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void RegisterDialogueEntry(const FNarr_DialogueEntry& Entry);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool HasDialogue(const FString& DialogueID) const;

    // Narrative events
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TriggerNarrativeEvent(const FString& EventID, ENarr_NarrativeContext Context = ENarr_NarrativeContext::Exploration);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void RegisterNarrativeEvent(const FNarr_NarrativeEvent& Event);

    // Flag system for story progression
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void SetStoryFlag(const FString& FlagName, bool bValue = true);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool GetStoryFlag(const FString& FlagName) const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void ClearStoryFlag(const FString& FlagName);

    // Context-aware narrative
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void UpdateNarrativeContext(ENarr_NarrativeContext NewContext);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    ENarr_NarrativeContext GetCurrentNarrativeContext() const { return CurrentContext; }

    // Survival narrative integration
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void OnPlayerHealthCritical();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void OnDinosaurEncounter(const FString& DinosaurType, float Distance);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void OnBiomeEntered(const FString& BiomeName);

protected:
    UPROPERTY()
    TMap<FString, FNarr_DialogueEntry> DialogueDatabase;

    UPROPERTY()
    TMap<FString, FNarr_NarrativeEvent> NarrativeEvents;

    UPROPERTY()
    TMap<FString, bool> StoryFlags;

    UPROPERTY()
    ENarr_NarrativeContext CurrentContext;

    UPROPERTY()
    float LastNarrativeTime;

    UPROPERTY()
    float NarrativeCooldown;

private:
    void LoadDefaultDialogues();
    void LoadDefaultNarrativeEvents();
    bool CanPlayNarrative() const;
    void PlayAudioForDialogue(const FString& AudioPath);
};