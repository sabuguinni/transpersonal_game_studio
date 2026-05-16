#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/DataTable.h"
#include "Narr_DialogueSystem.generated.h"

UENUM(BlueprintType)
enum class ENarr_DialogueType : uint8
{
    Greeting,
    Warning,
    Information,
    Quest,
    Lore,
    Survival_Tip
};

UENUM(BlueprintType)
enum class ENarr_SpeakerType : uint8
{
    Ancient_Storyteller,
    Tribal_Warrior,
    Experienced_Hunter,
    Wise_Elder,
    Young_Scout,
    Shaman_Healer
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DialogueID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ENarr_SpeakerType Speaker;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ENarr_DialogueType Type;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FText DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString AudioURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FString> RequiredConditions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bIsRepeatable;

    FNarr_DialogueEntry()
    {
        Speaker = ENarr_SpeakerType::Ancient_Storyteller;
        Type = ENarr_DialogueType::Information;
        Duration = 5.0f;
        bIsRepeatable = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_NarrativeEvent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString EventID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FText EventTitle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FText EventDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_DialogueEntry> DialogueSequence;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FVector TriggerLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float TriggerRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bHasBeenTriggered;

    FNarr_NarrativeEvent()
    {
        TriggerLocation = FVector::ZeroVector;
        TriggerRadius = 1000.0f;
        bHasBeenTriggered = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UNarr_DialogueSystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UNarr_DialogueSystem();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Dialogue Management
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void PlayDialogue(const FString& DialogueID, AActor* Speaker = nullptr);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void RegisterDialogueEntry(const FNarr_DialogueEntry& Entry);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool CanPlayDialogue(const FString& DialogueID) const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TriggerNarrativeEvent(const FString& EventID, const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void RegisterNarrativeEvent(const FNarr_NarrativeEvent& Event);

    // Survival Narrative
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TriggerSurvivalTip(const FString& TipCategory);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TriggerDangerWarning(const FString& ThreatType, const FVector& ThreatLocation);

    // Lore System
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void UnlockLoreEntry(const FString& LoreID);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    TArray<FString> GetUnlockedLore() const;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TMap<FString, FNarr_DialogueEntry> DialogueDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TMap<FString, FNarr_NarrativeEvent> NarrativeEvents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FString> UnlockedLoreEntries;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FString> PlayedDialogues;

    // Internal functions
    void InitializeDefaultDialogues();
    void InitializeNarrativeEvents();
    bool CheckDialogueConditions(const FNarr_DialogueEntry& Entry) const;
};