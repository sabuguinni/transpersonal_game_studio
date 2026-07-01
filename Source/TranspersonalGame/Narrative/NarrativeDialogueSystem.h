#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"
#include "NarrativeDialogueSystem.generated.h"

// ── Enums ────────────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class ENarr_DialogueTriggerType : uint8
{
    Proximity       UMETA(DisplayName = "Proximity"),
    QuestStart      UMETA(DisplayName = "Quest Start"),
    QuestComplete   UMETA(DisplayName = "Quest Complete"),
    CombatAlert     UMETA(DisplayName = "Combat Alert"),
    ResourceFound   UMETA(DisplayName = "Resource Found"),
    DayNightChange  UMETA(DisplayName = "Day Night Change"),
    PlayerDeath     UMETA(DisplayName = "Player Death"),
    FirstCraft      UMETA(DisplayName = "First Craft")
};

UENUM(BlueprintType)
enum class ENarr_SpeakerRole : uint8
{
    TribalElder     UMETA(DisplayName = "Tribal Elder"),
    Scout           UMETA(DisplayName = "Scout"),
    Crafter         UMETA(DisplayName = "Crafter"),
    Hunter          UMETA(DisplayName = "Hunter"),
    Survivor        UMETA(DisplayName = "Survivor"),
    Narrator        UMETA(DisplayName = "Narrator")
};

// ── Structs ───────────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_SpeakerRole SpeakerRole = ENarr_SpeakerRole::Narrator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString LineText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    USoundBase* VoiceAudio = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float DisplayDuration = 4.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bWaitForAudioEnd = true;
};

USTRUCT(BlueprintType)
struct FNarr_DialogueSequence
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FName SequenceID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_DialogueTriggerType TriggerType = ENarr_DialogueTriggerType::Proximity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_DialogueLine> Lines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bPlayOnce = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bHasPlayed = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float CooldownSeconds = 30.0f;
};

USTRUCT(BlueprintType)
struct FNarr_NPCDialogueEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FName NPCID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_SpeakerRole Role = ENarr_SpeakerRole::Survivor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_DialogueSequence> Sequences;
};

// ── Main Dialogue Actor ───────────────────────────────────────────────────────

UCLASS(ClassGroup = (Narrative), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API ANarr_DialogueTriggerActor : public AActor
{
    GENERATED_BODY()

public:
    ANarr_DialogueTriggerActor();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // ── Components ────────────────────────────────────────────────────────────
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Narrative", meta = (AllowPrivateAccess = "true"))
    UBoxComponent* TriggerVolume;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Narrative", meta = (AllowPrivateAccess = "true"))
    UAudioComponent* AudioComponent;

    // ── Configuration ─────────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FNarr_DialogueSequence DialogueSequence;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_DialogueTriggerType TriggerType = ENarr_DialogueTriggerType::Proximity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float TriggerRadius = 500.0f;

    // ── State ─────────────────────────────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    bool bIsPlaying = false;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    int32 CurrentLineIndex = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    float LineTimer = 0.0f;

    // ── Functions ─────────────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TriggerDialogue();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void StopDialogue();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void AdvanceLine();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool IsDialogueComplete() const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    FNarr_DialogueLine GetCurrentLine() const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void ResetDialogue();

    // ── Events ────────────────────────────────────────────────────────────────
    UFUNCTION(BlueprintImplementableEvent, Category = "Narrative")
    void OnDialogueLineStarted(const FNarr_DialogueLine& Line, int32 LineIndex);

    UFUNCTION(BlueprintImplementableEvent, Category = "Narrative")
    void OnDialogueSequenceComplete();

    UFUNCTION(BlueprintImplementableEvent, Category = "Narrative")
    void OnPlayerEnterTrigger();

private:
    UFUNCTION()
    void OnTriggerOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);

    float CooldownTimer = 0.0f;
    bool bOnCooldown = false;
};

// ── Dialogue Manager Subsystem ────────────────────────────────────────────────

UCLASS()
class TRANSPERSONALGAME_API UNarr_DialogueManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void RegisterDialogueTrigger(ANarr_DialogueTriggerActor* Trigger);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void UnregisterDialogueTrigger(ANarr_DialogueTriggerActor* Trigger);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void BroadcastGameEvent(ENarr_DialogueTriggerType EventType);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    int32 GetActiveDialogueCount() const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    TArray<FName> GetPlayedSequenceIDs() const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void MarkSequencePlayed(FName SequenceID);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool HasSequencePlayed(FName SequenceID) const;

private:
    UPROPERTY()
    TArray<ANarr_DialogueTriggerActor*> RegisteredTriggers;

    UPROPERTY()
    TSet<FName> PlayedSequenceIDs;
};
