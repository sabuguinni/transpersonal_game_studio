// DialogueSystem.h
// Agent #15 — Narrative & Dialogue Agent
// Prehistoric survival dialogue system — NPC conversations, quest hooks, lore delivery

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "SharedTypes.h"
#include "DialogueSystem.generated.h"

// ─── Enums ────────────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class ENarr_DialogueState : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Greeting        UMETA(DisplayName = "Greeting"),
    InConversation  UMETA(DisplayName = "In Conversation"),
    QuestOffer      UMETA(DisplayName = "Quest Offer"),
    QuestComplete   UMETA(DisplayName = "Quest Complete"),
    Farewell        UMETA(DisplayName = "Farewell")
};

UENUM(BlueprintType)
enum class ENarr_NPCRole : uint8
{
    Hunter      UMETA(DisplayName = "Hunter"),
    Elder       UMETA(DisplayName = "Elder"),
    Scout       UMETA(DisplayName = "Scout"),
    Crafter     UMETA(DisplayName = "Crafter"),
    Survivor    UMETA(DisplayName = "Survivor")
};

// ─── Structs ──────────────────────────────────────────────────────────────────

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
    FString TreeID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_DialogueLine> Lines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bIsQuestLinked;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString LinkedQuestID;

    FNarr_DialogueTree()
        : TreeID(TEXT(""))
        , bIsQuestLinked(false)
        , LinkedQuestID(TEXT(""))
    {}
};

// ─── NPC Dialogue Actor ───────────────────────────────────────────────────────

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ANarr_DialogueNPC : public AActor
{
    GENERATED_BODY()

public:
    ANarr_DialogueNPC();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Interaction trigger
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    USphereComponent* InteractionSphere;

    // NPC identity
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Identity")
    FString NPCName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Identity")
    ENarr_NPCRole NPCRole;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Identity")
    float InteractionRadius;

    // Dialogue state
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Narrative|State")
    ENarr_DialogueState CurrentState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Narrative|State")
    bool bPlayerInRange;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Narrative|State")
    int32 CurrentLineIndex;

    // Dialogue content
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    TArray<FNarr_DialogueTree> DialogueTrees;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    FString ActiveTreeID;

    // Quest hooks
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Quest")
    bool bOffersQuest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Quest")
    FString QuestID;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Narrative|Quest")
    bool bQuestAccepted;

    // Functions
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void StartDialogue();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void AdvanceDialogue();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void EndDialogue();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    FNarr_DialogueLine GetCurrentLine() const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool HasMoreLines() const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void AcceptQuest();

    UFUNCTION(BlueprintPure, Category = "Narrative")
    FString GetNPCDisplayName() const;

    UFUNCTION(BlueprintPure, Category = "Narrative")
    ENarr_DialogueState GetDialogueState() const;

private:
    UFUNCTION()
    void OnPlayerEnterRange(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnPlayerExitRange(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    FNarr_DialogueTree* FindActiveTree();
};
