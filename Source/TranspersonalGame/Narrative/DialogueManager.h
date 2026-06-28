// DialogueManager.h
// Agent #15 — Narrative & Dialogue Agent
// Manages NPC dialogue triggers, voice line playback, and contextual narrative events.
// Prefix: Narr_

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Sound/SoundBase.h"
#include "DialogueManager.generated.h"

UENUM(BlueprintType)
enum class ENarr_DialogueTrigger : uint8
{
    None            UMETA(DisplayName = "None"),
    DangerNear      UMETA(DisplayName = "DangerNear"),
    HuntSuccess     UMETA(DisplayName = "HuntSuccess"),
    HuntFailed      UMETA(DisplayName = "HuntFailed"),
    PlayerLowHealth UMETA(DisplayName = "PlayerLowHealth"),
    PlayerLowHunger UMETA(DisplayName = "PlayerLowHunger"),
    NestDiscovered  UMETA(DisplayName = "NestDiscovered"),
    CampFound       UMETA(DisplayName = "CampFound"),
    StampedeWarning UMETA(DisplayName = "StampedeWarning"),
    NightFalling    UMETA(DisplayName = "NightFalling"),
    PlayerDied      UMETA(DisplayName = "PlayerDied")
};

UENUM(BlueprintType)
enum class ENarr_CharacterRole : uint8
{
    HunterElder     UMETA(DisplayName = "HunterElder"),
    HuntCaller      UMETA(DisplayName = "HuntCaller"),
    TrailReader     UMETA(DisplayName = "TrailReader"),
    ScoutLeader     UMETA(DisplayName = "ScoutLeader"),
    SurvivalElder   UMETA(DisplayName = "SurvivalElder"),
    Narrator        UMETA(DisplayName = "Narrator")
};

USTRUCT(BlueprintType)
struct FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_DialogueTrigger Trigger = ENarr_DialogueTrigger::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_CharacterRole Speaker = ENarr_CharacterRole::Narrator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString LineText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    USoundBase* VoiceAsset = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float CooldownSeconds = 120.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float Priority = 1.0f;
};

USTRUCT(BlueprintType)
struct FNarr_DialogueState
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    ENarr_DialogueTrigger LastTrigger = ENarr_DialogueTrigger::None;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    float LastPlayedTime = -999.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    bool bIsPlaying = false;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    FString CurrentLineText;
};

UCLASS(ClassGroup = (Narrative), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNarr_DialogueManagerComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNarr_DialogueManagerComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Trigger a dialogue event — picks best matching line and plays it
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TriggerDialogue(ENarr_DialogueTrigger Trigger);

    // Register a dialogue line at runtime
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void RegisterDialogueLine(FNarr_DialogueLine Line);

    // Get current dialogue state
    UFUNCTION(BlueprintPure, Category = "Narrative")
    FNarr_DialogueState GetDialogueState() const;

    // Check if a trigger is on cooldown
    UFUNCTION(BlueprintPure, Category = "Narrative")
    bool IsTriggerOnCooldown(ENarr_DialogueTrigger Trigger) const;

    // Force stop any playing dialogue
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void StopDialogue();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_DialogueLine> DialogueLibrary;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float GlobalCooldownSeconds = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bEnableSubtitles = true;

private:
    FNarr_DialogueState CurrentState;
    TMap<ENarr_DialogueTrigger, float> TriggerCooldownMap;
    float ElapsedTime = 0.0f;

    FNarr_DialogueLine* FindBestLine(ENarr_DialogueTrigger Trigger);
    void PlayLine(FNarr_DialogueLine& Line);
};
