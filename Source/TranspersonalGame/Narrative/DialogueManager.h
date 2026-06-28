#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "DialogueManager.generated.h"

UENUM(BlueprintType)
enum class ENarr_DialogueTrigger : uint8
{
    Proximity       UMETA(DisplayName = "Proximity"),
    QuestStart      UMETA(DisplayName = "QuestStart"),
    QuestComplete   UMETA(DisplayName = "QuestComplete"),
    DangerNear      UMETA(DisplayName = "DangerNear"),
    PlayerInjured   UMETA(DisplayName = "PlayerInjured"),
    Discovery       UMETA(DisplayName = "Discovery"),
    Idle            UMETA(DisplayName = "Idle"),
    Cinematic       UMETA(DisplayName = "Cinematic")
};

UENUM(BlueprintType)
enum class ENarr_SpeakerRole : uint8
{
    HunterElder     UMETA(DisplayName = "HunterElder"),
    TrailReader     UMETA(DisplayName = "TrailReader"),
    ScoutLeader     UMETA(DisplayName = "ScoutLeader"),
    Survivor        UMETA(DisplayName = "Survivor"),
    TribalChief     UMETA(DisplayName = "TribalChief"),
    YoungHunter     UMETA(DisplayName = "YoungHunter"),
    Narrator        UMETA(DisplayName = "Narrator")
};

USTRUCT(BlueprintType)
struct FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FName LineID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_SpeakerRole Speaker;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FText LineText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_DialogueTrigger Trigger;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float DisplayDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TSoftObjectPtr<USoundBase> VoiceAudio;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bBlocksInput;

    FNarr_DialogueLine()
        : Speaker(ENarr_SpeakerRole::Narrator)
        , Trigger(ENarr_DialogueTrigger::Idle)
        , DisplayDuration(5.0f)
        , bBlocksInput(false)
    {}
};

USTRUCT(BlueprintType)
struct FNarr_DialogueSequence
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FName SequenceID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_DialogueLine> Lines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bLoops;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float PauseBetweenLines;

    FNarr_DialogueSequence()
        : bLoops(false)
        , PauseBetweenLines(1.0f)
    {}
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FNarr_OnDialogueStarted, FName, SequenceID, ENarr_SpeakerRole, Speaker);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNarr_OnDialogueEnded, FName, SequenceID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FNarr_OnLineDisplayed, FNarr_DialogueLine, Line, int32, LineIndex);

UCLASS(ClassGroup = "Narrative", meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNarr_DialogueManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UNarr_DialogueManager();

    UPROPERTY(BlueprintAssignable, Category = "Narrative|Events")
    FNarr_OnDialogueStarted OnDialogueStarted;

    UPROPERTY(BlueprintAssignable, Category = "Narrative|Events")
    FNarr_OnDialogueEnded OnDialogueEnded;

    UPROPERTY(BlueprintAssignable, Category = "Narrative|Events")
    FNarr_OnLineDisplayed OnLineDisplayed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Config")
    TArray<FNarr_DialogueSequence> DialogueLibrary;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Config")
    float GlobalVoiceVolume;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative|State", meta = (AllowPrivateAccess = "true"))
    bool bIsPlayingDialogue;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative|State", meta = (AllowPrivateAccess = "true"))
    FName ActiveSequenceID;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative|State", meta = (AllowPrivateAccess = "true"))
    int32 CurrentLineIndex;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void PlaySequence(FName SequenceID);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void StopDialogue();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void SkipCurrentLine();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool HasSequence(FName SequenceID) const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    FNarr_DialogueSequence GetSequence(FName SequenceID) const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TriggerByContext(ENarr_DialogueTrigger TriggerType);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void RegisterSequence(const FNarr_DialogueSequence& Sequence);

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    float LineTimer;
    FNarr_DialogueSequence ActiveSequence;

    void AdvanceToNextLine();
    void DisplayLine(const FNarr_DialogueLine& Line);
    void PopulateDefaultDialogue();
};
