#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DialogueSystem.generated.h"

UENUM(BlueprintType)
enum class ENarr_DialogueTriggerType : uint8
{
    ProximityEnter   UMETA(DisplayName = "Proximity Enter"),
    QuestStart       UMETA(DisplayName = "Quest Start"),
    QuestComplete    UMETA(DisplayName = "Quest Complete"),
    DinosaurSighted  UMETA(DisplayName = "Dinosaur Sighted"),
    DangerAlert      UMETA(DisplayName = "Danger Alert"),
    CampReached      UMETA(DisplayName = "Camp Reached"),
};

UENUM(BlueprintType)
enum class ENarr_SpeakerRole : uint8
{
    TribalLeader    UMETA(DisplayName = "Tribal Leader"),
    Scout           UMETA(DisplayName = "Scout"),
    Elder           UMETA(DisplayName = "Elder"),
    Hunter          UMETA(DisplayName = "Hunter"),
    Survivor        UMETA(DisplayName = "Survivor"),
    Narrator        UMETA(DisplayName = "Narrator"),
};

USTRUCT(BlueprintType)
struct FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_SpeakerRole SpeakerRole = ENarr_SpeakerRole::Hunter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString AudioURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float DisplayDuration = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bBlockPlayerMovement = false;
};

USTRUCT(BlueprintType)
struct FNarr_DialogueSequence
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString SequenceID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_DialogueTriggerType TriggerType = ENarr_DialogueTriggerType::ProximityEnter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_DialogueLine> Lines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bPlayOnce = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bHasPlayed = false;
};

/**
 * ANarr_DialogueZone — Proximity-triggered dialogue actor.
 * Place in level to trigger NPC voice lines when player enters radius.
 * Matches dialogue zones spawned in MinPlayableMap via Python.
 */
UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Dialogue Zone"))
class TRANSPERSONALGAME_API ANarr_DialogueZone : public AActor
{
    GENERATED_BODY()

public:
    ANarr_DialogueZone();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    FNarr_DialogueSequence DialogueSequence;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    float TriggerRadius = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    bool bShowDebugSphere = true;

    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    void TriggerDialogue();

    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    bool HasPlayerEntered() const;

    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    void ResetDialogue();

    UFUNCTION(BlueprintPure, Category = "Narrative|Dialogue")
    FString GetCurrentSpeakerName() const;

    UFUNCTION(BlueprintPure, Category = "Narrative|Dialogue")
    int32 GetLineCount() const;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    class USphereComponent* TriggerSphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    class UBillboardComponent* EditorIcon;

    UFUNCTION()
    void OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);

private:
    int32 CurrentLineIndex = 0;
    float LineTimer = 0.0f;
    bool bIsPlaying = false;
};
