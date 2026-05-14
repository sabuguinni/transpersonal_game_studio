#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Components/ActorComponent.h"
#include "Narr_DialogueManager.generated.h"

UENUM(BlueprintType)
enum class ENarr_DialogueType : uint8
{
    None = 0,
    TribalElder = 1,
    TribalScout = 2,
    TribalShaman = 3,
    TribalWarrior = 4,
    TribalHunter = 5,
    TribalCrafter = 6
};

USTRUCT(BlueprintType)
struct FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ENarr_DialogueType SpeakerType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bIsImportant;

    FNarr_DialogueLine()
    {
        SpeakerName = TEXT("Unknown");
        DialogueText = TEXT("");
        SpeakerType = ENarr_DialogueType::None;
        Duration = 3.0f;
        bIsImportant = false;
    }
};

USTRUCT(BlueprintType)
struct FNarr_DialogueSequence
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString SequenceName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FNarr_DialogueLine> DialogueLines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bAutoAdvance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float AutoAdvanceDelay;

    FNarr_DialogueSequence()
    {
        SequenceName = TEXT("DefaultSequence");
        bAutoAdvance = true;
        AutoAdvanceDelay = 1.0f;
    }
};

UCLASS(BlueprintType, Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNarr_DialogueManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UNarr_DialogueManager();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void PlayDialogueSequence(const FString& SequenceName);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void PlaySingleDialogue(const FNarr_DialogueLine& DialogueLine);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void StopCurrentDialogue();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void RegisterDialogueSequence(const FString& SequenceName, const FNarr_DialogueSequence& Sequence);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool IsDialoguePlaying() const;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    FString GetCurrentSpeaker() const;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void SetDialogueVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void CreateTribalDialogues();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TMap<FString, FNarr_DialogueSequence> RegisteredSequences;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FNarr_DialogueSequence CurrentSequence;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    int32 CurrentLineIndex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bIsPlaying;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float DialogueVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString CurrentSpeakerName;

    FTimerHandle DialogueTimerHandle;

private:
    void AdvanceDialogue();
    void OnDialogueLineComplete();
    void CreateDefaultTribalSequences();
};