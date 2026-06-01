#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SharedTypes.h"
#include "Narr_CharacterDialogue.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FText DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString AudioPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FString> ResponseOptions;

    FNarr_DialogueLine()
    {
        SpeakerName = TEXT("Unknown");
        DialogueText = FText::GetEmpty();
        AudioPath = TEXT("");
        Duration = 3.0f;
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
    TArray<FString> Prerequisites;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bIsRepeatable;

    FNarr_DialogueSequence()
    {
        SequenceID = TEXT("default");
        bIsRepeatable = true;
    }
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UNarr_CharacterDialogue : public UDataAsset
{
    GENERATED_BODY()

public:
    UNarr_CharacterDialogue();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    FString CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    ENarr_NPCType NPCType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FNarr_DialogueSequence> DialogueSequences;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FNarr_DialogueSequence DefaultGreeting;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FNarr_DialogueSequence EmergencyWarning;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TArray<FString> VoiceAssetPaths;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    FNarr_DialogueSequence GetDialogueByID(const FString& SequenceID);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    TArray<FString> GetAvailableSequences();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool HasPrerequisites(const FString& SequenceID, const TArray<FString>& PlayerProgress);
};