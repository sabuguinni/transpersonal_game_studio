#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SharedTypes.h"
#include "DialogueSystem.generated.h"

UENUM(BlueprintType)
enum class ENarr_DialogueTrigger : uint8
{
    Proximity       UMETA(DisplayName = "Proximity"),
    QuestStart      UMETA(DisplayName = "QuestStart"),
    QuestComplete   UMETA(DisplayName = "QuestComplete"),
    PlayerAction    UMETA(DisplayName = "PlayerAction"),
    Danger          UMETA(DisplayName = "Danger"),
    Discovery       UMETA(DisplayName = "Discovery")
};

UENUM(BlueprintType)
enum class ENarr_SpeakerRole : uint8
{
    TrailReader     UMETA(DisplayName = "TrailReader"),
    HuntCaller      UMETA(DisplayName = "HuntCaller"),
    CampKeeper      UMETA(DisplayName = "CampKeeper"),
    RiverGuide      UMETA(DisplayName = "RiverGuide"),
    ElderHunter     UMETA(DisplayName = "ElderHunter"),
    ScoutWarrior    UMETA(DisplayName = "ScoutWarrior"),
    HuntLeader      UMETA(DisplayName = "HuntLeader"),
    SurvivalScout   UMETA(DisplayName = "SurvivalScout")
};

USTRUCT(BlueprintType)
struct FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString LineID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_SpeakerRole SpeakerRole;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString AudioURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_DialogueTrigger TriggerType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float DisplayDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bHasBeenPlayed;

    FNarr_DialogueLine()
        : LineID(TEXT(""))
        , SpeakerName(TEXT("Unknown"))
        , SpeakerRole(ENarr_SpeakerRole::ScoutWarrior)
        , DialogueText(TEXT(""))
        , AudioURL(TEXT(""))
        , TriggerType(ENarr_DialogueTrigger::Proximity)
        , DisplayDuration(5.0f)
        , bHasBeenPlayed(false)
    {}
};

USTRUCT(BlueprintType)
struct FNarr_DialogueSequence
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString SequenceID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_DialogueLine> Lines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    int32 CurrentLineIndex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bIsActive;

    FNarr_DialogueSequence()
        : SequenceID(TEXT(""))
        , CurrentLineIndex(0)
        , bIsActive(false)
    {}
};

UCLASS(ClassGroup = (Narrative), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNarr_DialogueSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UNarr_DialogueSystem();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void StartDialogueSequence(const FString& SequenceID);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void AdvanceDialogue();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void EndDialogueSequence();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool IsDialogueActive() const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    FNarr_DialogueLine GetCurrentLine() const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TriggerContextualDialogue(ENarr_DialogueTrigger Trigger, const FString& ContextID);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void RegisterDialogueLine(const FNarr_DialogueLine& Line);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void RegisterDialogueSequence(const FNarr_DialogueSequence& Sequence);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_DialogueLine> RegisteredLines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_DialogueSequence> RegisteredSequences;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float ProximityTriggerRadius;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative", meta = (AllowPrivateAccess = "true"))
    bool bDialogueActive;

private:
    FNarr_DialogueSequence ActiveSequence;
    float LineTimer;
    float LineDisplayDuration;

    void LoadDefaultDialogueLines();
    void CheckProximityTriggers();
};
