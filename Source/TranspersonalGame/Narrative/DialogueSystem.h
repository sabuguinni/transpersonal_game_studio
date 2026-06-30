
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "SharedTypes.h"
#include "DialogueSystem.generated.h"

// ============================================================
// ENarr_DialogueSpeaker — who is speaking
// ============================================================
UENUM(BlueprintType)
enum class ENarr_DialogueSpeaker : uint8
{
    Player          UMETA(DisplayName = "Player"),
    ElderKael       UMETA(DisplayName = "Elder Kael"),
    ScoutMira       UMETA(DisplayName = "Scout Mira"),
    HunterBrak      UMETA(DisplayName = "Hunter Brak"),
    Narrator        UMETA(DisplayName = "Narrator"),
    Unknown         UMETA(DisplayName = "Unknown")
};

// ============================================================
// ENarr_DialogueCondition — conditions that unlock dialogue
// ============================================================
UENUM(BlueprintType)
enum class ENarr_DialogueCondition : uint8
{
    None            UMETA(DisplayName = "None"),
    HasCraftedAxe   UMETA(DisplayName = "Has Crafted Axe"),
    SurvivedNight   UMETA(DisplayName = "Survived Night"),
    KilledRaptor    UMETA(DisplayName = "Killed Raptor"),
    FoundHerdTracks UMETA(DisplayName = "Found Herd Tracks"),
    HealthBelow30   UMETA(DisplayName = "Health Below 30"),
    HungerBelow50   UMETA(DisplayName = "Hunger Below 50")
};

// ============================================================
// FNarr_DialogueLine — a single spoken line
// ============================================================
USTRUCT(BlueprintType)
struct FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    FName LineID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    ENarr_DialogueSpeaker Speaker;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    FText LineText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    FString AudioURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    float DisplayDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    ENarr_DialogueCondition RequiredCondition;

    FNarr_DialogueLine()
        : Speaker(ENarr_DialogueSpeaker::Unknown)
        , DisplayDuration(4.0f)
        , RequiredCondition(ENarr_DialogueCondition::None)
    {}
};

// ============================================================
// FNarr_DialogueNode — a node in the dialogue tree
// ============================================================
USTRUCT(BlueprintType)
struct FNarr_DialogueNode
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    FName NodeID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    TArray<FNarr_DialogueLine> Lines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    TArray<FName> NextNodeIDs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    bool bIsTerminal;

    FNarr_DialogueNode()
        : bIsTerminal(false)
    {}
};

// ============================================================
// FNarr_DialogueTree — full conversation tree for an NPC
// ============================================================
USTRUCT(BlueprintType)
struct FNarr_DialogueTree
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    FName TreeID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    ENarr_DialogueSpeaker OwnerNPC;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    TArray<FNarr_DialogueNode> Nodes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    FName RootNodeID;

    FNarr_DialogueTree()
        : OwnerNPC(ENarr_DialogueSpeaker::Unknown)
    {}
};

// ============================================================
// UNarr_DialogueComponent — attaches to NPC actors
// ============================================================
UCLASS(ClassGroup = (Narrative), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNarr_DialogueComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNarr_DialogueComponent();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    FNarr_DialogueTree DialogueTree;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    ENarr_DialogueSpeaker NPCSpeaker;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    float InteractionRadius;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative|Dialogue", meta = (AllowPrivateAccess = "true"))
    bool bIsInDialogue;

    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    bool StartDialogue(AActor* Initiator);

    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    void EndDialogue();

    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    FNarr_DialogueLine GetCurrentLine() const;

    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    bool AdvanceDialogue();

    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    bool CheckCondition(ENarr_DialogueCondition Condition, AActor* Player) const;

protected:
    virtual void BeginPlay() override;

private:
    FName CurrentNodeID;
    int32 CurrentLineIndex;
};

// ============================================================
// ANarr_DialogueManager — world actor managing all dialogue
// ============================================================
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ANarr_DialogueManager : public AActor
{
    GENERATED_BODY()

public:
    ANarr_DialogueManager();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    TArray<FNarr_DialogueTree> AllDialogueTrees;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    TMap<FName, FString> VoiceLineAudioURLs;

    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    void RegisterVoiceLine(FName LineID, const FString& AudioURL);

    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    FString GetVoiceLineURL(FName LineID) const;

    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    void RegisterDialogueTree(const FNarr_DialogueTree& Tree);

    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    FNarr_DialogueTree FindTreeByNPC(ENarr_DialogueSpeaker NPC) const;

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Narrative|Dialogue")
    void PopulateDefaultDialogue();

protected:
    virtual void BeginPlay() override;
};
