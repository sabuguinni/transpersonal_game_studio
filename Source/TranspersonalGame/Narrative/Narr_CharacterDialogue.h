#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Sound/SoundBase.h"
#include "Narr_CharacterDialogue.generated.h"

UENUM(BlueprintType)
enum class ENarr_DialogueType : uint8
{
    Greeting,
    Warning,
    Information,
    Trade,
    Quest,
    Farewell,
    Combat,
    Emergency
};

UENUM(BlueprintType)
enum class ENarr_CharacterRole : uint8
{
    TribalElder,
    WarChief,
    Scout,
    CampKeeper,
    Hunter,
    Crafter,
    Healer,
    Trader
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FText DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TSoftObjectPtr<USoundBase> VoiceClip;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ENarr_DialogueType DialogueType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FText> PlayerResponses;

    FNarr_DialogueLine()
    {
        DialogueType = ENarr_DialogueType::Information;
        Duration = 3.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_CharacterProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    FText CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    ENarr_CharacterRole Role;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    FText BackgroundStory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    TArray<FNarr_DialogueLine> DialogueLines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    TArray<FString> KnownTopics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    float TrustLevel;

    FNarr_CharacterProfile()
    {
        Role = ENarr_CharacterRole::CampKeeper;
        TrustLevel = 0.5f;
    }
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UNarr_CharacterDialogue : public UDataAsset
{
    GENERATED_BODY()

public:
    UNarr_CharacterDialogue();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    FNarr_CharacterProfile CharacterProfile;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TMap<ENarr_DialogueType, TArray<FNarr_DialogueLine>> DialoguesByType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FNarr_DialogueLine> ContextualDialogue;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    FNarr_DialogueLine GetRandomDialogue(ENarr_DialogueType Type);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    TArray<FNarr_DialogueLine> GetDialogueByType(ENarr_DialogueType Type);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool HasDialogueForType(ENarr_DialogueType Type);

    UFUNCTION(BlueprintCallable, Category = "Character")
    void ModifyTrustLevel(float Delta);

    UFUNCTION(BlueprintCallable, Category = "Character")
    bool CanDiscussTopics(const TArray<FString>& Topics);
};