#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "SharedTypes.h"
#include "Narr_QuestNarrativeIntegration.generated.h"

class UNarr_DialogueSystem;
class UNarr_StoryManager;
class UQuest_SurvivalCraftingController;

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_QuestDialogueData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Dialogue")
    FString QuestID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Dialogue")
    FString DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Dialogue")
    FString CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Dialogue")
    float DialoguePriority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Dialogue")
    bool bIsEmergencyDialogue;

    FNarr_QuestDialogueData()
        : QuestID(TEXT(""))
        , DialogueText(TEXT(""))
        , CharacterName(TEXT(""))
        , DialoguePriority(1.0f)
        , bIsEmergencyDialogue(false)
    {
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_SurvivalContextData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Context")
    float PlayerHealth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Context")
    float PlayerHunger;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Context")
    float PlayerThirst;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Context")
    float ThreatLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Context")
    FString CurrentBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Context")
    TArray<FString> NearbyThreats;

    FNarr_SurvivalContextData()
        : PlayerHealth(100.0f)
        , PlayerHunger(100.0f)
        , PlayerThirst(100.0f)
        , ThreatLevel(0.0f)
        , CurrentBiome(TEXT("Forest"))
    {
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UNarr_QuestNarrativeIntegration : public UActorComponent
{
    GENERATED_BODY()

public:
    UNarr_QuestNarrativeIntegration();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Quest Integration Functions
    UFUNCTION(BlueprintCallable, Category = "Quest Integration")
    void RegisterQuestDialogue(const FString& QuestID, const FString& DialogueText, const FString& CharacterName);

    UFUNCTION(BlueprintCallable, Category = "Quest Integration")
    void TriggerQuestNarrative(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest Integration")
    void UpdateSurvivalContext(const FNarr_SurvivalContextData& ContextData);

    // Dialogue Management
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool PlayQuestDialogue(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void PlayEmergencyDialogue(const FString& ThreatType, float ThreatLevel);

    // Context-Aware Narration
    UFUNCTION(BlueprintCallable, Category = "Narration")
    void TriggerContextualNarration();

    UFUNCTION(BlueprintCallable, Category = "Narration")
    FString GenerateSurvivalTip(const FNarr_SurvivalContextData& Context);

protected:
    // Component References
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UNarr_DialogueSystem* DialogueSystem;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UNarr_StoryManager* StoryManager;

    // Quest Dialogue Database
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Data")
    TMap<FString, FNarr_QuestDialogueData> QuestDialogueMap;

    // Current Context
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    FNarr_SurvivalContextData CurrentSurvivalContext;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float ContextUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float EmergencyDialogueCooldown;

    // State Tracking
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    float LastContextUpdate;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    float LastEmergencyDialogue;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    bool bIsDialogueActive;

private:
    // Internal Functions
    void InitializeDialogueDatabase();
    void UpdateContextualNarration(float DeltaTime);
    bool ShouldTriggerEmergencyDialogue(float ThreatLevel);
    FString SelectAppropriateDialogue(const FString& QuestID, const FNarr_SurvivalContextData& Context);
};