#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/DataTable.h"
#include "Sound/SoundCue.h"
#include "Narr_DialogueSystem.generated.h"

UENUM(BlueprintType)
enum class ENarr_DialogueType : uint8
{
    Greeting,
    Warning,
    Information,
    Quest,
    Emergency,
    Storytelling
};

UENUM(BlueprintType)
enum class ENarr_NPCPersonality : uint8
{
    Veteran_Cautious,
    Elder_Wise,
    Scout_Alert,
    Tracker_Analytical,
    Hunter_Aggressive,
    Survivor_Desperate
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ENarr_DialogueType DialogueType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ENarr_NPCPersonality PersonalityType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundCue> VoiceLine;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Conditions")
    float TriggerDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Conditions")
    bool bRequiresLineOfSight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Conditions")
    int32 Priority;

    FNarr_DialogueLine()
    {
        DialogueText = TEXT("");
        DialogueType = ENarr_DialogueType::Information;
        CharacterName = TEXT("Unknown");
        PersonalityType = ENarr_NPCPersonality::Survivor_Desperate;
        TriggerDistance = 500.0f;
        bRequiresLineOfSight = true;
        Priority = 1;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_CharacterProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    FString CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    ENarr_NPCPersonality PersonalityType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    FString BackgroundStory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    TArray<FString> SpecialtyKnowledge;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FNarr_DialogueLine> AvailableDialogues;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float CommunicationRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    bool bIsHostile;

    FNarr_CharacterProfile()
    {
        CharacterName = TEXT("Unnamed Survivor");
        PersonalityType = ENarr_NPCPersonality::Survivor_Desperate;
        BackgroundStory = TEXT("A survivor trying to make it in the prehistoric world.");
        CommunicationRange = 800.0f;
        bIsHostile = false;
    }
};

/**
 * Narrative and Dialogue System for prehistoric survival game
 * Manages character interactions, voice lines, and story progression
 */
UCLASS()
class TRANSPERSONALGAME_API UNarr_DialogueSystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UNarr_DialogueSystem();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Character management
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void RegisterCharacter(const FString& CharacterID, const FNarr_CharacterProfile& Profile);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool GetCharacterProfile(const FString& CharacterID, FNarr_CharacterProfile& OutProfile);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    TArray<FString> GetAllCharacterIDs();

    // Dialogue system
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool TriggerDialogue(const FString& CharacterID, ENarr_DialogueType DialogueType, AActor* Player, AActor* NPC);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    FNarr_DialogueLine GetBestDialogue(const FString& CharacterID, ENarr_DialogueType DialogueType, float Distance);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void PlayVoiceLine(const FNarr_DialogueLine& DialogueLine, AActor* SourceActor);

    // Proximity-based dialogue
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void CheckProximityDialogues(AActor* Player, const TArray<AActor*>& NPCs);

    // Emergency and warning systems
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void BroadcastEmergencyWarning(const FString& WarningText, const FVector& DangerLocation);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TriggerStorytellingSequence(const FString& CharacterID, const FString& StoryTheme);

    // Data management
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void LoadDialogueData();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void SaveDialogueProgress();

protected:
    // Character database
    UPROPERTY(BlueprintReadOnly, Category = "Data")
    TMap<FString, FNarr_CharacterProfile> RegisteredCharacters;

    // Dialogue history
    UPROPERTY(BlueprintReadOnly, Category = "Data")
    TMap<FString, TArray<FString>> DialogueHistory;

    // Audio management
    UPROPERTY(BlueprintReadOnly, Category = "Audio")
    TMap<FString, USoundCue*> LoadedVoiceLines;

    // System settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float GlobalDialogueRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float DialogueCooldownTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    bool bEnableProximityDialogues;

    // Internal methods
    bool CanTriggerDialogue(const FString& CharacterID, ENarr_DialogueType DialogueType);
    void UpdateDialogueHistory(const FString& CharacterID, const FString& DialogueText);
    float CalculateDialoguePriority(const FNarr_DialogueLine& DialogueLine, float Distance, ENarr_DialogueType RequestedType);
};