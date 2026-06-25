#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SharedTypes.h"
#include "DialogueSystem.generated.h"

// ── Narr_ prefixed types to avoid collision with other agents ──

UENUM(BlueprintType)
enum class ENarr_DialogueTone : uint8
{
    Urgent      UMETA(DisplayName = "Urgent"),
    Informative UMETA(DisplayName = "Informative"),
    Warning     UMETA(DisplayName = "Warning"),
    Tactical    UMETA(DisplayName = "Tactical")
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
    ENarr_DialogueTone Tone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float DisplayDuration;

    FNarr_DialogueLine()
        : SpeakerName(TEXT("Unknown"))
        , LineText(TEXT(""))
        , Tone(ENarr_DialogueTone::Informative)
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
    bool bIsTriggered;

    FNarr_DialogueSequence()
        : SequenceID(TEXT(""))
        , bIsTriggered(false)
    {}
};

/**
 * ANarr_DialogueActor — placed in level near NPCs.
 * Player enters overlap → dialogue sequence plays.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ANarr_DialogueActor : public AActor
{
    GENERATED_BODY()

public:
    ANarr_DialogueActor();

    // ── NPC Identity ──
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|NPC")
    FString NPCName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|NPC")
    FString NPCRole;

    // ── Dialogue Content ──
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    TArray<FNarr_DialogueLine> DialogueLines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    int32 CurrentLineIndex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    bool bDialogueActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    float TriggerRadius;

    // ── Audio URLs (ElevenLabs generated) ──
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Audio")
    TArray<FString> AudioURLs;

    // ── Functions ──
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
    void AddDialogueLine(const FString& Speaker, const FString& Text, ENarr_DialogueTone Tone, float Duration);

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

private:
    float DialogueTimer;
};
