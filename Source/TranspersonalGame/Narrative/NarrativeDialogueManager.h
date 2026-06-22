#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NarrativeDialogueManager.generated.h"

UENUM(BlueprintType)
enum class ENarr_NPCRole : uint8
{
    TribalElder     UMETA(DisplayName = "Tribal Elder"),
    HunterScout     UMETA(DisplayName = "Hunter Scout"),
    Forager         UMETA(DisplayName = "Forager"),
    Ambient         UMETA(DisplayName = "Ambient")
};

UENUM(BlueprintType)
enum class ENarr_DialogueState : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Greeting        UMETA(DisplayName = "Greeting"),
    QuestAssign     UMETA(DisplayName = "Quest Assign"),
    QuestComplete   UMETA(DisplayName = "Quest Complete"),
    Warning         UMETA(DisplayName = "Warning"),
    Lore            UMETA(DisplayName = "Lore")
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
    ENarr_DialogueState State;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float DisplayDuration;

    FNarr_DialogueLine()
        : SpeakerName(TEXT("Unknown"))
        , LineText(TEXT(""))
        , State(ENarr_DialogueState::Idle)
        , DisplayDuration(4.0f)
    {}
};

USTRUCT(BlueprintType)
struct FNarr_NPCDialogueSet
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString NPCLabel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_NPCRole Role;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_DialogueLine> Lines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    int32 CurrentLineIndex;

    FNarr_NPCDialogueSet()
        : NPCLabel(TEXT(""))
        , Role(ENarr_NPCRole::Ambient)
        , CurrentLineIndex(0)
    {}
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ANarrativeDialogueManager : public AActor
{
    GENERATED_BODY()

public:
    ANarrativeDialogueManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Config")
    float ProximityTriggerRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Config")
    float AmbientLineCooldown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Data")
    TArray<FNarr_NPCDialogueSet> NPCDialogueSets;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative|State")
    ENarr_DialogueState CurrentGlobalState;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative|State")
    FString ActiveSpeaker;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative|State")
    FString ActiveLineText;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void InitializeDialogueSets();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    FNarr_DialogueLine GetNextLine(const FString& NPCLabel);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TriggerDialogue(const FString& NPCLabel, ENarr_DialogueState State);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void AdvanceDialogue(const FString& NPCLabel);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool IsPlayerNearNPC(const FString& NPCLabel, float Radius) const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    TArray<FString> GetAmbientLines() const;

    UFUNCTION(CallInEditor, Category = "Narrative|Debug")
    void DebugPrintAllDialogue();

private:
    float AmbientCooldownTimer;
    int32 AmbientLineIndex;

    void SetupElderDialogue(FNarr_NPCDialogueSet& Set);
    void SetupScoutDialogue(FNarr_NPCDialogueSet& Set);
    void SetupForagerDialogue(FNarr_NPCDialogueSet& Set);
};
