// DialogueTriggerComponent.h
// Agent #15 — Narrative & Dialogue Agent
// Proximity-based dialogue trigger — attaches to NPC actors
// Plays audio cue when player enters range

#pragma once

#include "CoreMinimal.h"
#include "Components/SphereComponent.h"
#include "Sound/SoundBase.h"
#include "DialogueTriggerComponent.generated.h"

UENUM(BlueprintType)
enum class ENarr_DialogueState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Playing     UMETA(DisplayName = "Playing"),
    Cooldown    UMETA(DisplayName = "Cooldown"),
    Exhausted   UMETA(DisplayName = "Exhausted")
};

USTRUCT(BlueprintType)
struct FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TSoftObjectPtr<USoundBase> AudioAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float CooldownSeconds = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bPlayOnce = false;
};

UCLASS(ClassGroup = (Narrative), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UDialogueTriggerComponent : public USphereComponent
{
    GENERATED_BODY()

public:
    UDialogueTriggerComponent();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_DialogueLine> DialogueLines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float TriggerRadius = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString NPCName;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    ENarr_DialogueState CurrentState;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TriggerDialogue(int32 LineIndex = 0);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void ResetCooldown();

    UFUNCTION(BlueprintPure, Category = "Narrative")
    bool CanTrigger() const;

protected:
    virtual void BeginPlay() override;

    UFUNCTION()
    void OnPlayerEnterRange(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);

private:
    int32 CurrentLineIndex;
    float CooldownTimer;
    bool bHasPlayed;

    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
        FActorComponentTickFunction* ThisTickFunction) override;
};
