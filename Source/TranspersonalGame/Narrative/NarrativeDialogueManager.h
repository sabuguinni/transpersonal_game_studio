#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "NarrativeDialogueManager.generated.h"

UENUM(BlueprintType)
enum class ENarr_DialogueType : uint8
{
    TacticalWarning UMETA(DisplayName = "Tactical Warning"),
    FieldResearch UMETA(DisplayName = "Field Research"),
    SafetyAlert UMETA(DisplayName = "Safety Alert"),
    TribalWisdom UMETA(DisplayName = "Tribal Wisdom"),
    EnvironmentalInfo UMETA(DisplayName = "Environmental Info"),
    CombatGuidance UMETA(DisplayName = "Combat Guidance")
};

UENUM(BlueprintType)
enum class ENarr_DialogueTrigger : uint8
{
    PlayerProximity UMETA(DisplayName = "Player Proximity"),
    DinosaurSighting UMETA(DisplayName = "Dinosaur Sighting"),
    DangerZone UMETA(DisplayName = "Danger Zone"),
    QuestProgress UMETA(DisplayName = "Quest Progress"),
    EnvironmentalChange UMETA(DisplayName = "Environmental Change"),
    ManualTrigger UMETA(DisplayName = "Manual Trigger")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DialogueID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ENarr_DialogueType DialogueType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ENarr_DialogueTrigger TriggerType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString AudioURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    int32 Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bIsRepeatable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bHasBeenPlayed;

    FNarr_DialogueEntry()
    {
        DialogueID = TEXT("");
        DialogueType = ENarr_DialogueType::TacticalWarning;
        TriggerType = ENarr_DialogueTrigger::PlayerProximity;
        DialogueText = TEXT("");
        AudioURL = TEXT("");
        CharacterName = TEXT("");
        Duration = 0.0f;
        Priority = 1;
        bIsRepeatable = false;
        bHasBeenPlayed = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ANarrativeDialogueManager : public AActor
{
    GENERATED_BODY()

public:
    ANarrativeDialogueManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UAudioComponent* DialogueAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UTextRenderComponent* SubtitleText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue System")
    TArray<FNarr_DialogueEntry> DialogueDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue System")
    FNarr_DialogueEntry CurrentDialogue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue System")
    bool bIsPlayingDialogue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue System")
    float DialogueVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue System")
    float SubtitleDisplayTime;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Dialogue System")
    void InitializeDialogueDatabase();

    UFUNCTION(BlueprintCallable, Category = "Dialogue System")
    void PlayDialogue(const FString& DialogueID);

    UFUNCTION(BlueprintCallable, Category = "Dialogue System")
    void PlayDialogueByType(ENarr_DialogueType DialogueType);

    UFUNCTION(BlueprintCallable, Category = "Dialogue System")
    void StopCurrentDialogue();

    UFUNCTION(BlueprintCallable, Category = "Dialogue System")
    bool TriggerDialogue(ENarr_DialogueTrigger TriggerType, const FVector& TriggerLocation);

    UFUNCTION(BlueprintCallable, Category = "Dialogue System")
    void AddDialogueEntry(const FNarr_DialogueEntry& NewEntry);

    UFUNCTION(BlueprintCallable, Category = "Dialogue System")
    TArray<FNarr_DialogueEntry> GetDialoguesByType(ENarr_DialogueType DialogueType);

    UFUNCTION(BlueprintCallable, Category = "Dialogue System")
    FNarr_DialogueEntry GetHighestPriorityDialogue(ENarr_DialogueTrigger TriggerType);

    UFUNCTION(BlueprintCallable, Category = "Dialogue System")
    void SetSubtitleVisibility(bool bVisible);

    UFUNCTION(BlueprintCallable, Category = "Dialogue System")
    bool IsDialoguePlaying() const;

protected:
    UFUNCTION()
    void OnDialogueFinished();

private:
    void CreateDefaultDialogueEntries();
    void UpdateSubtitleDisplay();
    void LoadAudioFromURL(const FString& AudioURL);

    UPROPERTY()
    class ATranspersonalCharacter* PlayerCharacter;

    float DialogueStartTime;
    FTimerHandle DialogueTimer;
    FTimerHandle SubtitleTimer;
};