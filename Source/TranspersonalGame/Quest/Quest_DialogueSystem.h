#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/DataTable.h"
#include "Kismet/GameplayStatics.h"
#include "Quest_DialogueSystem.generated.h"

// Forward declarations
class APlayerController;
class APawn;

UENUM(BlueprintType)
enum class EQuest_DialogueState : uint8
{
    None UMETA(DisplayName = "None"),
    Active UMETA(DisplayName = "Active"),
    Completed UMETA(DisplayName = "Completed"),
    Failed UMETA(DisplayName = "Failed")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_DialogueEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FText DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bRequiresPlayerResponse;

    FQuest_DialogueEntry()
    {
        SpeakerName = TEXT("Unknown");
        DialogueText = FText::FromString(TEXT("..."));
        Duration = 3.0f;
        bRequiresPlayerResponse = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_DialogueChoice
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FText ChoiceText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    int32 NextDialogueID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bEndsDialogue;

    FQuest_DialogueChoice()
    {
        ChoiceText = FText::FromString(TEXT("Continue"));
        NextDialogueID = -1;
        bEndsDialogue = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_DialogueNode : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    int32 DialogueID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FQuest_DialogueEntry DialogueEntry;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FQuest_DialogueChoice> Choices;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString QuestID;

    FQuest_DialogueNode()
    {
        DialogueID = 0;
        QuestID = TEXT("");
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FQuest_OnDialogueStarted, const FString&, NPCName, const FText&, FirstLine);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FQuest_OnDialogueEnded);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FQuest_OnDialogueChoice, int32, ChoiceIndex, const FText&, ChoiceText);

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQuest_DialogueSystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UQuest_DialogueSystem();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Dialogue management
    UFUNCTION(BlueprintCallable, Category = "Quest Dialogue")
    bool StartDialogue(const FString& NPCName, int32 StartingDialogueID);

    UFUNCTION(BlueprintCallable, Category = "Quest Dialogue")
    void EndDialogue();

    UFUNCTION(BlueprintCallable, Category = "Quest Dialogue")
    void SelectDialogueChoice(int32 ChoiceIndex);

    UFUNCTION(BlueprintCallable, Category = "Quest Dialogue")
    bool IsDialogueActive() const { return CurrentDialogueState != EQuest_DialogueState::None; }

    UFUNCTION(BlueprintCallable, Category = "Quest Dialogue")
    FQuest_DialogueNode GetCurrentDialogueNode() const { return CurrentDialogueNode; }

    // Data table management
    UFUNCTION(BlueprintCallable, Category = "Quest Dialogue")
    void LoadDialogueDataTable(UDataTable* DialogueTable);

    UFUNCTION(BlueprintCallable, Category = "Quest Dialogue")
    bool HasDialogueForNPC(const FString& NPCName) const;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Quest Dialogue Events")
    FQuest_OnDialogueStarted OnDialogueStarted;

    UPROPERTY(BlueprintAssignable, Category = "Quest Dialogue Events")
    FQuest_OnDialogueEnded OnDialogueEnded;

    UPROPERTY(BlueprintAssignable, Category = "Quest Dialogue Events")
    FQuest_OnDialogueChoice OnDialogueChoice;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest Dialogue State")
    EQuest_DialogueState CurrentDialogueState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest Dialogue State")
    FQuest_DialogueNode CurrentDialogueNode;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest Dialogue State")
    FString CurrentNPCName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Dialogue Data")
    UDataTable* DialogueDataTable;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest Dialogue State")
    TMap<FString, TArray<int32>> NPCDialogueMap;

private:
    void ProcessDialogueNode(int32 DialogueID);
    void BuildNPCDialogueMap();
    FQuest_DialogueNode* FindDialogueNode(int32 DialogueID);

    FTimerHandle DialogueTimerHandle;
};