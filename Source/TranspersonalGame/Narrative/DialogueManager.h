// DialogueManager.h
// Agent #15 — Narrative & Dialogue Agent
// Manages NPC dialogue lines, voice audio cues, and narrative triggers in the world.
// All dialogue is survival-focused: danger warnings, resource tips, tribal lore.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "Sound/SoundBase.h"
#include "DialogueManager.generated.h"

// ─── Enums ────────────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class ENarr_DialogueTrigger : uint8
{
    OnProximity         UMETA(DisplayName = "On Proximity"),
    OnQuestStart        UMETA(DisplayName = "On Quest Start"),
    OnQuestComplete     UMETA(DisplayName = "On Quest Complete"),
    OnDangerNear        UMETA(DisplayName = "On Danger Near"),
    OnPlayerLowHealth   UMETA(DisplayName = "On Player Low Health"),
    OnFirstVisit        UMETA(DisplayName = "On First Visit"),
    OnItemPickup        UMETA(DisplayName = "On Item Pickup"),
    Manual              UMETA(DisplayName = "Manual")
};

UENUM(BlueprintType)
enum class ENarr_SpeakerRole : uint8
{
    TribalLeader    UMETA(DisplayName = "Tribal Leader"),
    Scout           UMETA(DisplayName = "Scout"),
    Elder           UMETA(DisplayName = "Elder"),
    Hunter          UMETA(DisplayName = "Hunter"),
    Survivor        UMETA(DisplayName = "Survivor"),
    PlayerVoice     UMETA(DisplayName = "Player Voice")
};

// ─── Structs ──────────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FNarr_DialogueLine : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FName LineID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ENarr_SpeakerRole SpeakerRole;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString LineText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TSoftObjectPtr<USoundBase> VoiceAudio;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float DisplayDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ENarr_DialogueTrigger TriggerType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bPlayOnce;

    FNarr_DialogueLine()
        : LineID(NAME_None)
        , SpeakerName(TEXT("Unknown"))
        , SpeakerRole(ENarr_SpeakerRole::Hunter)
        , LineText(TEXT(""))
        , DisplayDuration(5.0f)
        , TriggerType(ENarr_DialogueTrigger::OnProximity)
        , bPlayOnce(true)
    {}
};

USTRUCT(BlueprintType)
struct FNarr_ActiveDialogue
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    FName LineID;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    FString SpeakerName;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    FString LineText;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    float RemainingTime;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    bool bIsActive;

    FNarr_ActiveDialogue()
        : LineID(NAME_None)
        , SpeakerName(TEXT(""))
        , LineText(TEXT(""))
        , RemainingTime(0.0f)
        , bIsActive(false)
    {}
};

// ─── DialogueManager Actor ────────────────────────────────────────────────────

UCLASS(ClassGroup = "Narrative", meta = (DisplayName = "Dialogue Manager"))
class TRANSPERSONALGAME_API ADialogueManager : public AActor
{
    GENERATED_BODY()

public:
    ADialogueManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // ── Data ──────────────────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Data")
    UDataTable* DialogueDataTable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Data")
    TArray<FNarr_DialogueLine> HardcodedLines;

    // ── State ─────────────────────────────────────────────────────────────────

    UPROPERTY(BlueprintReadOnly, Category = "Narrative|State")
    FNarr_ActiveDialogue CurrentDialogue;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative|State")
    TArray<FName> PlayedLineIDs;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative|State")
    bool bDialogueActive;

    // ── Settings ──────────────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Settings")
    float ProximityTriggerRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Settings")
    float DangerCheckInterval;

    // ── Functions ─────────────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TriggerDialogueLine(FName LineID);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TriggerDialogueByRole(ENarr_SpeakerRole Role, ENarr_DialogueTrigger Trigger);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void StopCurrentDialogue();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool HasLineBeenPlayed(FName LineID) const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    FNarr_DialogueLine GetLineByID(FName LineID) const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    TArray<FNarr_DialogueLine> GetLinesByTrigger(ENarr_DialogueTrigger Trigger) const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void RegisterHardcodedLines();

    UFUNCTION(CallInEditor, Category = "Narrative|Debug")
    void DebugPlayRandomLine();

private:
    float DangerCheckTimer;

    void AdvanceDialogueTimer(float DeltaTime);
    void CheckProximityTriggers();
    void PopulateDefaultLines();
};
