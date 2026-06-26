#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NarrativeDialogueManager.generated.h"

// ============================================================
// Narrative & Dialogue Manager — Agent #15
// Manages NPC dialogue trees, quest narrative state, voice cues
// ============================================================

UENUM(BlueprintType)
enum class ENarr_QuestArc : uint8
{
    NotStarted      UMETA(DisplayName = "Not Started"),
    TribeSurvival   UMETA(DisplayName = "Tribe Survival"),
    FirstHunt       UMETA(DisplayName = "First Hunt"),
    TriceratopsMigration UMETA(DisplayName = "Triceratops Migration"),
    PredatorTerritory    UMETA(DisplayName = "Predator Territory"),
    Completed       UMETA(DisplayName = "Completed")
};

UENUM(BlueprintType)
enum class ENarr_DialogueCondition : uint8
{
    Always          UMETA(DisplayName = "Always"),
    HasAxe          UMETA(DisplayName = "Has Stone Axe"),
    HasCampfire     UMETA(DisplayName = "Has Campfire"),
    FirstHuntDone   UMETA(DisplayName = "First Hunt Done"),
    DinoKillCount   UMETA(DisplayName = "Dino Kill Count"),
    DaysSurvived    UMETA(DisplayName = "Days Survived")
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
    FString AudioURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_DialogueCondition Condition = ENarr_DialogueCondition::Always;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float DisplayDuration = 5.0f;
};

USTRUCT(BlueprintType)
struct FNarr_DialogueTree
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString NPCName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_DialogueLine> Lines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_QuestArc RequiredArc = ENarr_QuestArc::NotStarted;
};

USTRUCT(BlueprintType)
struct FNarr_QuestState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_QuestArc CurrentArc = ENarr_QuestArc::NotStarted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    int32 DaysSurvived = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    int32 DinoKillCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bHasStoneAxe = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bHasCampfire = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bFirstHuntCompleted = false;
};

// ============================================================
// ANarr_DialogueNPC — Placeable NPC with dialogue tree
// ============================================================
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ANarr_DialogueNPC : public AActor
{
    GENERATED_BODY()

public:
    ANarr_DialogueNPC();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FNarr_DialogueTree DialogueTree;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float InteractionRadius = 300.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Narrative")
    class USphereComponent* InteractionSphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Narrative")
    class UStaticMeshComponent* NPCMesh;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void StartDialogue(AActor* Interactor);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    FNarr_DialogueLine GetNextLine(const FNarr_QuestState& QuestState);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void EndDialogue();

protected:
    virtual void BeginPlay() override;

    UFUNCTION()
    void OnPlayerEnterRange(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);

private:
    int32 CurrentLineIndex = 0;
    bool bIsInDialogue = false;
};

// ============================================================
// ANarr_QuestManager — Tracks narrative arc progression
// ============================================================
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ANarr_QuestManager : public AActor
{
    GENERATED_BODY()

public:
    ANarr_QuestManager();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FNarr_QuestState QuestState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<ANarr_DialogueNPC*> RegisteredNPCs;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void AdvanceArc(ENarr_QuestArc NewArc);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void OnPlayerCraftedAxe();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void OnPlayerCraftedCampfire();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void OnPlayerKilledDino();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void OnDaySurvived();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    ENarr_QuestArc GetCurrentArc() const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    FString GetArcDisplayName(ENarr_QuestArc Arc) const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool CheckArcAdvancement();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

private:
    float DayTimer = 0.0f;
    float DayDuration = 300.0f; // 5 minutes = 1 in-game day
};
