#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/DataTable.h"
#include "Narr_DialogueSystem.generated.h"

UENUM(BlueprintType)
enum class ENarr_DialogueType : uint8
{
    PlayerChoice,
    NPCStatement,
    SystemMessage,
    QuestUpdate
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueEntry : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FText DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ENarr_DialogueType DialogueType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FString> PlayerChoices;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString NextDialogueID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bRequiresItem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString RequiredItemName;

    FNarr_DialogueEntry()
    {
        SpeakerName = TEXT("Unknown");
        DialogueText = FText::FromString(TEXT("..."));
        DialogueType = ENarr_DialogueType::NPCStatement;
        bRequiresItem = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UNarr_DialogueSystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UNarr_DialogueSystem();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool StartDialogue(const FString& DialogueID, AActor* Speaker, AActor* Listener);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void EndDialogue();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool SelectPlayerChoice(int32 ChoiceIndex);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    FNarr_DialogueEntry GetCurrentDialogue() const;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool IsDialogueActive() const;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void LoadDialogueData(UDataTable* DialogueTable);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void RegisterDialogueActor(AActor* Actor, const FString& ActorID);

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    FNarr_DialogueEntry CurrentDialogue;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    bool bDialogueActive;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    AActor* CurrentSpeaker;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    AActor* CurrentListener;

    UPROPERTY()
    UDataTable* DialogueDataTable;

    UPROPERTY()
    TMap<FString, AActor*> RegisteredDialogueActors;

private:
    bool LoadDialogueEntry(const FString& DialogueID);
    void ProcessDialogueChoice(const FString& NextDialogueID);
    bool CheckDialogueRequirements(const FNarr_DialogueEntry& Entry);
};

#include "Narr_DialogueSystem.generated.h"