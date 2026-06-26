#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Components/SphereComponent.h"
#include "DialogueSystem.generated.h"

// ─────────────────────────────────────────────────────────────────────────────
// Enums — global scope (UHT requirement)
// ─────────────────────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class ENarr_DialogueState : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Greeting        UMETA(DisplayName = "Greeting"),
    QuestOffer      UMETA(DisplayName = "QuestOffer"),
    QuestActive     UMETA(DisplayName = "QuestActive"),
    QuestComplete   UMETA(DisplayName = "QuestComplete"),
    Lore            UMETA(DisplayName = "Lore"),
    Warning         UMETA(DisplayName = "Warning"),
};

UENUM(BlueprintType)
enum class ENarr_NPCRole : uint8
{
    Elder           UMETA(DisplayName = "Elder"),
    Scout           UMETA(DisplayName = "Scout"),
    Craftsman       UMETA(DisplayName = "Craftsman"),
    Hunter          UMETA(DisplayName = "Hunter"),
    Survivor        UMETA(DisplayName = "Survivor"),
};

// ─────────────────────────────────────────────────────────────────────────────
// Structs — global scope
// ─────────────────────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_DialogueState TriggerState;

    // Audio URL from ElevenLabs TTS (set in editor or at runtime)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString AudioURL;

    // How long to display this line (seconds)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float DisplayDuration;

    FNarr_DialogueLine()
        : SpeakerName(TEXT("Unknown"))
        , DialogueText(TEXT(""))
        , TriggerState(ENarr_DialogueState::Idle)
        , AudioURL(TEXT(""))
        , DisplayDuration(4.0f)
    {}
};

USTRUCT(BlueprintType)
struct FNarr_DialogueTree
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString QuestID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_DialogueLine> Lines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    int32 CurrentLineIndex;

    FNarr_DialogueTree()
        : QuestID(TEXT(""))
        , CurrentLineIndex(0)
    {}
};

// ─────────────────────────────────────────────────────────────────────────────
// UNarr_DialogueComponent — attach to NPC actors
// ─────────────────────────────────────────────────────────────────────────────

UCLASS(ClassGroup = "Narrative", meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNarr_DialogueComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNarr_DialogueComponent();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_NPCRole NPCRole;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString NPCName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_DialogueState CurrentState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_DialogueTree> DialogueTrees;

    // Currently active tree index
    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    int32 ActiveTreeIndex;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void StartDialogue(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    FNarr_DialogueLine GetCurrentLine() const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool AdvanceLine();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void EndDialogue();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void SetState(ENarr_DialogueState NewState);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool IsDialogueActive() const;

    // Populate the Raptor Hunt quest dialogue tree
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Narrative")
    void InitRaptorHuntDialogue();

    // Populate the Crafting Introduction dialogue tree
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Narrative")
    void InitCraftingIntroDialogue();

protected:
    virtual void BeginPlay() override;

private:
    bool bDialogueActive;
};

// ─────────────────────────────────────────────────────────────────────────────
// ANarr_NPCDialogueActor — placeable NPC with proximity trigger
// ─────────────────────────────────────────────────────────────────────────────

UCLASS()
class TRANSPERSONALGAME_API ANarr_NPCDialogueActor : public AActor
{
    GENERATED_BODY()

public:
    ANarr_NPCDialogueActor();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components",
        meta = (AllowPrivateAccess = "true"))
    USphereComponent* ProximityTrigger;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components",
        meta = (AllowPrivateAccess = "true"))
    UNarr_DialogueComponent* DialogueComponent;

    // Interaction radius in cm
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float InteractionRadius;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void OnPlayerEnterRange(UPrimitiveComponent* OverlappedComp,
                            AActor* OtherActor,
                            UPrimitiveComponent* OtherComp,
                            int32 OtherBodyIndex,
                            bool bFromSweep,
                            const FHitResult& SweepResult);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void OnPlayerExitRange(UPrimitiveComponent* OverlappedComp,
                           AActor* OtherActor,
                           UPrimitiveComponent* OtherComp,
                           int32 OtherBodyIndex);

protected:
    virtual void BeginPlay() override;
};
