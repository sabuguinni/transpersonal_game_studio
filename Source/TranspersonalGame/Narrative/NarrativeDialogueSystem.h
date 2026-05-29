#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Components/AudioComponent.h"
#include "SharedTypes.h"
#include "NarrativeDialogueSystem.generated.h"

UENUM(BlueprintType)
enum class ENarr_DialogueType : uint8
{
    QuestGiver,
    Warning,
    Discovery,
    Combat,
    Tutorial,
    Ambient
};

UENUM(BlueprintType)
enum class ENarr_NPCRole : uint8
{
    TribalElder,
    Hunter,
    Shaman,
    Scout,
    Crafter,
    Warrior
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString AudioPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ENarr_DialogueType DialogueType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bIsLooping;

    FNarr_DialogueLine()
    {
        DialogueText = TEXT("");
        AudioPath = TEXT("");
        DialogueType = ENarr_DialogueType::Ambient;
        Duration = 5.0f;
        bIsLooping = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_NPCProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
    FString NPCName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
    ENarr_NPCRole Role;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
    TArray<FNarr_DialogueLine> DialogueLines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
    FString VoiceID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
    bool bIsQuestGiver;

    FNarr_NPCProfile()
    {
        NPCName = TEXT("Unknown");
        Role = ENarr_NPCRole::Scout;
        VoiceID = TEXT("");
        bIsQuestGiver = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UNarrativeDialogueSystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UNarrativeDialogueSystem();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void PlayDialogue(const FString& NPCName, ENarr_DialogueType DialogueType);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void RegisterNPC(const FNarr_NPCProfile& NPCProfile);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TriggerQuestDialogue(const FString& QuestID, const FString& NPCName);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void PlayWarningDialogue(const FString& ThreatType, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void PlayDiscoveryDialogue(const FString& DiscoveryType);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool IsDialoguePlaying() const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void StopCurrentDialogue();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TMap<FString, FNarr_NPCProfile> RegisteredNPCs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TMap<FString, FNarr_DialogueLine> QuestDialogueLibrary;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TMap<FString, FNarr_DialogueLine> WarningDialogueLibrary;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TMap<FString, FNarr_DialogueLine> DiscoveryDialogueLibrary;

    UPROPERTY()
    UAudioComponent* CurrentAudioComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bDialoguePlaying;

private:
    void InitializeDialogueLibraries();
    FNarr_DialogueLine GetDialogueForType(ENarr_DialogueType DialogueType, const FString& Context);
    void PlayAudioDialogue(const FNarr_DialogueLine& DialogueLine);
};