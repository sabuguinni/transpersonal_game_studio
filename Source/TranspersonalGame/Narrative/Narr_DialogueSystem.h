#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "SharedTypes.h"
#include "Narr_DialogueSystem.generated.h"

// Dialogue line data structure
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString AudioFilePath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    EEng_BiomeType TriggerBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bIsContextual;

    FNarr_DialogueLine()
    {
        SpeakerName = "Unknown";
        DialogueText = "";
        AudioFilePath = "";
        Duration = 5.0f;
        TriggerBiome = EEng_BiomeType::Savanna;
        bIsContextual = false;
    }
};

// Narrative event data
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
    EEng_QuestType RelatedQuestType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bHasBeenTriggered;

    FNarr_NarrativeEvent()
    {
        EventID = "DefaultEvent";
        EventDescription = "";
        RelatedQuestType = EEng_QuestType::Survival;
        bHasBeenTriggered = false;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNarr_DialogueSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UNarr_DialogueSystem();

protected:
    virtual void BeginPlay() override;

    // Dialogue data storage
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue System")
    TArray<FNarr_DialogueLine> DialogueDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue System")
    TArray<FNarr_NarrativeEvent> NarrativeEvents;

    // Current dialogue state
    UPROPERTY(BlueprintReadOnly, Category = "Dialogue System")
    bool bIsDialoguePlaying;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue System")
    FNarr_DialogueLine CurrentDialogue;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue System")
    float DialogueTimer;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Dialogue control functions
    UFUNCTION(BlueprintCallable, Category = "Dialogue System")
    void PlayDialogue(const FNarr_DialogueLine& DialogueLine);

    UFUNCTION(BlueprintCallable, Category = "Dialogue System")
    void StopDialogue();

    UFUNCTION(BlueprintCallable, Category = "Dialogue System")
    void TriggerNarrativeEvent(const FString& EventID);

    UFUNCTION(BlueprintCallable, Category = "Dialogue System")
    void TriggerBiomeDialogue(EEng_BiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Dialogue System")
    void TriggerSurvivalDialogue(EEng_SurvivalStat StatType, float StatValue);

    UFUNCTION(BlueprintCallable, Category = "Dialogue System")
    void TriggerDinosaurEncounterDialogue(EEng_DinosaurSpecies Species, float Distance);

    // Data management
    UFUNCTION(BlueprintCallable, Category = "Dialogue System")
    void LoadDialogueFromDataTable(UDataTable* DialogueTable);

    UFUNCTION(BlueprintCallable, Category = "Dialogue System")
    void AddDialogueToDatabase(const FNarr_DialogueLine& NewDialogue);

    UFUNCTION(BlueprintCallable, Category = "Dialogue System")
    TArray<FNarr_DialogueLine> GetDialogueByBiome(EEng_BiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Dialogue System")
    bool IsDialogueActive() const { return bIsDialoguePlaying; }

    // Events
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDialogueStarted, const FNarr_DialogueLine&, DialogueLine);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDialogueEnded);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNarrativeEventTriggered, const FString&, EventID);

    UPROPERTY(BlueprintAssignable, Category = "Dialogue System")
    FOnDialogueStarted OnDialogueStarted;

    UPROPERTY(BlueprintAssignable, Category = "Dialogue System")
    FOnDialogueEnded OnDialogueEnded;

    UPROPERTY(BlueprintAssignable, Category = "Dialogue System")
    FOnNarrativeEventTriggered OnNarrativeEventTriggered;

private:
    void InitializeDefaultDialogue();
    void UpdateDialogueTimer(float DeltaTime);
    FNarr_DialogueLine GetRandomDialogueByType(EEng_BiomeType BiomeType);
};