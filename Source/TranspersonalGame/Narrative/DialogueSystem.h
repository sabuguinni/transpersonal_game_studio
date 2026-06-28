#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "DialogueSystem.generated.h"

// ============================================================
// ENarr_DialogueState — state machine for NPC conversations
// ============================================================
UENUM(BlueprintType)
enum class ENarr_DialogueState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Greeting    UMETA(DisplayName = "Greeting"),
    Active      UMETA(DisplayName = "Active"),
    Cooldown    UMETA(DisplayName = "Cooldown"),
    Completed   UMETA(DisplayName = "Completed")
};

// ============================================================
// ENarr_NPCRole — role of the NPC in the tribe
// ============================================================
UENUM(BlueprintType)
enum class ENarr_NPCRole : uint8
{
    ElderTracker    UMETA(DisplayName = "Elder Tracker"),
    ChiefHunter     UMETA(DisplayName = "Chief Hunter"),
    Craftmaster     UMETA(DisplayName = "Craftmaster"),
    ScoutRanger     UMETA(DisplayName = "Scout Ranger"),
    Survivor        UMETA(DisplayName = "Survivor")
};

// ============================================================
// FNarr_DialogueLine — a single voiced/subtitled line
// ============================================================
USTRUCT(BlueprintType)
struct FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FText LineText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString AudioAssetPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_NPCRole Speaker = ENarr_NPCRole::Survivor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float DisplayDuration = 4.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bRequiresSurvivalContext = false;
};

// ============================================================
// FNarr_DialogueSequence — ordered list of lines for one NPC
// ============================================================
USTRUCT(BlueprintType)
struct FNarr_DialogueSequence
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FName SequenceID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_DialogueLine> Lines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bRepeatable = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float CooldownSeconds = 120.0f;
};

// ============================================================
// ANarr_DialogueTrigger — proximity trigger that starts dialogue
// ============================================================
UCLASS(ClassGroup = "Narrative", meta = (DisplayName = "Dialogue Trigger"))
class TRANSPERSONALGAME_API ANarr_DialogueTrigger : public AActor
{
    GENERATED_BODY()

public:
    ANarr_DialogueTrigger();

    // Trigger volume
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components",
        meta = (AllowPrivateAccess = "true"))
    UBoxComponent* TriggerVolume;

    // NPC role this trigger represents
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_NPCRole NPCRole = ENarr_NPCRole::Survivor;

    // Current dialogue state
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Narrative")
    ENarr_DialogueState DialogueState = ENarr_DialogueState::Idle;

    // Dialogue sequence to play
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FNarr_DialogueSequence DialogueSequence;

    // Interaction radius in cm
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative",
        meta = (ClampMin = "100.0", ClampMax = "2000.0"))
    float InteractionRadius = 400.0f;

    // Whether this NPC has been spoken to
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Narrative")
    bool bHasBeenActivated = false;

    // Current line index in the sequence
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Narrative")
    int32 CurrentLineIndex = 0;

    // Time since last dialogue ended
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Narrative")
    float TimeSinceLastDialogue = 0.0f;

    // Blueprint event — called when player enters trigger range
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Narrative")
    void OnPlayerEnterRange(AActor* PlayerActor);

    // Blueprint event — called when dialogue line completes
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Narrative")
    void OnDialogueLineComplete(int32 LineIndex);

    // Advance to next line
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void AdvanceDialogue();

    // Reset dialogue to beginning
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void ResetDialogue();

    // Get current line text
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    FText GetCurrentLineText() const;

    // Get NPC display name
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    FString GetNPCDisplayName() const;

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UFUNCTION()
    void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);
};
