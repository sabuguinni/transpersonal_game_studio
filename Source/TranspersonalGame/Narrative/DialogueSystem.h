#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DialogueSystem.generated.h"

// Narr_ prefix per UE5 compilation rules — unique across project

UENUM(BlueprintType)
enum class ENarr_DialogueState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Active      UMETA(DisplayName = "Active"),
    Completed   UMETA(DisplayName = "Completed"),
    Locked      UMETA(DisplayName = "Locked")
};

UENUM(BlueprintType)
enum class ENarr_NPCRole : uint8
{
    QuestGiver      UMETA(DisplayName = "QuestGiver"),
    Merchant        UMETA(DisplayName = "Merchant"),
    TribalMember    UMETA(DisplayName = "TribalMember"),
    Scout           UMETA(DisplayName = "Scout"),
    Hunter          UMETA(DisplayName = "Hunter"),
    Elder           UMETA(DisplayName = "Elder")
};

USTRUCT(BlueprintType)
struct FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString LineText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float DisplayDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bRequiresPlayerResponse;

    FNarr_DialogueLine()
        : SpeakerName(TEXT("Unknown"))
        , LineText(TEXT(""))
        , DisplayDuration(4.0f)
        , bRequiresPlayerResponse(false)
    {}
};

USTRUCT(BlueprintType)
struct FNarr_DialogueTree
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FName DialogueID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_DialogueLine> Lines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_DialogueState State;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FName LinkedQuestID;

    FNarr_DialogueTree()
        : DialogueID(NAME_None)
        , State(ENarr_DialogueState::Idle)
        , LinkedQuestID(NAME_None)
    {}
};

UCLASS(ClassGroup = (Narrative), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNarr_DialogueComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNarr_DialogueComponent();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_NPCRole NPCRole;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString NPCName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_DialogueTree> DialogueTrees;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float InteractionRadius;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    bool bIsInDialogue;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    int32 CurrentLineIndex;

    // Start dialogue with given ID
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool StartDialogue(FName DialogueID);

    // Advance to next line
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool AdvanceLine();

    // End current dialogue
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void EndDialogue();

    // Get current line text
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    FString GetCurrentLineText() const;

    // Get current speaker name
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    FString GetCurrentSpeakerName() const;

    // Check if player is in range
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool IsPlayerInRange(AActor* Player) const;

    // Mark dialogue as completed
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void MarkDialogueCompleted(FName DialogueID);

protected:
    virtual void BeginPlay() override;

private:
    int32 ActiveTreeIndex;

    void InitializeDefaultDialogues();
};
