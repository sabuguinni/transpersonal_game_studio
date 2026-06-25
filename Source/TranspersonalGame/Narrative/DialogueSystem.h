#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "DialogueSystem.generated.h"

// ============================================================
// Agent #15 — Narrative & Dialogue Agent
// DialogueSystem.h — Quest NPC dialogue, branching conversations
// Cycle: PROD_CYCLE_AUTO_20260625_010
// ============================================================

// ---- Enums (global scope — RULE 1) ----

UENUM(BlueprintType)
enum class ENarr_DialogueState : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Greeting        UMETA(DisplayName = "Greeting"),
    QuestOffer      UMETA(DisplayName = "QuestOffer"),
    QuestActive     UMETA(DisplayName = "QuestActive"),
    QuestComplete   UMETA(DisplayName = "QuestComplete"),
    Farewell        UMETA(DisplayName = "Farewell")
};

UENUM(BlueprintType)
enum class ENarr_NPCRole : uint8
{
    Elder       UMETA(DisplayName = "Elder"),
    Scout       UMETA(DisplayName = "Scout"),
    Hunter      UMETA(DisplayName = "Hunter"),
    Chieftain   UMETA(DisplayName = "Chieftain"),
    Gatherer    UMETA(DisplayName = "Gatherer"),
    Survivor    UMETA(DisplayName = "Survivor")
};

// ---- Structs (global scope — RULE 1) ----

USTRUCT(BlueprintType)
struct FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString LineText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_DialogueState TriggerState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float DisplayDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString AudioAssetPath;

    FNarr_DialogueLine()
        : SpeakerName(TEXT("Unknown"))
        , LineText(TEXT(""))
        , TriggerState(ENarr_DialogueState::Idle)
        , DisplayDuration(4.0f)
        , AudioAssetPath(TEXT(""))
    {}
};

USTRUCT(BlueprintType)
struct FNarr_QuestDialogue
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString QuestID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_DialogueLine> GreetingLines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_DialogueLine> QuestOfferLines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_DialogueLine> QuestActiveLines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_DialogueLine> QuestCompleteLines;

    FNarr_QuestDialogue()
        : QuestID(TEXT(""))
    {}
};

// ---- NPC Dialogue Actor ----

UCLASS(ClassGroup = (Narrative), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API ANarr_DialogueNPC : public AActor
{
    GENERATED_BODY()

public:
    ANarr_DialogueNPC();

    // NPC identity
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Identity")
    FString NPCName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Identity")
    ENarr_NPCRole NPCRole;

    // Current dialogue state
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Narrative|State")
    ENarr_DialogueState CurrentState;

    // Dialogue data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    FNarr_QuestDialogue QuestDialogue;

    // Interaction range (player must be within this distance)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Interaction")
    float InteractionRadius;

    // Is player currently in range?
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Narrative|Interaction")
    bool bPlayerInRange;

    // Functions
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void BeginDialogue();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void AdvanceDialogue();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void EndDialogue();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void SetQuestState(ENarr_DialogueState NewState);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    FNarr_DialogueLine GetCurrentLine() const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool IsPlayerInRange() const;

    // AActor overrides
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

private:
    int32 CurrentLineIndex;
    TArray<FNarr_DialogueLine>* ActiveLineArray;

    void UpdatePlayerProximity();
};

// ---- Dialogue Manager Component (attaches to PlayerController) ----

UCLASS(ClassGroup = (Narrative), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNarr_DialogueManagerComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNarr_DialogueManagerComponent();

    // Currently active NPC
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Narrative")
    ANarr_DialogueNPC* ActiveNPC;

    // Is dialogue currently running?
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Narrative")
    bool bDialogueActive;

    // Current line being displayed
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Narrative")
    FNarr_DialogueLine CurrentDisplayLine;

    // Functions
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void StartDialogueWithNPC(ANarr_DialogueNPC* NPC);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void AdvanceCurrentDialogue();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void StopDialogue();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool HasActiveDialogue() const;

    // UActorComponent overrides
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    float LineTimer;
};
