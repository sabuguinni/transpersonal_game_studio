#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "../SharedTypes.h"
#include "Narr_DialogueSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString AudioPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ENPCMoodState RequiredMood;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bIsPlayerResponse;

    FNarr_DialogueLine()
    {
        SpeakerName = TEXT("");
        DialogueText = TEXT("");
        AudioPath = TEXT("");
        Duration = 3.0f;
        RequiredMood = ENPCMoodState::Neutral;
        bIsPlayerResponse = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueChoice
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString ChoiceText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString NextDialogueID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    int32 RequiredSurvivalLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bEndsConversation;

    FNarr_DialogueChoice()
    {
        ChoiceText = TEXT("");
        NextDialogueID = TEXT("");
        RequiredSurvivalLevel = 0;
        bEndsConversation = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueNode
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DialogueID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FNarr_DialogueLine> DialogueLines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FNarr_DialogueChoice> PlayerChoices;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ENPCRole NPCRole;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bIsQuestDialogue;

    FNarr_DialogueNode()
    {
        DialogueID = TEXT("");
        NPCRole = ENPCRole::Gatherer;
        bIsQuestDialogue = false;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNarr_DialogueSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UNarr_DialogueSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Dialogue Management
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool StartDialogue(const FString& DialogueID, class ANarr_TribalNPC* NPC);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void SelectPlayerChoice(int32 ChoiceIndex);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void EndDialogue();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool IsDialogueActive() const { return bIsDialogueActive; }

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    FNarr_DialogueNode GetCurrentDialogueNode() const { return CurrentDialogueNode; }

    // Dialogue Database
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void LoadDialogueDatabase();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    FNarr_DialogueNode GetDialogueNode(const FString& DialogueID) const;

    // Audio Playback
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void PlayDialogueAudio(const FString& AudioPath);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void StopDialogueAudio();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TMap<FString, FNarr_DialogueNode> DialogueDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FNarr_DialogueNode CurrentDialogueNode;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bIsDialogueActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    int32 CurrentLineIndex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float DialogueTimer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    class ANarr_TribalNPC* CurrentNPC;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    class UAudioComponent* DialogueAudioComponent;

private:
    void InitializeDialogueDatabase();
    void ProcessCurrentDialogueLine();
    void AdvanceToNextLine();
    bool ValidateDialogueChoice(const FNarr_DialogueChoice& Choice) const;
};