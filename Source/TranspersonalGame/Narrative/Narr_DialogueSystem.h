#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "Sound/SoundWave.h"
#include "Narr_DialogueSystem.generated.h"

// Forward declarations
class UNarr_DialogueManager;
class ANarr_NPCCharacter;

UENUM(BlueprintType)
enum class ENarr_DialogueType : uint8
{
    Greeting        UMETA(DisplayName = "Greeting"),
    QuestGiver      UMETA(DisplayName = "Quest Giver"),
    Trader          UMETA(DisplayName = "Trader"),
    Warning         UMETA(DisplayName = "Warning"),
    Storytelling    UMETA(DisplayName = "Storytelling"),
    Combat          UMETA(DisplayName = "Combat"),
    Emergency       UMETA(DisplayName = "Emergency")
};

UENUM(BlueprintType)
enum class ENarr_NPCRole : uint8
{
    TribalElder     UMETA(DisplayName = "Tribal Elder"),
    HuntMaster      UMETA(DisplayName = "Hunt Master"),
    WaterSeeker     UMETA(DisplayName = "Water Seeker"),
    WarChief        UMETA(DisplayName = "War Chief"),
    Scout           UMETA(DisplayName = "Scout"),
    Crafter         UMETA(DisplayName = "Crafter"),
    Healer          UMETA(DisplayName = "Healer"),
    Survivor        UMETA(DisplayName = "Survivor")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FText DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TSoftObjectPtr<USoundWave> VoiceClip;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ENarr_DialogueType DialogueType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bRequiresPlayerResponse;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FText> PlayerResponseOptions;

    FNarr_DialogueLine()
    {
        DialogueText = FText::GetEmpty();
        DialogueType = ENarr_DialogueType::Greeting;
        Duration = 5.0f;
        bRequiresPlayerResponse = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_NPCProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
    FName NPCName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
    ENarr_NPCRole Role;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
    FText BackgroundStory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
    TArray<FNarr_DialogueLine> DialogueLines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
    float TrustLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
    bool bIsHostile;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
    FVector TerritoryCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
    float TerritoryRadius;

    FNarr_NPCProfile()
    {
        NPCName = TEXT("UnnamedNPC");
        Role = ENarr_NPCRole::Survivor;
        BackgroundStory = FText::GetEmpty();
        TrustLevel = 0.5f;
        bIsHostile = false;
        TerritoryCenter = FVector::ZeroVector;
        TerritoryRadius = 1000.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_StoryEvent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FName EventID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FText EventTitle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FText EventDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    bool bIsTriggered;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    TArray<FName> PrerequisiteEvents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    TArray<FNarr_DialogueLine> EventDialogue;

    FNarr_StoryEvent()
    {
        EventID = TEXT("DefaultEvent");
        EventTitle = FText::GetEmpty();
        EventDescription = FText::GetEmpty();
        bIsTriggered = false;
        bIsCompleted = false;
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

    // NPC Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPCs")
    TArray<FNarr_NPCProfile> RegisteredNPCs;

    // Story Events
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    TArray<FNarr_StoryEvent> StoryEvents;

    // Dialogue State
    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    bool bIsDialogueActive;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    FNarr_NPCProfile* CurrentNPC;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    int32 CurrentDialogueIndex;

    // Audio Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    class UAudioComponent* DialogueAudioComponent;

public:
    // NPC Registration and Management
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void RegisterNPC(const FNarr_NPCProfile& NPCProfile);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    FNarr_NPCProfile* FindNPCByName(const FName& NPCName);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    TArray<FNarr_NPCProfile*> GetNPCsInRadius(const FVector& Location, float Radius);

    // Dialogue Management
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool StartDialogue(const FName& NPCName, ENarr_DialogueType DialogueType = ENarr_DialogueType::Greeting);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void EndDialogue();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool PlayNextDialogueLine();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void SelectPlayerResponse(int32 ResponseIndex);

    // Story Event Management
    UFUNCTION(BlueprintCallable, Category = "Story")
    void TriggerStoryEvent(const FName& EventID);

    UFUNCTION(BlueprintCallable, Category = "Story")
    bool IsStoryEventCompleted(const FName& EventID);

    UFUNCTION(BlueprintCallable, Category = "Story")
    void CompleteStoryEvent(const FName& EventID);

    // Utility Functions
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void UpdateNPCTrustLevel(const FName& NPCName, float TrustDelta);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    float GetNPCTrustLevel(const FName& NPCName);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    TArray<FNarr_DialogueLine> GetDialogueByType(const FName& NPCName, ENarr_DialogueType DialogueType);

    // Events
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDialogueStarted, FName, NPCName, ENarr_DialogueType, DialogueType);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDialogueEnded, FName, NPCName);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStoryEventTriggered, FName, EventID, FText, EventTitle);

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnDialogueStarted OnDialogueStarted;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnDialogueEnded OnDialogueEnded;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnStoryEventTriggered OnStoryEventTriggered;

private:
    void InitializeDefaultNPCs();
    void InitializeStoryEvents();
    bool CanTriggerStoryEvent(const FNarr_StoryEvent& Event);
    void PlayDialogueAudio(const FNarr_DialogueLine& DialogueLine);
};

#include "Narr_DialogueSystem.generated.h"