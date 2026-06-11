#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "Sound/SoundCue.h"
#include "SharedTypes.h"
#include "NarrativeDialogueSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueNode
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString NodeID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FString> PlayerResponses;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FString> NextNodeIDs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TSoftObjectPtr<USoundCue> VoiceAudio;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float DisplayDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bIsEndNode;

    FNarr_DialogueNode()
    {
        NodeID = TEXT("");
        SpeakerName = TEXT("");
        DialogueText = TEXT("");
        VoiceAudio = nullptr;
        DisplayDuration = 3.0f;
        bIsEndNode = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueTree : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue Tree")
    FString TreeID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue Tree")
    FString TreeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue Tree")
    TArray<FNarr_DialogueNode> DialogueNodes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue Tree")
    FString StartNodeID;

    FNarr_DialogueTree()
    {
        TreeID = TEXT("");
        TreeName = TEXT("");
        StartNodeID = TEXT("");
    }
};

UENUM(BlueprintType)
enum class ENarr_DialogueState : uint8
{
    Inactive        UMETA(DisplayName = "Inactive"),
    Playing         UMETA(DisplayName = "Playing"),
    WaitingForInput UMETA(DisplayName = "Waiting For Input"),
    Paused          UMETA(DisplayName = "Paused")
};

UCLASS(BlueprintType, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNarrativeDialogueSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UNarrativeDialogueSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue System")
    class UDataTable* DialogueDataTable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue System")
    float DefaultTextDisplaySpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue System")
    bool bAutoAdvanceDialogue;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    ENarr_DialogueState CurrentDialogueState;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    FNarr_DialogueTree* CurrentDialogueTree;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    FNarr_DialogueNode* CurrentDialogueNode;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    float CurrentDisplayTimer;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    int32 CurrentCharacterIndex;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    FString DisplayedText;

public:
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool StartDialogueTree(const FString& TreeID);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void StopDialogue();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void PauseDialogue();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void ResumeDialogue();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void AdvanceDialogue();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void SelectPlayerResponse(int32 ResponseIndex);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool IsDialogueActive() const;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    FString GetCurrentSpeaker() const;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    FString GetCurrentDialogueText() const;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    TArray<FString> GetCurrentPlayerResponses() const;

    UFUNCTION(BlueprintImplementableEvent, Category = "Dialogue Events")
    void OnDialogueStarted(const FString& TreeID);

    UFUNCTION(BlueprintImplementableEvent, Category = "Dialogue Events")
    void OnDialogueEnded();

    UFUNCTION(BlueprintImplementableEvent, Category = "Dialogue Events")
    void OnDialogueNodeChanged(const FString& Speaker, const FString& Text);

    UFUNCTION(BlueprintImplementableEvent, Category = "Dialogue Events")
    void OnPlayerResponsesAvailable(const TArray<FString>& Responses);

protected:
    UFUNCTION()
    void UpdateTextDisplay(float DeltaTime);

    UFUNCTION()
    FNarr_DialogueTree* FindDialogueTree(const FString& TreeID);

    UFUNCTION()
    FNarr_DialogueNode* FindDialogueNode(const FString& NodeID);

    UFUNCTION()
    void SetCurrentDialogueNode(FNarr_DialogueNode* NewNode);

    UFUNCTION()
    void ProcessDialogueNode();

    UFUNCTION()
    void InitializeDefaultDialogueTrees();
};