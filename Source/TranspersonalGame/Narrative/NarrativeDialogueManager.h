// NarrativeDialogueManager.h
// Agent #15 — Narrative & Dialogue Agent
// CYCLE: PROD_CYCLE_AUTO_20260627_005
// Manages NPC dialogue lines, voice audio references, and tribal lore delivery

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NarrativeDialogueManager.generated.h"

UENUM(BlueprintType)
enum class ENarr_DialogueSpeaker : uint8
{
    TribeElder      UMETA(DisplayName = "Tribe Elder"),
    HuntLeader      UMETA(DisplayName = "Hunt Leader"),
    Scout           UMETA(DisplayName = "Scout"),
    TribalWarrior   UMETA(DisplayName = "Tribal Warrior"),
    Unknown         UMETA(DisplayName = "Unknown")
};

UENUM(BlueprintType)
enum class ENarr_DialogueTrigger : uint8
{
    OnApproach      UMETA(DisplayName = "On Player Approach"),
    OnQuestStart    UMETA(DisplayName = "On Quest Start"),
    OnQuestComplete UMETA(DisplayName = "On Quest Complete"),
    OnDanger        UMETA(DisplayName = "On Danger Detected"),
    OnIdle          UMETA(DisplayName = "On Idle")
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
    ENarr_DialogueTrigger Trigger;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString AudioURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float Duration;

    FNarr_DialogueLine()
        : LineText(TEXT(""))
        , Speaker(ENarr_DialogueSpeaker::Unknown)
        , Trigger(ENarr_DialogueTrigger::OnIdle)
        , AudioURL(TEXT(""))
        , Duration(0.0f)
    {}
};

USTRUCT(BlueprintType)
struct FNarr_NPCVoiceProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_DialogueSpeaker Speaker;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_DialogueLine> DialogueLines;

    FNarr_NPCVoiceProfile()
        : Speaker(ENarr_DialogueSpeaker::Unknown)
        , CharacterName(TEXT(""))
    {}
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ANarrativeDialogueManager : public AActor
{
    GENERATED_BODY()

public:
    ANarrativeDialogueManager();

    virtual void BeginPlay() override;

    // All registered NPC voice profiles
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Profiles")
    TArray<FNarr_NPCVoiceProfile> NPCProfiles;

    // Currently active dialogue line
    UPROPERTY(BlueprintReadOnly, Category = "Narrative|State")
    FNarr_DialogueLine ActiveLine;

    // Whether a dialogue is currently playing
    UPROPERTY(BlueprintReadOnly, Category = "Narrative|State")
    bool bDialogueActive;

    // Trigger a dialogue line by speaker and trigger type
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TriggerDialogue(ENarr_DialogueSpeaker Speaker, ENarr_DialogueTrigger Trigger);

    // End current dialogue
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void EndDialogue();

    // Get all lines for a specific speaker
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    TArray<FNarr_DialogueLine> GetLinesForSpeaker(ENarr_DialogueSpeaker Speaker) const;

    // Register a new NPC profile at runtime
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void RegisterNPCProfile(const FNarr_NPCVoiceProfile& Profile);

    // Load default dialogue lines from game bible
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Narrative")
    void LoadDefaultDialogueLines();

protected:
    void InitializeDefaultProfiles();
};
