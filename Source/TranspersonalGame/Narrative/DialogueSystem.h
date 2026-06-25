// DialogueSystem.h
// Agent #15 — Narrative & Dialogue Agent
// Cycle: PROD_CYCLE_AUTO_20260625_011
// Dialogue system for NPC interactions in prehistoric survival game

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "DialogueSystem.generated.h"

// ─────────────────────────────────────────────────────────────
// ENUMS — must be at global scope (UHT rule)
// ─────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class ENarr_NPCRole : uint8
{
    Elder       UMETA(DisplayName = "Elder"),
    Scout       UMETA(DisplayName = "Scout"),
    Hunter      UMETA(DisplayName = "Hunter"),
    Crafter     UMETA(DisplayName = "Crafter"),
    Lookout     UMETA(DisplayName = "Lookout"),
    Healer      UMETA(DisplayName = "Healer")
};

UENUM(BlueprintType)
enum class ENarr_DialogueTrigger : uint8
{
    Proximity       UMETA(DisplayName = "Proximity"),
    QuestStart      UMETA(DisplayName = "QuestStart"),
    QuestComplete   UMETA(DisplayName = "QuestComplete"),
    PlayerLowHealth UMETA(DisplayName = "PlayerLowHealth"),
    DinosaureNearby UMETA(DisplayName = "DinosaurNearby"),
    NightFall       UMETA(DisplayName = "NightFall")
};

// ─────────────────────────────────────────────────────────────
// STRUCTS
// ─────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString LineText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_DialogueTrigger TriggerCondition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float DisplayDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString AudioAssetPath;

    FNarr_DialogueLine()
        : SpeakerName(TEXT("Unknown"))
        , LineText(TEXT(""))
        , TriggerCondition(ENarr_DialogueTrigger::Proximity)
        , DisplayDuration(4.0f)
        , AudioAssetPath(TEXT(""))
    {}
};

USTRUCT(BlueprintType)
struct FNarr_NPCData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString NPCName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_NPCRole Role;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_DialogueLine> DialogueLines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bHasBeenSpokenTo;

    FNarr_NPCData()
        : NPCName(TEXT("Unknown"))
        , Role(ENarr_NPCRole::Elder)
        , bHasBeenSpokenTo(false)
    {}
};

// ─────────────────────────────────────────────────────────────
// MAIN DIALOGUE ACTOR CLASS
// ─────────────────────────────────────────────────────────────

UCLASS(ClassGroup = "Narrative", meta = (DisplayName = "Dialogue NPC Actor"))
class TRANSPERSONALGAME_API ANarr_DialogueActor : public AActor
{
    GENERATED_BODY()

public:
    ANarr_DialogueActor();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // ── Components ──────────────────────────────────────────
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components",
        meta = (AllowPrivateAccess = "true"))
    USphereComponent* InteractionSphere;

    // ── NPC Data ─────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FNarr_NPCData NPCData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float InteractionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bIsPlayerInRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    int32 CurrentDialogueIndex;

    // ── Functions ─────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TriggerDialogue(ENarr_DialogueTrigger Trigger);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    FNarr_DialogueLine GetCurrentDialogueLine() const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void AdvanceDialogue();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool HasMoreDialogue() const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void ResetDialogue();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    FString GetNPCName() const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    ENarr_NPCRole GetNPCRole() const;

    // ── Overlap callbacks ─────────────────────────────────────
    UFUNCTION()
    void OnPlayerEnterRange(UPrimitiveComponent* OverlappedComp,
        AActor* OtherActor, UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnPlayerExitRange(UPrimitiveComponent* OverlappedComp,
        AActor* OtherActor, UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex);

private:
    void InitialiseDefaultDialogue();
};
