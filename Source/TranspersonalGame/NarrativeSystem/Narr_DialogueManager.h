#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/DataTable.h"
#include "Narr_DialogueManager.generated.h"

UENUM(BlueprintType)
enum class ENarr_CharacterType : uint8
{
    TribalElder     UMETA(DisplayName = "Tribal Elder"),
    TribalScout     UMETA(DisplayName = "Tribal Scout"),
    TribalShaman    UMETA(DisplayName = "Tribal Shaman"),
    TribalMentor    UMETA(DisplayName = "Tribal Mentor"),
    TribalHunter    UMETA(DisplayName = "Tribal Hunter"),
    TribalCrafter   UMETA(DisplayName = "Tribal Crafter")
};

UENUM(BlueprintType)
enum class ENarr_DialogueContext : uint8
{
    FirstMeeting    UMETA(DisplayName = "First Meeting"),
    QuestGiving     UMETA(DisplayName = "Quest Giving"),
    QuestProgress   UMETA(DisplayName = "Quest Progress"),
    QuestComplete   UMETA(DisplayName = "Quest Complete"),
    Warning         UMETA(DisplayName = "Warning"),
    Teaching        UMETA(DisplayName = "Teaching"),
    Trading         UMETA(DisplayName = "Trading"),
    Casual          UMETA(DisplayName = "Casual")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FText DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ENarr_CharacterType SpeakerType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ENarr_DialogueContext Context;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString AudioURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float DisplayDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FText> PlayerResponses;

    FNarr_DialogueLine()
    {
        SpeakerType = ENarr_CharacterType::TribalElder;
        Context = ENarr_DialogueContext::Casual;
        DisplayDuration = 5.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueSequence
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString SequenceID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FNarr_DialogueLine> DialogueLines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bIsRepeatable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bRequiresQuestCompletion;

    FNarr_DialogueSequence()
    {
        bIsRepeatable = true;
        bRequiresQuestCompletion = false;
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

    // Dialogue management
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void StartDialogue(const FString& SequenceID, AActor* NPCActor);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void EndDialogue();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void AdvanceDialogue();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void SelectPlayerResponse(int32 ResponseIndex);

    // Dialogue data management
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void RegisterDialogueSequence(const FNarr_DialogueSequence& Sequence);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    FNarr_DialogueSequence GetDialogueSequence(const FString& SequenceID);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool HasDialogueSequence(const FString& SequenceID);

    // Character voice management
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    FString GetCharacterVoiceURL(ENarr_CharacterType CharacterType, const FString& DialogueKey);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void RegisterCharacterVoice(ENarr_CharacterType CharacterType, const FString& DialogueKey, const FString& AudioURL);

    // State queries
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool IsDialogueActive() const { return bIsDialogueActive; }

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    FNarr_DialogueLine GetCurrentDialogueLine() const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    int32 GetCurrentDialogueIndex() const { return CurrentDialogueIndex; }

protected:
    // Dialogue state
    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    bool bIsDialogueActive;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    FString CurrentSequenceID;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    int32 CurrentDialogueIndex;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    AActor* CurrentNPCActor;

    // Dialogue data storage
    UPROPERTY()
    TMap<FString, FNarr_DialogueSequence> DialogueSequences;

    UPROPERTY()
    TMap<FString, FString> CharacterVoiceMap;

private:
    void InitializeDefaultDialogues();
    void InitializeCharacterVoices();
    FString GenerateVoiceKey(ENarr_CharacterType CharacterType, const FString& DialogueKey);
};