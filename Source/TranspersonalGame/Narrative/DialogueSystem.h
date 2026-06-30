#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "DialogueSystem.generated.h"

// ============================================================
// Narrative & Dialogue Agent #15 — DialogueSystem.h
// Cycle: PROD_CYCLE_AUTO_20260630_005
// Elder Kael NPC dialogue tree — "First Craft" quest arc
// ============================================================

UENUM(BlueprintType)
enum class ENarr_DialogueState : uint8
{
    Greeting        UMETA(DisplayName = "Greeting"),
    QuestIntro      UMETA(DisplayName = "Quest Intro"),
    CraftingGuide   UMETA(DisplayName = "Crafting Guide"),
    DangerWarning   UMETA(DisplayName = "Danger Warning"),
    QuestComplete   UMETA(DisplayName = "Quest Complete"),
    Idle            UMETA(DisplayName = "Idle")
};

UENUM(BlueprintType)
enum class ENarr_QuestStage : uint8
{
    NotStarted      UMETA(DisplayName = "Not Started"),
    GatherResources UMETA(DisplayName = "Gather Resources"),
    CraftAxe        UMETA(DisplayName = "Craft Axe"),
    CraftFire       UMETA(DisplayName = "Craft Fire"),
    ReturnToElder   UMETA(DisplayName = "Return To Elder"),
    Completed       UMETA(DisplayName = "Completed")
};

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

    FNarr_DialogueLine()
        : SpeakerName(TEXT("Elder Kael"))
        , LineText(TEXT(""))
        , TriggerState(ENarr_DialogueState::Idle)
        , DisplayDuration(5.0f)
    {}
};

USTRUCT(BlueprintType)
struct FNarr_QuestObjective
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString ObjectiveText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    int32 RequiredCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    int32 CurrentCount;

    FNarr_QuestObjective()
        : ObjectiveText(TEXT(""))
        , bCompleted(false)
        , RequiredCount(1)
        , CurrentCount(0)
    {}
};

// ============================================================
// ANarr_ElderKaelNPC — Quest-giving NPC with dialogue tree
// ============================================================
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ANarr_ElderKaelNPC : public AActor
{
    GENERATED_BODY()

public:
    ANarr_ElderKaelNPC();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Interaction trigger
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    USphereComponent* InteractionSphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    UStaticMeshComponent* NPCMesh;

    // Current dialogue state
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_DialogueState CurrentDialogueState;

    // Current quest stage
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_QuestStage CurrentQuestStage;

    // All dialogue lines for Elder Kael
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_DialogueLine> DialogueLines;

    // Active quest objectives
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_QuestObjective> ActiveObjectives;

    // Interaction radius
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float InteractionRadius;

    // Is player currently in dialogue range
    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    bool bPlayerInRange;

    // Dialogue display timer
    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    float DialogueTimer;

    // Current line index
    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    int32 CurrentLineIndex;

    // Functions
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void InitializeDialogueTree();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TriggerDialogue(ENarr_DialogueState State);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    FNarr_DialogueLine GetCurrentLine() const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void AdvanceDialogue();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void OnQuestStageAdvanced(ENarr_QuestStage NewStage);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void UpdateObjective(int32 ObjectiveIndex, int32 NewCount);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool AreAllObjectivesComplete() const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    FString GetObjectiveSummary() const;

private:
    UFUNCTION()
    void OnPlayerEnterRange(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnPlayerExitRange(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    void InitializeObjectives();
};
