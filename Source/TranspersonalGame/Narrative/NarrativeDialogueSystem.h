#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "NarrativeDialogueSystem.generated.h"

// ============================================================
// Narrative & Dialogue System — Agent #15
// Prehistoric survival game dialogue and quest narrative types
// ============================================================

UENUM(BlueprintType)
enum class ENarr_DialogueTone : uint8
{
    Urgent       UMETA(DisplayName = "Urgent"),
    Cautious     UMETA(DisplayName = "Cautious"),
    Informative  UMETA(DisplayName = "Informative"),
    Warning      UMETA(DisplayName = "Warning"),
    Neutral      UMETA(DisplayName = "Neutral")
};

UENUM(BlueprintType)
enum class ENarr_NPCRole : uint8
{
    TribalElder  UMETA(DisplayName = "Tribal Elder"),
    Scout        UMETA(DisplayName = "Scout"),
    Hunter       UMETA(DisplayName = "Hunter"),
    Craftsman    UMETA(DisplayName = "Craftsman"),
    Survivor     UMETA(DisplayName = "Survivor")
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
    ENarr_DialogueTone Tone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float DisplayDuration;

    FNarr_DialogueLine()
        : SpeakerID(TEXT("Unknown"))
        , LineText(TEXT(""))
        , Tone(ENarr_DialogueTone::Neutral)
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
    bool bIsRepeatable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString RequiredQuestState;

    FNarr_DialogueSequence()
        : SequenceID(TEXT(""))
        , bIsRepeatable(false)
        , RequiredQuestState(TEXT(""))
    {}
};

USTRUCT(BlueprintType)
struct FNarr_NPCProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString NPCID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_NPCRole Role;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_DialogueSequence> DialogueTrees;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bHasBeenMet;

    FNarr_NPCProfile()
        : NPCID(TEXT(""))
        , DisplayName(TEXT("Unknown"))
        , Role(ENarr_NPCRole::Survivor)
        , bHasBeenMet(false)
    {}
};

// ============================================================
// ANarr_DialogueTrigger — proximity-based dialogue trigger actor
// ============================================================
UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Narrative Dialogue Trigger"))
class TRANSPERSONALGAME_API ANarr_DialogueTrigger : public AActor
{
    GENERATED_BODY()

public:
    ANarr_DialogueTrigger();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Narrative",
        meta = (AllowPrivateAccess = "true"))
    USphereComponent* TriggerSphere;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FNarr_NPCProfile NPCProfile;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float InteractionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bTriggerOnce;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    bool bHasTriggered;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TriggerDialogue(AActor* Interactor);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    FNarr_DialogueLine GetNextDialogueLine();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void ResetDialogue();

protected:
    virtual void BeginPlay() override;

private:
    int32 CurrentLineIndex;

    UFUNCTION()
    void OnSphereOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);
};
