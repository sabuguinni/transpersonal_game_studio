#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameInstanceSubsystem.h"
#include "Engine/DataTable.h"
#include "Sound/SoundCue.h"
#include "Narr_DialogueManager.generated.h"

UENUM(BlueprintType)
enum class ENarr_DialogueTrigger : uint8
{
    None = 0,
    PlayerApproach = 1,
    LowHealth = 2,
    DinosaurNearby = 3,
    FirstEncounter = 4,
    QuestComplete = 5,
    Survival_Hunger = 6,
    Survival_Thirst = 7,
    Combat_Warning = 8,
    Discovery = 9
};

UENUM(BlueprintType)
enum class ENarr_SpeakerType : uint8
{
    None = 0,
    Narrator = 1,
    TribalElder = 2,
    Hunter = 3,
    Scout = 4,
    Shaman = 5,
    PlayerThought = 6,
    Warning = 7
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ENarr_SpeakerType Speaker;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ENarr_DialogueTrigger TriggerCondition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsRepeatable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CooldownTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<USoundCue> VoiceAudio;

    FNarr_DialogueLine()
    {
        DialogueText = TEXT("");
        Speaker = ENarr_SpeakerType::None;
        TriggerCondition = ENarr_DialogueTrigger::None;
        Priority = 1.0f;
        bIsRepeatable = true;
        CooldownTime = 30.0f;
        VoiceAudio = nullptr;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueContext
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PlayerHealthPercent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PlayerHungerLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PlayerThirstLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bDinosaurNearby;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DinosaurDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString DinosaurType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString CurrentBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TimeOfDay;

    FNarr_DialogueContext()
    {
        PlayerHealthPercent = 100.0f;
        PlayerHungerLevel = 0.0f;
        PlayerThirstLevel = 0.0f;
        bDinosaurNearby = false;
        DinosaurDistance = 0.0f;
        DinosaurType = TEXT("");
        CurrentBiome = TEXT("Savana");
        TimeOfDay = 12.0f;
    }
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UNarr_DialogueManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UNarr_DialogueManager();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TriggerDialogue(ENarr_DialogueTrigger TriggerType, const FNarr_DialogueContext& Context);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void RegisterDialogueLine(const FNarr_DialogueLine& DialogueLine);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void PlayDialogue(const FNarr_DialogueLine& DialogueLine);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool CanPlayDialogue(const FNarr_DialogueLine& DialogueLine) const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void UpdateContext(const FNarr_DialogueContext& NewContext);

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    TArray<FNarr_DialogueLine> DialogueDatabase;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    FNarr_DialogueContext CurrentContext;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    TMap<FString, float> LastPlayedTimes;

    UFUNCTION(BlueprintImplementableEvent, Category = "Narrative")
    void OnDialogueTriggered(const FNarr_DialogueLine& DialogueLine);

    UFUNCTION(BlueprintImplementableEvent, Category = "Narrative")
    void OnContextChanged(const FNarr_DialogueContext& NewContext);

private:
    void InitializeDefaultDialogues();
    bool EvaluateTriggerCondition(const FNarr_DialogueLine& DialogueLine, const FNarr_DialogueContext& Context) const;
    FNarr_DialogueLine* FindBestDialogue(ENarr_DialogueTrigger TriggerType, const FNarr_DialogueContext& Context);
};