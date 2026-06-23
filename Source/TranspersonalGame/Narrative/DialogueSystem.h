// DialogueSystem.h
// Narrative & Dialogue Agent #15 — Transpersonal Game Studio
// Dialogue system for prehistoric survival NPCs
// PROD_CYCLE_AUTO_20260623_005

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "SharedTypes.h"
#include "DialogueSystem.generated.h"

// ─── Enums ────────────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class ENarr_DialogueType : uint8
{
    Warning         UMETA(DisplayName = "Warning"),
    QuestGive       UMETA(DisplayName = "QuestGive"),
    QuestComplete   UMETA(DisplayName = "QuestComplete"),
    Ambient         UMETA(DisplayName = "Ambient"),
    Lore            UMETA(DisplayName = "Lore"),
    Danger          UMETA(DisplayName = "Danger")
};

UENUM(BlueprintType)
enum class ENarr_NPCRole : uint8
{
    TribalElder     UMETA(DisplayName = "TribalElder"),
    Scout           UMETA(DisplayName = "Scout"),
    Crafter         UMETA(DisplayName = "Crafter"),
    Hunter          UMETA(DisplayName = "Hunter"),
    Survivor        UMETA(DisplayName = "Survivor")
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
    ENarr_DialogueType DialogueType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float DisplayDuration;

    FNarr_DialogueLine()
        : SpeakerName(TEXT("Unknown"))
        , LineText(TEXT(""))
        , DialogueType(ENarr_DialogueType::Ambient)
        , DisplayDuration(4.0f)
    {}
};

USTRUCT(BlueprintType)
struct FNarr_DialogueSequence
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FName SequenceID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_DialogueLine> Lines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bHasBeenPlayed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bCanRepeat;

    FNarr_DialogueSequence()
        : SequenceID(NAME_None)
        , bHasBeenPlayed(false)
        , bCanRepeat(false)
    {}
};

// ─── Dialogue Trigger Actor ────────────────────────────────────────────────────

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ANarr_DialogueTrigger : public AActor
{
    GENERATED_BODY()

public:
    ANarr_DialogueTrigger();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components",
        meta = (AllowPrivateAccess = "true"))
    UBoxComponent* TriggerVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FNarr_DialogueSequence DialogueSequence;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_DialogueType TriggerType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float TriggerRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bTriggerOnce;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void PlayDialogueSequence();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool HasBeenTriggered() const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void ResetTrigger();

private:
    bool bTriggered;

    UFUNCTION()
    void OnPlayerEnterTrigger(AActor* OverlappedActor, AActor* OtherActor);
};

// ─── NPC Quest Giver Actor ────────────────────────────────────────────────────

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ANarr_NPCQuestGiver : public AActor
{
    GENERATED_BODY()

public:
    ANarr_NPCQuestGiver();

    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components",
        meta = (AllowPrivateAccess = "true"))
    USphereComponent* InteractionSphere;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
    FString NPCName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
    ENarr_NPCRole NPCRole;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
    TArray<FNarr_DialogueSequence> AvailableDialogues;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
    float InteractionRange;

    UFUNCTION(BlueprintCallable, Category = "NPC")
    void Interact(AActor* Interactor);

    UFUNCTION(BlueprintCallable, Category = "NPC")
    FNarr_DialogueLine GetCurrentLine() const;

    UFUNCTION(BlueprintCallable, Category = "NPC")
    void AdvanceDialogue();

    UFUNCTION(BlueprintCallable, Category = "NPC")
    bool IsPlayerInRange(AActor* Player) const;

private:
    int32 CurrentDialogueIndex;
    int32 CurrentLineIndex;
    bool bInDialogue;
};

// ─── Narrative Manager (World Subsystem) ─────────────────────────────────────

UCLASS()
class TRANSPERSONALGAME_API UNarr_DialogueManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void RegisterDialogueTrigger(ANarr_DialogueTrigger* Trigger);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TriggerDialogue(FName SequenceID);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool IsDialogueActive() const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    FNarr_DialogueLine GetActiveDialogueLine() const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void AdvanceActiveDialogue();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void EndActiveDialogue();

private:
    TArray<ANarr_DialogueTrigger*> RegisteredTriggers;
    FNarr_DialogueSequence ActiveSequence;
    int32 ActiveLineIndex;
    bool bDialogueActive;
};
