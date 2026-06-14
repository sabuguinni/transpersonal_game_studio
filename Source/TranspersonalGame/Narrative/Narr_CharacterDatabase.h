#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SharedTypes.h"
#include "Narr_CharacterDatabase.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_CharacterProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    FString CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    FString CharacterRole;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    FString BackgroundStory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    TArray<FString> DialogueLines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    FString VoiceAudioPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    bool bIsQuestGiver;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    bool bIsTrader;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    TArray<FString> AssociatedQuests;

    FNarr_CharacterProfile()
    {
        CharacterName = TEXT("Unknown");
        CharacterRole = TEXT("Survivor");
        bIsQuestGiver = false;
        bIsTrader = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString AudioFilePath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString TriggerCondition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bIsQuestRelated;

    FNarr_DialogueLine()
    {
        SpeakerName = TEXT("Unknown");
        bIsQuestRelated = false;
    }
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UNarr_CharacterDatabase : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Characters")
    TArray<FNarr_CharacterProfile> Characters;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FNarr_DialogueLine> DialogueDatabase;

    UFUNCTION(BlueprintCallable, Category = "Characters")
    FNarr_CharacterProfile GetCharacterByName(const FString& Name) const;

    UFUNCTION(BlueprintCallable, Category = "Characters")
    TArray<FNarr_DialogueLine> GetDialogueForCharacter(const FString& CharacterName) const;

    UFUNCTION(BlueprintCallable, Category = "Characters")
    TArray<FNarr_CharacterProfile> GetQuestGivers() const;

    UFUNCTION(BlueprintCallable, Category = "Characters")
    void AddCharacter(const FNarr_CharacterProfile& NewCharacter);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void AddDialogueLine(const FNarr_DialogueLine& NewDialogue);

protected:
    void InitializeDefaultCharacters();
};