#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SharedTypes.h"
#include "DialogueSystem.generated.h"

// ============================================================
// Narr_ prefixed types — unique across project
// ============================================================

UENUM(BlueprintType)
enum class ENarr_DialogueTrigger : uint8
{
    Proximity       UMETA(DisplayName = "Proximity"),
    QuestStart      UMETA(DisplayName = "QuestStart"),
    QuestComplete   UMETA(DisplayName = "QuestComplete"),
    CombatEnter     UMETA(DisplayName = "CombatEnter"),
    DinoSighted     UMETA(DisplayName = "DinoSighted"),
    NightFall       UMETA(DisplayName = "NightFall"),
    PlayerDamaged   UMETA(DisplayName = "PlayerDamaged"),
    Manual          UMETA(DisplayName = "Manual"),
};

UENUM(BlueprintType)
enum class ENarr_CharacterRole : uint8
{
    Scout       UMETA(DisplayName = "Scout"),
    Elder       UMETA(DisplayName = "Elder"),
    Hunter      UMETA(DisplayName = "Hunter"),
    Tracker     UMETA(DisplayName = "Tracker"),
    Survivor    UMETA(DisplayName = "Survivor"),
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
    ENarr_CharacterRole SpeakerRole;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float DisplayDuration;

    FNarr_DialogueLine()
        : SpeakerName(TEXT("Unknown"))
        , LineText(TEXT(""))
        , AudioURL(TEXT(""))
        , SpeakerRole(ENarr_CharacterRole::Survivor)
        , DisplayDuration(4.0f)
    {}
};

USTRUCT(BlueprintType)
struct FNarr_DialogueSequence
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString SequenceID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_DialogueLine> Lines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_DialogueTrigger TriggerType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bPlayedThisSession;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bCanRepeat;

    FNarr_DialogueSequence()
        : SequenceID(TEXT(""))
        , TriggerType(ENarr_DialogueTrigger::Manual)
        , bPlayedThisSession(false)
        , bCanRepeat(false)
    {}
};

// ============================================================
// UNarr_DialogueSystem — ActorComponent
// Attach to any NPC or trigger volume to drive dialogue
// ============================================================

UCLASS(ClassGroup = (Narrative), meta = (BlueprintSpawnableComponent), DisplayName = "Dialogue System")
class TRANSPERSONALGAME_API UNarr_DialogueSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UNarr_DialogueSystem();

    // --- Configuration ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    TArray<FNarr_DialogueSequence> DialogueLibrary;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    float ProximityTriggerRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    bool bAutoTriggerOnProximity;

    // --- Runtime state ---
    UPROPERTY(BlueprintReadOnly, Category = "Narrative|State", meta = (AllowPrivateAccess = "true"))
    int32 CurrentLineIndex;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative|State", meta = (AllowPrivateAccess = "true"))
    bool bIsPlaying;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative|State", meta = (AllowPrivateAccess = "true"))
    FString ActiveSequenceID;

    // --- Public API ---
    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    bool TriggerDialogue(const FString& SequenceID);

    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    void TriggerByType(ENarr_DialogueTrigger TriggerType);

    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    void AdvanceLine();

    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    void StopDialogue();

    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    FNarr_DialogueLine GetCurrentLine() const;

    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    bool IsDialoguePlaying() const;

    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    void AddDialogueLine(const FString& SequenceID, const FNarr_DialogueLine& Line);

    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    int32 GetTotalLinesInSequence(const FString& SequenceID) const;

    // --- Lifecycle ---
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    FNarr_DialogueSequence* FindSequence(const FString& SequenceID);
    const FNarr_DialogueSequence* FindSequenceConst(const FString& SequenceID) const;

    float LineTimer;
    FNarr_DialogueSequence* ActiveSequence;
};
