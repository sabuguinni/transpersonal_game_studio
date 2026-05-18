#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/DataTable.h"
#include "Sound/SoundCue.h"
#include "../SharedTypes.h"
#include "NarrativeManager.generated.h"

// Forward declarations
class USoundCue;
class AActor;
class APlayerController;

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    USoundCue* VoiceClip;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bIsPlayerChoice;

    FNarr_DialogueLine()
    {
        SpeakerName = TEXT("");
        DialogueText = TEXT("");
        VoiceClip = nullptr;
        Duration = 3.0f;
        bIsPlayerChoice = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_NarrativeEvent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString EventID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString EventDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_DialogueLine> DialogueLines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FVector TriggerLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float TriggerRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bIsRepeatable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bHasBeenTriggered;

    FNarr_NarrativeEvent()
    {
        EventID = TEXT("");
        EventDescription = TEXT("");
        TriggerLocation = FVector::ZeroVector;
        TriggerRadius = 1000.0f;
        bIsRepeatable = false;
        bHasBeenTriggered = false;
    }
};

UENUM(BlueprintType)
enum class ENarr_StoryPhase : uint8
{
    Introduction    UMETA(DisplayName = "Introduction"),
    EarlyExploration UMETA(DisplayName = "Early Exploration"),
    FirstContact    UMETA(DisplayName = "First Contact"),
    TerritoryWars   UMETA(DisplayName = "Territory Wars"),
    GreatMigration  UMETA(DisplayName = "Great Migration"),
    FinalSurvival   UMETA(DisplayName = "Final Survival")
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UNarrativeManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UNarrativeManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core narrative functions
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TriggerNarrativeEvent(const FString& EventID, AActor* TriggeringActor = nullptr);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void PlayDialogue(const FNarr_DialogueLine& DialogueLine, APlayerController* PlayerController = nullptr);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void AdvanceStoryPhase();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    ENarr_StoryPhase GetCurrentStoryPhase() const { return CurrentStoryPhase; }

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void RegisterNarrativeEvent(const FNarr_NarrativeEvent& NewEvent);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool IsEventTriggered(const FString& EventID) const;

    // Location-based narrative
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void CheckLocationBasedEvents(const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TriggerEnvironmentalNarration(EBiomeType BiomeType, const FVector& Location);

    // Survival narrative integration
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void OnSurvivalStateChanged(float Health, float Hunger, float Thirst, float Fear);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void OnDinosaurEncounter(const FString& DinosaurType, bool bIsHostile);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative", meta = (AllowPrivateAccess = "true"))
    ENarr_StoryPhase CurrentStoryPhase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative", meta = (AllowPrivateAccess = "true"))
    TArray<FNarr_NarrativeEvent> RegisteredEvents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative", meta = (AllowPrivateAccess = "true"))
    TArray<FString> TriggeredEventIDs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative", meta = (AllowPrivateAccess = "true"))
    float LastNarrationTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative", meta = (AllowPrivateAccess = "true"))
    float NarrationCooldown;

private:
    void InitializeDefaultEvents();
    void LoadNarrativeData();
    FNarr_NarrativeEvent* FindEventByID(const FString& EventID);
    bool CanTriggerNarration() const;
};