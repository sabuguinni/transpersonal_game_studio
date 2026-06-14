#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "Components/AudioComponent.h"
#include "Narr_DialogueManager.generated.h"

UENUM(BlueprintType)
enum class ENarr_DialogueType : uint8
{
    Greeting,
    Warning,
    Information,
    Trade,
    Quest,
    Combat
};

USTRUCT(BlueprintType)
struct FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FText DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ENarr_DialogueType DialogueType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    class USoundBase* VoiceClip;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bIsPlayerResponse;

    FNarr_DialogueLine()
    {
        SpeakerName = TEXT("Unknown");
        DialogueText = FText::FromString(TEXT("..."));
        DialogueType = ENarr_DialogueType::Information;
        VoiceClip = nullptr;
        Duration = 3.0f;
        bIsPlayerResponse = false;
    }
};

USTRUCT(BlueprintType)
struct FNarr_DialogueSequence
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString SequenceID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FNarr_DialogueLine> DialogueLines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bRepeatable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    int32 Priority;

    FNarr_DialogueSequence()
    {
        SequenceID = TEXT("DefaultSequence");
        bRepeatable = true;
        Priority = 1;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ANarr_DialogueManager : public AActor
{
    GENERATED_BODY()

public:
    ANarr_DialogueManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue System")
    class UDataTable* DialogueDataTable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue System")
    TMap<FString, FNarr_DialogueSequence> LoadedDialogues;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue System")
    class UAudioComponent* VoiceAudioComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue System")
    float DefaultDialogueRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue System")
    bool bAutoPlayVoice;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue State")
    bool bIsDialogueActive;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue State")
    FString CurrentSequenceID;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue State")
    int32 CurrentLineIndex;

public:
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool StartDialogue(const FString& SequenceID, class APawn* Speaker, class APawn* Listener);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void EndDialogue();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void NextDialogueLine();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool LoadDialogueSequence(const FString& SequenceID);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    FNarr_DialogueLine GetCurrentDialogueLine();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    TArray<FString> GetAvailableDialogues();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void RegisterNPCDialogue(class APawn* NPC, const FString& SequenceID);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool CanStartDialogue(class APawn* Speaker, class APawn* Listener);

    UFUNCTION(BlueprintImplementableEvent, Category = "Dialogue Events")
    void OnDialogueStarted(const FString& SequenceID);

    UFUNCTION(BlueprintImplementableEvent, Category = "Dialogue Events")
    void OnDialogueLineChanged(const FNarr_DialogueLine& NewLine);

    UFUNCTION(BlueprintImplementableEvent, Category = "Dialogue Events")
    void OnDialogueEnded();

private:
    void PlayVoiceLine(class USoundBase* VoiceClip);
    void InitializeDefaultDialogues();

    UPROPERTY()
    TMap<class APawn*, FString> NPCDialogueMap;

    UPROPERTY()
    class APawn* CurrentSpeaker;

    UPROPERTY()
    class APawn* CurrentListener;
};