#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/DataTable.h"
#include "Components/ActorComponent.h"
#include "NarrativeDialogueSystem.generated.h"

UENUM(BlueprintType)
enum class ENarr_DialogueTrigger : uint8
{
    OnPlayerEnterZone,
    OnDinosaurSpotted,
    OnCombatStart,
    OnResourceFound,
    OnCraftingComplete,
    OnNightFall,
    OnDayBreak,
    OnLowHealth,
    OnLowHunger,
    OnFireLit
};

UENUM(BlueprintType)
enum class ENarr_SpeakerType : uint8
{
    Narrator,
    TribalScout,
    ElderHunter,
    FireKeeper,
    PlayerThought
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
    FString AudioURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsContextual;

    FNarr_DialogueLine()
    {
        DialogueText = TEXT("");
        Speaker = ENarr_SpeakerType::Narrator;
        AudioURL = TEXT("");
        Duration = 5.0f;
        bIsContextual = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ENarr_DialogueTrigger TriggerType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FNarr_DialogueLine> DialogueLines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CooldownTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxTriggers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bRequiresLineOfSight;

    FNarr_DialogueEntry()
    {
        TriggerType = ENarr_DialogueTrigger::OnPlayerEnterZone;
        CooldownTime = 30.0f;
        MaxTriggers = -1; // Unlimited
        bRequiresLineOfSight = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UNarr_DialogueComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNarr_DialogueComponent();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FNarr_DialogueEntry> DialogueEntries;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float TriggerRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bAutoTrigger;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void TriggerDialogue(ENarr_DialogueTrigger TriggerType);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool CanTriggerDialogue(ENarr_DialogueTrigger TriggerType);

protected:
    virtual void BeginPlay() override;

private:
    TMap<ENarr_DialogueTrigger, float> LastTriggerTimes;
    TMap<ENarr_DialogueTrigger, int32> TriggerCounts;
};

UCLASS()
class TRANSPERSONALGAME_API UNarrativeDialogueSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void PlayDialogue(const FNarr_DialogueLine& DialogueLine);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void RegisterDialogueZone(AActor* ZoneActor, ENarr_DialogueTrigger TriggerType);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TriggerContextualDialogue(ENarr_DialogueTrigger TriggerType, const FVector& Location);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TMap<ENarr_SpeakerType, FString> SpeakerAudioURLs;

protected:
    UPROPERTY()
    TArray<AActor*> RegisteredDialogueZones;

    UPROPERTY()
    TMap<ENarr_DialogueTrigger, float> GlobalCooldowns;

private:
    void LoadDefaultDialogueLibrary();
    bool IsOnCooldown(ENarr_DialogueTrigger TriggerType);
};