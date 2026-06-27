#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Components/BoxComponent.h"
#include "DialogueSystem.generated.h"

// ─── Enums ───────────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class ENarr_DialogueCondition : uint8
{
    None            UMETA(DisplayName = "None"),
    QuestActive     UMETA(DisplayName = "Quest Active"),
    QuestComplete   UMETA(DisplayName = "Quest Complete"),
    HasItem         UMETA(DisplayName = "Has Item"),
    StatThreshold   UMETA(DisplayName = "Stat Threshold"),
    FirstMeeting    UMETA(DisplayName = "First Meeting")
};

UENUM(BlueprintType)
enum class ENarr_DialogueAction : uint8
{
    None            UMETA(DisplayName = "None"),
    StartQuest      UMETA(DisplayName = "Start Quest"),
    CompleteQuest   UMETA(DisplayName = "Complete Quest"),
    GiveItem        UMETA(DisplayName = "Give Item"),
    SetFlag         UMETA(DisplayName = "Set Flag"),
    EndConversation UMETA(DisplayName = "End Conversation")
};

// ─── Structs ─────────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FNarr_DialogueChoice
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString ChoiceText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    int32 NextNodeIndex = -1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ENarr_DialogueCondition Condition = ENarr_DialogueCondition::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString ConditionValue;
};

USTRUCT(BlueprintType)
struct FNarr_DialogueNode
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    int32 NodeIndex = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString SpeakerID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DialogueText;

    /** ElevenLabs TTS audio URL for this line */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString AudioURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FNarr_DialogueChoice> Choices;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ENarr_DialogueAction OnEnterAction = ENarr_DialogueAction::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString ActionPayload;

    /** If true and no choices, conversation ends after this node */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bIsTerminal = false;
};

USTRUCT(BlueprintType)
struct FNarr_DialogueTree
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString TreeID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString NPCID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FNarr_DialogueNode> Nodes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    int32 EntryNodeIndex = 0;
};

// ─── Dialogue Manager Component ──────────────────────────────────────────────

UCLASS(ClassGroup = "Narrative", meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNarr_DialogueManagerComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNarr_DialogueManagerComponent();

    /** All registered dialogue trees (loaded at game start) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FNarr_DialogueTree> DialogueTrees;

    /** Currently active tree (null if no conversation in progress) */
    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    FString ActiveTreeID;

    /** Current node index within the active tree */
    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    int32 CurrentNodeIndex = 0;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool StartDialogue(const FString& TreeID);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool AdvanceDialogue(int32 ChoiceIndex);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void EndDialogue();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    FNarr_DialogueNode GetCurrentNode() const;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool IsDialogueActive() const;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void RegisterDialogueTree(const FNarr_DialogueTree& Tree);

    /** Build all Elder NPC dialogue trees at runtime */
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void BuildElderDialogueTrees();

protected:
    virtual void BeginPlay() override;

private:
    FNarr_DialogueTree* FindTree(const FString& TreeID);
    void ExecuteNodeAction(const FNarr_DialogueNode& Node);

    UPROPERTY()
    bool bDialogueActive = false;
};

// ─── NPC Dialogue Actor ───────────────────────────────────────────────────────

UCLASS()
class TRANSPERSONALGAME_API ANarr_NPCDialogueActor : public AActor
{
    GENERATED_BODY()

public:
    ANarr_NPCDialogueActor();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UBoxComponent* InteractionVolume;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UNarr_DialogueManagerComponent* DialogueManager;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
    FString NPCID = TEXT("Elder");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
    FString NPCDisplayName = TEXT("Elder");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
    FString DefaultDialogueTreeID = TEXT("ELDER_INTRO");

    UFUNCTION(BlueprintCallable, Category = "NPC")
    void OnPlayerEnterRange(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);

protected:
    virtual void BeginPlay() override;
};
