#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "NarrativeDialogueManager.generated.h"

// ─── Enums ────────────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class ENarr_DialogueTrigger : uint8
{
    Proximity       UMETA(DisplayName = "Proximity"),
    QuestStart      UMETA(DisplayName = "QuestStart"),
    QuestComplete   UMETA(DisplayName = "QuestComplete"),
    DinosaurNearby  UMETA(DisplayName = "DinosaurNearby"),
    PlayerInjured   UMETA(DisplayName = "PlayerInjured"),
    NightFall       UMETA(DisplayName = "NightFall"),
    ResourceFound   UMETA(DisplayName = "ResourceFound"),
    TribeEvent      UMETA(DisplayName = "TribeEvent")
};

UENUM(BlueprintType)
enum class ENarr_SpeakerRole : uint8
{
    Narrator        UMETA(DisplayName = "Narrator"),
    Scout           UMETA(DisplayName = "Scout"),
    TribalElder     UMETA(DisplayName = "TribalElder"),
    HunterLeader    UMETA(DisplayName = "HunterLeader"),
    Survivor        UMETA(DisplayName = "Survivor"),
    Child           UMETA(DisplayName = "Child")
};

// ─── Structs ──────────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FNarr_DialogueLine : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString LineID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_SpeakerRole Speaker;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_DialogueTrigger TriggerType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float DisplayDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bIsUrgent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString AudioAssetPath;

    FNarr_DialogueLine()
        : LineID(TEXT(""))
        , DialogueText(TEXT(""))
        , Speaker(ENarr_SpeakerRole::Narrator)
        , TriggerType(ENarr_DialogueTrigger::Proximity)
        , DisplayDuration(4.0f)
        , bIsUrgent(false)
        , AudioAssetPath(TEXT(""))
    {}
};

USTRUCT(BlueprintType)
struct FNarr_ActiveDialogue
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    FString LineID;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    FString Text;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    ENarr_SpeakerRole Speaker;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    float RemainingTime;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    bool bIsUrgent;

    FNarr_ActiveDialogue()
        : LineID(TEXT(""))
        , Text(TEXT(""))
        , Speaker(ENarr_SpeakerRole::Narrator)
        , RemainingTime(0.0f)
        , bIsUrgent(false)
    {}
};

// ─── Class ────────────────────────────────────────────────────────────────────

UCLASS(ClassGroup = (Narrative), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API ANarrativeDialogueManager : public AActor
{
    GENERATED_BODY()

public:
    ANarrativeDialogueManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // ── Core dialogue API ──────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TriggerDialogue(ENarr_DialogueTrigger TriggerType, AActor* InstigatorActor);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void PlayDialogueLine(const FNarr_DialogueLine& Line);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void QueueDialogueLine(const FNarr_DialogueLine& Line);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void ClearDialogueQueue();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool IsDialoguePlaying() const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    FNarr_ActiveDialogue GetCurrentDialogue() const;

    // ── Survival context triggers ──────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Narrative|Survival")
    void OnDinosaurDetected(FName DinosaurSpecies, float DistanceMeters);

    UFUNCTION(BlueprintCallable, Category = "Narrative|Survival")
    void OnPlayerHealthCritical(float HealthPercent);

    UFUNCTION(BlueprintCallable, Category = "Narrative|Survival")
    void OnNightfallApproaching(float HoursUntilDark);

    UFUNCTION(BlueprintCallable, Category = "Narrative|Survival")
    void OnResourceDiscovered(FName ResourceType, int32 Quantity);

    UFUNCTION(BlueprintCallable, Category = "Narrative|Survival")
    void OnTribeEventOccurred(FName EventID);

    // ── Properties ────────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Config")
    float DialogueCooldownSeconds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Config")
    int32 MaxQueueSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Config")
    bool bEnableSubtitles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Config")
    float ProximityTriggerRadius;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Narrative|State",
        meta = (AllowPrivateAccess = "true"))
    FNarr_ActiveDialogue CurrentDialogue;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Narrative|State",
        meta = (AllowPrivateAccess = "true"))
    TArray<FNarr_DialogueLine> DialogueQueue;

private:
    float TimeSinceLastDialogue;
    float CurrentLineTimer;
    bool bDialoguePlaying;

    void AdvanceQueue();
    FNarr_DialogueLine BuildSurvivalLine(ENarr_DialogueTrigger Trigger,
                                          const FString& ContextText,
                                          ENarr_SpeakerRole Speaker,
                                          bool bUrgent);
};
