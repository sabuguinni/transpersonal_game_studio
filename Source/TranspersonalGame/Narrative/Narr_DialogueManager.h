#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/TriggerBox.h"
#include "Components/AudioComponent.h"
#include "SharedTypes.h"
#include "Narr_DialogueManager.generated.h"

UENUM(BlueprintType)
enum class ENarr_DialogueState : uint8
{
    Inactive,
    Ready,
    Playing,
    Completed,
    Cooldown
};

UENUM(BlueprintType)
enum class ENarr_DialogueTrigger : uint8
{
    OnEnterZone,
    OnExitZone,
    OnPlayerAction,
    OnTimeElapsed,
    OnQuestProgress
};

USTRUCT(BlueprintType)
struct FNarr_DialogueEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    class USoundBase* AudioClip;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ENarr_DialogueTrigger TriggerType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bIsRepeatable;

    FNarr_DialogueEntry()
    {
        CharacterName = TEXT("");
        DialogueText = TEXT("");
        AudioClip = nullptr;
        Duration = 5.0f;
        TriggerType = ENarr_DialogueTrigger::OnEnterZone;
        bIsRepeatable = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ANarr_DialogueManager : public AActor
{
    GENERATED_BODY()

public:
    ANarr_DialogueManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Core dialogue system
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue System")
    TArray<FNarr_DialogueEntry> DialogueEntries;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dialogue System")
    ENarr_DialogueState CurrentState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue System")
    float CooldownDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue System")
    float TriggerRadius;

    // Audio system
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio")
    class UAudioComponent* AudioComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float VolumeMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float PitchMultiplier;

    // Trigger system
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Triggers")
    class USphereComponent* TriggerSphere;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Triggers")
    bool bAutoTriggerOnEnter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Triggers")
    bool bRequireLineOfSight;

    // Player tracking
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player")
    class APawn* PlayerPawn;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player")
    float PlayerDetectionRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player")
    bool bTrackPlayerMovement;

    // Dialogue functions
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void PlayDialogue(int32 DialogueIndex);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void StopDialogue();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void AddDialogueEntry(const FNarr_DialogueEntry& NewEntry);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool IsDialoguePlaying() const;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    FNarr_DialogueEntry GetCurrentDialogue() const;

    // Trigger events
    UFUNCTION()
    void OnTriggerEnter(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnTriggerExit(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex);

    // Utility functions
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void SetDialogueState(ENarr_DialogueState NewState);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool CanTriggerDialogue() const;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void ResetCooldown();

private:
    float CooldownTimer;
    int32 CurrentDialogueIndex;
    bool bPlayerInRange;
    
    void UpdatePlayerDetection();
    void ProcessDialogueTriggers();
    bool CheckLineOfSight() const;
};

#include "Narr_DialogueManager.generated.h"