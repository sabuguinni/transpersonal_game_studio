#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Narr_DialogueSystem.generated.h"

UENUM(BlueprintType)
enum class ENarr_DialogueTrigger : uint8
{
    None = 0,
    LowHealth,
    Hunger,
    Thirst,
    DinosaurNearby,
    CombatWarning,
    Discovery,
    FireLit,
    NightFall,
    DayBreak,
    WeatherChange,
    MAX UMETA(Hidden)
};

UENUM(BlueprintType)
enum class ENarr_DialoguePriority : uint8
{
    Low = 0,
    Normal = 1,
    High = 2,
    Critical = 3
};

USTRUCT(BlueprintType)
struct FNarr_DialogueEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ENarr_DialogueTrigger TriggerType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ENarr_DialoguePriority Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float CooldownTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bIsRepeatable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString AudioFilePath;

    FNarr_DialogueEntry()
    {
        DialogueText = TEXT("");
        TriggerType = ENarr_DialogueTrigger::None;
        Priority = ENarr_DialoguePriority::Normal;
        CooldownTime = 30.0f;
        bIsRepeatable = true;
        AudioFilePath = TEXT("");
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

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TriggerDialogue(ENarr_DialogueTrigger TriggerType, float HealthPercent = 1.0f, float HungerLevel = 0.0f, float ThirstLevel = 0.0f);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void AddDialogueEntry(const FNarr_DialogueEntry& NewEntry);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void PlayDialogue(const FString& DialogueText, ENarr_DialoguePriority Priority = ENarr_DialoguePriority::Normal);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool IsDialogueOnCooldown(ENarr_DialogueTrigger TriggerType) const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void SetDialogueCooldown(ENarr_DialogueTrigger TriggerType, float CooldownTime);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_DialogueEntry> DialogueEntries;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    TMap<ENarr_DialogueTrigger, float> DialogueCooldowns;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float DefaultCooldownTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bEnableDebugOutput;

private:
    void InitializeDefaultDialogues();
    FNarr_DialogueEntry* FindBestDialogue(ENarr_DialogueTrigger TriggerType, float HealthPercent, float HungerLevel, float ThirstLevel);
    void UpdateCooldowns(float DeltaTime);
    void LogDialogue(const FString& DialogueText, ENarr_DialogueTrigger TriggerType) const;
};