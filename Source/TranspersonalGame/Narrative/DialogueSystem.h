#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "DialogueSystem.generated.h"

UENUM(BlueprintType)
enum class ENarr_DialogueTriggerType : uint8
{
    NPC_Greeting    UMETA(DisplayName = "NPC Greeting"),
    QuestOffer      UMETA(DisplayName = "Quest Offer"),
    DangerWarning   UMETA(DisplayName = "Danger Warning"),
    LoreReveal      UMETA(DisplayName = "Lore Reveal"),
    HuntingTip      UMETA(DisplayName = "Hunting Tip")
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
    float DisplayDuration;

    FNarr_DialogueLine()
        : SpeakerName(TEXT("Unknown"))
        , LineText(TEXT(""))
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
    bool bPlayedOnce;

    FNarr_DialogueSequence()
        : SequenceID(TEXT(""))
        , bPlayedOnce(false)
    {}
};

/**
 * ANarr_DialogueTrigger — proximity-based dialogue trigger actor.
 * Player enters sphere radius → dialogue sequence fires.
 * Realistic survival dialogue only: hunting tips, danger warnings, lore.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ANarr_DialogueTrigger : public AActor
{
    GENERATED_BODY()

public:
    ANarr_DialogueTrigger();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TriggerDialogue(AActor* InstigatorActor);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void ResetTrigger();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool HasBeenTriggered() const { return bHasTriggered; }

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    FNarr_DialogueSequence GetCurrentSequence() const { return DialogueSequence; }

protected:
    UFUNCTION()
    void OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);

private:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    USphereComponent* TriggerSphere;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative", meta = (AllowPrivateAccess = "true"))
    ENarr_DialogueTriggerType TriggerType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative", meta = (AllowPrivateAccess = "true"))
    FNarr_DialogueSequence DialogueSequence;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative", meta = (AllowPrivateAccess = "true"))
    bool bOneShot;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative", meta = (AllowPrivateAccess = "true"))
    float TriggerRadius;

    bool bHasTriggered;
};
