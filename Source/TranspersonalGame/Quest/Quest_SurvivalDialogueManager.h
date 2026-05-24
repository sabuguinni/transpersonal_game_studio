#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "Quest_SurvivalDialogueManager.generated.h"

UENUM(BlueprintType)
enum class EQuest_DialogueType : uint8
{
    Tutorial        UMETA(DisplayName = "Tutorial"),
    Advanced        UMETA(DisplayName = "Advanced"),
    Exploration     UMETA(DisplayName = "Exploration"),
    Challenge       UMETA(DisplayName = "Challenge"),
    Emergency       UMETA(DisplayName = "Emergency")
};

UENUM(BlueprintType)
enum class EQuest_NPCPersonality : uint8
{
    Elder           UMETA(DisplayName = "Elder"),
    Hunter          UMETA(DisplayName = "Hunter"),
    Tracker         UMETA(DisplayName = "Tracker"),
    Guide           UMETA(DisplayName = "Guide"),
    Survivor        UMETA(DisplayName = "Survivor")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_DialogueEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DialogueID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FText DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    EQuest_DialogueType DialogueType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FString> RequiredQuests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FString> TriggeredQuests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    FString AudioURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float DialogueDuration;

    FQuest_DialogueEntry()
    {
        DialogueID = "";
        DialogueText = FText::GetEmpty();
        DialogueType = EQuest_DialogueType::Tutorial;
        AudioURL = "";
        DialogueDuration = 10.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_NPCDialogueData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
    FString NPCName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
    EQuest_NPCPersonality Personality;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
    TArray<FQuest_DialogueEntry> DialogueEntries;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
    FVector SpawnLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
    bool bIsActive;

    FQuest_NPCDialogueData()
    {
        NPCName = "";
        Personality = EQuest_NPCPersonality::Elder;
        SpawnLocation = FVector::ZeroVector;
        bIsActive = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_SurvivalDialogueManager : public AActor
{
    GENERATED_BODY()

public:
    AQuest_SurvivalDialogueManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue System")
    TArray<FQuest_NPCDialogueData> NPCDialogueDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue System")
    TMap<FString, FQuest_DialogueEntry> ActiveDialogues;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TMap<FString, FString> DialogueAudioURLs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float DialogueRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float DialogueTimeout;

public:
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool StartDialogue(const FString& NPCName, const FString& DialogueID);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void EndDialogue(const FString& DialogueID);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    FQuest_DialogueEntry GetDialogueByID(const FString& DialogueID);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    TArray<FQuest_DialogueEntry> GetAvailableDialogues(const FString& NPCName);

    UFUNCTION(BlueprintCallable, Category = "NPC")
    bool RegisterNPCDialogue(const FQuest_NPCDialogueData& NPCData);

    UFUNCTION(BlueprintCallable, Category = "NPC")
    void UpdateNPCLocation(const FString& NPCName, const FVector& NewLocation);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlayDialogueAudio(const FString& DialogueID);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void RegisterDialogueAudio(const FString& DialogueID, const FString& AudioURL);

    UFUNCTION(BlueprintCallable, Category = "Quest Integration")
    void TriggerQuestsFromDialogue(const FString& DialogueID);

    UFUNCTION(BlueprintCallable, Category = "Validation")
    bool ValidateDialogueRequirements(const FString& DialogueID);

private:
    void InitializeDefaultDialogues();
    void LoadDialogueAudioURLs();
    bool IsPlayerInRange(const FVector& NPCLocation);
};