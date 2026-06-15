#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/DataTable.h"
#include "SharedTypes.h"
#include "Narr_DialogueManager.generated.h"

class USoundWave;
class UAudioComponent;

UENUM(BlueprintType)
enum class ENarr_DialogueTrigger : uint8
{
    None UMETA(DisplayName = "None"),
    LocationEnter UMETA(DisplayName = "Location Enter"),
    DinosaurSighted UMETA(DisplayName = "Dinosaur Sighted"),
    CombatStart UMETA(DisplayName = "Combat Start"),
    CombatEnd UMETA(DisplayName = "Combat End"),
    QuestStart UMETA(DisplayName = "Quest Start"),
    QuestComplete UMETA(DisplayName = "Quest Complete"),
    SurvivalCritical UMETA(DisplayName = "Survival Critical"),
    TimeOfDay UMETA(DisplayName = "Time Of Day"),
    WeatherChange UMETA(DisplayName = "Weather Change")
};

UENUM(BlueprintType)
enum class ENarr_CharacterType : uint8
{
    None UMETA(DisplayName = "None"),
    TribalElder UMETA(DisplayName = "Tribal Elder"),
    TribalScout UMETA(DisplayName = "Tribal Scout"),
    TribalShaman UMETA(DisplayName = "Tribal Shaman"),
    TribalHunter UMETA(DisplayName = "Tribal Hunter"),
    TribalWitness UMETA(DisplayName = "Tribal Witness"),
    Narrator UMETA(DisplayName = "Narrator"),
    PlayerThought UMETA(DisplayName = "Player Thought")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DialogueID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ENarr_CharacterType CharacterType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TSoftObjectPtr<USoundWave> AudioAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ENarr_DialogueTrigger TriggerType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString TriggerCondition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    int32 Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float Cooldown;

    FNarr_DialogueLine()
    {
        CharacterType = ENarr_CharacterType::None;
        Duration = 5.0f;
        TriggerType = ENarr_DialogueTrigger::None;
        Priority = 0;
        Cooldown = 30.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_ActiveDialogue
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    FNarr_DialogueLine DialogueLine;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    float TimeStarted;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    bool bIsPlaying;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    UAudioComponent* AudioComponent;

    FNarr_ActiveDialogue()
    {
        TimeStarted = 0.0f;
        bIsPlaying = false;
        AudioComponent = nullptr;
    }
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UNarr_DialogueManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UNarr_DialogueManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core dialogue functions
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void TriggerDialogue(ENarr_DialogueTrigger TriggerType, const FString& Condition = TEXT(""));

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void PlayDialogueLine(const FNarr_DialogueLine& DialogueLine);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void StopCurrentDialogue();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool IsDialoguePlaying() const;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void RegisterDialogueLine(const FNarr_DialogueLine& DialogueLine);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void LoadDialogueDatabase(UDataTable* DialogueTable);

    // Context-aware dialogue
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void SetPlayerLocation(const FString& LocationName);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void SetNearbyDinosaur(const FString& DinosaurType);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void SetSurvivalState(float Health, float Hunger, float Thirst, float Fear);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void SetTimeOfDay(float TimeHours);

    // Dialogue history and cooldowns
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool CanPlayDialogue(const FString& DialogueID) const;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void AddDialogueCooldown(const FString& DialogueID, float CooldownTime);

protected:
    // Dialogue database
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FNarr_DialogueLine> DialogueDatabase;

    // Current dialogue state
    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    FNarr_ActiveDialogue CurrentDialogue;

    // Context tracking
    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    FString CurrentLocation;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    FString NearbyDinosaur;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    float PlayerHealth;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    float PlayerHunger;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    float PlayerThirst;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    float PlayerFear;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    float CurrentTimeOfDay;

    // Cooldown tracking
    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    TMap<FString, float> DialogueCooldowns;

    // Audio management
    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    UAudioComponent* DialogueAudioComponent;

private:
    // Internal dialogue selection
    FNarr_DialogueLine* FindBestDialogue(ENarr_DialogueTrigger TriggerType, const FString& Condition);
    bool EvaluateDialogueCondition(const FNarr_DialogueLine& DialogueLine, const FString& Condition);
    void UpdateCooldowns();
    void OnDialogueFinished();

    // Timer for dialogue updates
    FTimerHandle DialogueUpdateTimer;
    void TickDialogueSystem();
};