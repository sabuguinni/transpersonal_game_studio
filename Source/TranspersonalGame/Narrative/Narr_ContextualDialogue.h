#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/TriggerBox.h"
#include "SharedTypes.h"
#include "Narr_ContextualDialogue.generated.h"

UENUM(BlueprintType)
enum class ENarr_DialogueContext : uint8
{
    None = 0,
    FirstFootprints,
    HerdSighting,
    PredatorNearby,
    AbandonedCamp,
    NightFall,
    LowHealth,
    Hungry,
    Thirsty,
    Exhausted,
    SafeZone
};

USTRUCT(BlueprintType)
struct FNarr_ContextualLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString VoicelineFile;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ENarr_DialogueContext Context;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bPlayOnce;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bHasBeenPlayed;

    FNarr_ContextualLine()
    {
        DialogueText = TEXT("");
        VoicelineFile = TEXT("");
        Context = ENarr_DialogueContext::None;
        Priority = 1.0f;
        bPlayOnce = true;
        bHasBeenPlayed = false;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNarr_ContextualDialogue : public UActorComponent
{
    GENERATED_BODY()

public:
    UNarr_ContextualDialogue();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Dialogue management
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TriggerContextualDialogue(ENarr_DialogueContext Context);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void AddContextualLine(const FNarr_ContextualLine& NewLine);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool HasDialogueForContext(ENarr_DialogueContext Context) const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void PlayDialogueLine(const FNarr_ContextualLine& Line);

    // Context detection
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void CheckPlayerContext();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    ENarr_DialogueContext GetCurrentPlayerContext() const;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue", meta = (AllowPrivateAccess = "true"))
    TArray<FNarr_ContextualLine> ContextualLines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue", meta = (AllowPrivateAccess = "true"))
    float ContextCheckInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue", meta = (AllowPrivateAccess = "true"))
    float LastContextCheckTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue", meta = (AllowPrivateAccess = "true"))
    ENarr_DialogueContext CurrentContext;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue", meta = (AllowPrivateAccess = "true"))
    bool bIsPlayingDialogue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue", meta = (AllowPrivateAccess = "true"))
    float DialogueCooldown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue", meta = (AllowPrivateAccess = "true"))
    float LastDialogueTime;

private:
    void InitializeDefaultDialogues();
    bool CanPlayDialogue() const;
    FNarr_ContextualLine* FindBestDialogueForContext(ENarr_DialogueContext Context);
    void CheckProximityToNarrativeTriggers();
    void CheckPlayerSurvivalStats();
};

#include "Narr_ContextualDialogue.generated.h"