// DialogueManager.h
// Agent #15 — Narrative & Dialogue Agent
// Manages NPC dialogue lines, trigger volumes, and voice audio cues
// CYCLE: PROD_CYCLE_AUTO_20260627_012

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "DialogueManager.generated.h"

UENUM(BlueprintType)
enum class ENarr_DialogueSpeaker : uint8
{
    TribalLeader    UMETA(DisplayName = "Tribal Leader"),
    Scout           UMETA(DisplayName = "Scout"),
    Elder           UMETA(DisplayName = "Elder"),
    Mentor          UMETA(DisplayName = "Mentor"),
    Unknown         UMETA(DisplayName = "Unknown")
};

USTRUCT(BlueprintType)
struct FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString LineText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_DialogueSpeaker Speaker;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TSoftObjectPtr<USoundBase> AudioCue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float DisplayDuration;

    FNarr_DialogueLine()
        : LineText(TEXT(""))
        , Speaker(ENarr_DialogueSpeaker::Unknown)
        , DisplayDuration(5.0f)
    {}
};

UCLASS(ClassGroup = (Narrative), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API ADialogueManager : public AActor
{
    GENERATED_BODY()

public:
    ADialogueManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Trigger sphere for proximity detection
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dialogue", meta = (AllowPrivateAccess = "true"))
    USphereComponent* TriggerSphere;

    // Which NPC speaker owns this trigger
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ENarr_DialogueSpeaker AssignedSpeaker;

    // Dialogue lines for this NPC
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FNarr_DialogueLine> DialogueLines;

    // Radius within which player triggers dialogue
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float TriggerRadius;

    // Has this dialogue been triggered this session
    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    bool bHasTriggered;

    // Current line index
    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    int32 CurrentLineIndex;

    // Advance to next dialogue line
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void AdvanceDialogue();

    // Reset dialogue to beginning
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void ResetDialogue();

    // Get current dialogue line
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    FNarr_DialogueLine GetCurrentLine() const;

    // Check if dialogue is active
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool IsDialogueActive() const;

protected:
    UFUNCTION()
    void OnPlayerEnterTrigger(UPrimitiveComponent* OverlappedComponent,
        AActor* OtherActor, UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

private:
    bool bDialogueActive;
    float LineTimer;
};
