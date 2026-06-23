#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "DialogueSystem.generated.h"

// ============================================================
// Narrative & Dialogue Agent #15 — DialogueSystem
// Proximity-based dialogue trigger for prehistoric survival game
// ============================================================

UENUM(BlueprintType)
enum class ENarr_DialogueContext : uint8
{
    None            UMETA(DisplayName = "None"),
    RaptorWarning   UMETA(DisplayName = "Raptor Warning"),
    HerdObservation UMETA(DisplayName = "Herd Observation"),
    ScoutReport     UMETA(DisplayName = "Scout Report"),
    DangerZone      UMETA(DisplayName = "Danger Zone"),
    ResourceFound   UMETA(DisplayName = "Resource Found"),
    ShelterNearby   UMETA(DisplayName = "Shelter Nearby")
};

USTRUCT(BlueprintType)
struct FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString SpeakerID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString LineText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float DisplayDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_DialogueContext Context;

    FNarr_DialogueLine()
        : SpeakerID(TEXT("Scout"))
        , LineText(TEXT(""))
        , DisplayDuration(4.0f)
        , Context(ENarr_DialogueContext::None)
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
    bool bOneShot;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float CooldownSeconds;

    FNarr_DialogueSequence()
        : SequenceID(TEXT(""))
        , bOneShot(true)
        , CooldownSeconds(30.0f)
    {}
};

/**
 * ANarr_DialogueTrigger
 * Proximity-based actor that fires dialogue when the player enters its radius.
 * Designed for prehistoric survival game — no spiritual content.
 * Dialogue covers: raptor sightings, herd movements, resource locations, danger zones.
 */
UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Narrative Dialogue Trigger"))
class TRANSPERSONALGAME_API ANarr_DialogueTrigger : public AActor
{
    GENERATED_BODY()

public:
    ANarr_DialogueTrigger();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Trigger sphere — player proximity detection
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Narrative", meta = (AllowPrivateAccess = "true"))
    USphereComponent* TriggerSphere;

    // The dialogue sequence to play when triggered
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FNarr_DialogueSequence DialogueSequence;

    // Context tag for this trigger
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_DialogueContext TriggerContext;

    // Trigger radius in cm
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative", meta = (ClampMin = "100.0", ClampMax = "5000.0"))
    float TriggerRadius;

    // Whether this trigger has already fired (for one-shot)
    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    bool bHasFired;

    // Time since last fire (for cooldown)
    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    float TimeSinceLastFire;

    // Called when player enters trigger radius
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Narrative")
    void OnPlayerEnterTrigger(AActor* OverlappingActor);
    virtual void OnPlayerEnterTrigger_Implementation(AActor* OverlappingActor);

    // Called when player exits trigger radius
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Narrative")
    void OnPlayerExitTrigger(AActor* OverlappingActor);
    virtual void OnPlayerExitTrigger_Implementation(AActor* OverlappingActor);

    // Manually fire the dialogue sequence
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Narrative")
    void FireDialogueSequence();

    // Reset the trigger so it can fire again
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Narrative")
    void ResetTrigger();

    // Get current line index
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    int32 GetCurrentLineIndex() const { return CurrentLineIndex; }

    // Get total lines in sequence
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    int32 GetTotalLines() const { return DialogueSequence.Lines.Num(); }

private:
    UFUNCTION()
    void OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    void AdvanceDialogue();

    int32 CurrentLineIndex;
    FTimerHandle DialogueTimerHandle;
};
