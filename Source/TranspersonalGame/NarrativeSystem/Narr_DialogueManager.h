#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/DataTable.h"
#include "Narr_DialogueManager.generated.h"

UENUM(BlueprintType)
enum class ENarr_CharacterType : uint8
{
    TribalElder         UMETA(DisplayName = "Tribal Elder"),
    TribalScout         UMETA(DisplayName = "Tribal Scout"), 
    TribalShaman        UMETA(DisplayName = "Tribal Shaman"),
    TribalGuide         UMETA(DisplayName = "Tribal Guide"),
    TribalHunter        UMETA(DisplayName = "Tribal Hunter"),
    TribalCrafter       UMETA(DisplayName = "Tribal Crafter"),
    Survivor            UMETA(DisplayName = "Survivor"),
    Stranger            UMETA(DisplayName = "Stranger")
};

UENUM(BlueprintType)
enum class ENarr_DialogueContext : uint8
{
    FirstMeeting        UMETA(DisplayName = "First Meeting"),
    QuestGiving         UMETA(DisplayName = "Quest Giving"),
    QuestProgress       UMETA(DisplayName = "Quest Progress"),
    QuestComplete       UMETA(DisplayName = "Quest Complete"),
    Warning             UMETA(DisplayName = "Warning"),
    Information         UMETA(DisplayName = "Information"),
    Trading             UMETA(DisplayName = "Trading"),
    Farewell            UMETA(DisplayName = "Farewell")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ENarr_CharacterType CharacterType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ENarr_DialogueContext Context;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString AudioFilePath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FString> PlayerResponses;

    FNarr_DialogueLine()
    {
        SpeakerName = TEXT("");
        CharacterType = ENarr_CharacterType::Survivor;
        Context = ENarr_DialogueContext::Information;
        DialogueText = TEXT("");
        AudioFilePath = TEXT("");
        Duration = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_StoryBeat
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString BeatID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString Title;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    TArray<FString> RequiredQuests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    TArray<FNarr_DialogueLine> DialogueLines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    bool bIsCompleted;

    FNarr_StoryBeat()
    {
        BeatID = TEXT("");
        Title = TEXT("");
        Description = TEXT("");
        bIsCompleted = false;
    }
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UNarr_DialogueManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UNarr_DialogueManager();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Dialogue system functions
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void StartDialogue(const FString& CharacterName, ENarr_CharacterType CharacterType, ENarr_DialogueContext Context);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    FNarr_DialogueLine GetDialogueLine(const FString& CharacterName, ENarr_DialogueContext Context);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void AddDialogueLine(const FNarr_DialogueLine& NewDialogue);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    TArray<FNarr_DialogueLine> GetDialoguesByCharacterType(ENarr_CharacterType CharacterType);

    // Story progression functions
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TriggerStoryBeat(const FString& BeatID);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void CompleteStoryBeat(const FString& BeatID);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool IsStoryBeatCompleted(const FString& BeatID);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    TArray<FNarr_StoryBeat> GetAvailableStoryBeats();

    // Narrative context functions
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void SetNarrativeContext(const FString& ContextKey, const FString& ContextValue);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    FString GetNarrativeContext(const FString& ContextKey);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void InitializeDefaultDialogues();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    TArray<FNarr_DialogueLine> DialogueDatabase;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    TArray<FNarr_StoryBeat> StoryBeats;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    TMap<FString, FString> NarrativeContext;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    FString CurrentDialogueCharacter;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    ENarr_DialogueContext CurrentContext;

private:
    void CreateTribalElderDialogues();
    void CreateTribalScoutDialogues();
    void CreateTribalShamanDialogues();
    void CreateTribalGuideDialogues();
    void CreateSurvivalStoryBeats();
};