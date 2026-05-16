#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/DataTable.h"
#include "Narr_DialogueSystem.generated.h"

UENUM(BlueprintType)
enum class ENarr_DialogueType : uint8
{
    Greeting,
    Warning,
    Information,
    Quest,
    Lore,
    Combat
};

UENUM(BlueprintType)
enum class ENarr_CharacterType : uint8
{
    TribalElder,
    TribalWarrior,
    TribalScout,
    TribalTracker,
    TribalHunter,
    TribalShaman
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DialogueID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ENarr_CharacterType SpeakerType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ENarr_DialogueType DialogueType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FText DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString AudioURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FString> TriggerConditions;

    FNarr_DialogueLine()
    {
        SpeakerType = ENarr_CharacterType::TribalElder;
        DialogueType = ENarr_DialogueType::Information;
        Duration = 10.0f;
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
    int32 Priority;

    FNarr_DialogueSequence()
    {
        bIsRepeatable = true;
        Priority = 1;
    }
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UNarr_DialogueSystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void RegisterDialogueLine(const FNarr_DialogueLine& DialogueLine);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void RegisterDialogueSequence(const FNarr_DialogueSequence& DialogueSequence);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    FNarr_DialogueLine GetRandomDialogue(ENarr_DialogueType DialogueType, ENarr_CharacterType CharacterType);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    TArray<FNarr_DialogueLine> GetDialogueSequence(const FString& SequenceID);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void PlayDialogue(const FNarr_DialogueLine& DialogueLine);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void LoadDialogueDatabase();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    int32 GetTotalDialogueCount() const;

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    TArray<FNarr_DialogueLine> DialogueDatabase;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    TArray<FNarr_DialogueSequence> DialogueSequences;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    TMap<FString, int32> PlayedDialogues;

private:
    void InitializeDefaultDialogues();
    bool CheckDialogueConditions(const TArray<FString>& Conditions);
};