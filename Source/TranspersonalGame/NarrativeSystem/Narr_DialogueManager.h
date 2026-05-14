#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/DataTable.h"
#include "Sound/SoundBase.h"
#include "Components/AudioComponent.h"
#include "Narr_DialogueManager.generated.h"

UENUM(BlueprintType)
enum class ENarr_DialogueType : uint8
{
    Greeting,
    Warning,
    Instruction,
    Lore,
    Quest,
    Combat,
    Trading,
    Survival
};

UENUM(BlueprintType)
enum class ENarr_CharacterRole : uint8
{
    HuntMaster,
    TribalElder,
    Scout,
    WaterSeeker,
    WarChief,
    Shaman,
    Crafter,
    Trader
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DialogueID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ENarr_CharacterRole Speaker;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ENarr_DialogueType Type;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FText DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TSoftObjectPtr<USoundBase> VoiceClip;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FString> RequiredConditions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FString> ResponseOptions;

    FNarr_DialogueEntry()
    {
        DialogueID = "";
        Speaker = ENarr_CharacterRole::TribalElder;
        Type = ENarr_DialogueType::Greeting;
        DialogueText = FText::GetEmpty();
        Duration = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_ConversationNode
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Conversation")
    FString NodeID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Conversation")
    FNarr_DialogueEntry DialogueEntry;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Conversation")
    TArray<FString> NextNodeIDs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Conversation")
    bool bIsEndNode;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Conversation")
    bool bRequiresPlayerChoice;

    FNarr_ConversationNode()
    {
        NodeID = "";
        bIsEndNode = false;
        bRequiresPlayerChoice = false;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FNarr_OnDialogueStarted, const FString&, DialogueID, ENarr_CharacterRole, Speaker);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNarr_OnDialogueEnded, const FString&, DialogueID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FNarr_OnDialogueLineSpoken, const FString&, DialogueID, const FText&, DialogueText, float, Duration);

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UNarr_DialogueManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UNarr_DialogueManager();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Dialogue Management
    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    bool StartDialogue(const FString& DialogueID, AActor* Speaker = nullptr, AActor* Listener = nullptr);

    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    void EndDialogue();

    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    bool IsDialogueActive() const;

    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    FString GetCurrentDialogueID() const;

    // Conversation System
    UFUNCTION(BlueprintCallable, Category = "Narrative|Conversation")
    bool StartConversation(const FString& ConversationID, AActor* Speaker = nullptr, AActor* Listener = nullptr);

    UFUNCTION(BlueprintCallable, Category = "Narrative|Conversation")
    bool AdvanceConversation(const FString& ChoiceID = "");

    UFUNCTION(BlueprintCallable, Category = "Narrative|Conversation")
    TArray<FString> GetCurrentChoices() const;

    // Dialogue Registration
    UFUNCTION(BlueprintCallable, Category = "Narrative|Registration")
    void RegisterDialogue(const FNarr_DialogueEntry& DialogueEntry);

    UFUNCTION(BlueprintCallable, Category = "Narrative|Registration")
    void RegisterConversation(const FString& ConversationID, const TArray<FNarr_ConversationNode>& Nodes);

    // Dialogue Queries
    UFUNCTION(BlueprintCallable, Category = "Narrative|Query")
    bool GetDialogueEntry(const FString& DialogueID, FNarr_DialogueEntry& OutEntry) const;

    UFUNCTION(BlueprintCallable, Category = "Narrative|Query")
    TArray<FString> GetDialoguesByType(ENarr_DialogueType Type) const;

    UFUNCTION(BlueprintCallable, Category = "Narrative|Query")
    TArray<FString> GetDialoguesByCharacter(ENarr_CharacterRole Character) const;

    // Audio Management
    UFUNCTION(BlueprintCallable, Category = "Narrative|Audio")
    void PlayDialogueAudio(const FString& DialogueID);

    UFUNCTION(BlueprintCallable, Category = "Narrative|Audio")
    void StopDialogueAudio();

    UFUNCTION(BlueprintCallable, Category = "Narrative|Audio")
    bool IsDialogueAudioPlaying() const;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Narrative|Events")
    FNarr_OnDialogueStarted OnDialogueStarted;

    UPROPERTY(BlueprintAssignable, Category = "Narrative|Events")
    FNarr_OnDialogueEnded OnDialogueEnded;

    UPROPERTY(BlueprintAssignable, Category = "Narrative|Events")
    FNarr_OnDialogueLineSpoken OnDialogueLineSpoken;

protected:
    // Internal dialogue storage
    UPROPERTY()
    TMap<FString, FNarr_DialogueEntry> DialogueDatabase;

    UPROPERTY()
    TMap<FString, TArray<FNarr_ConversationNode>> ConversationDatabase;

    // Current dialogue state
    UPROPERTY()
    FString CurrentDialogueID;

    UPROPERTY()
    FString CurrentConversationID;

    UPROPERTY()
    FString CurrentNodeID;

    UPROPERTY()
    bool bIsDialogueActive;

    UPROPERTY()
    bool bIsConversationActive;

    // Audio components
    UPROPERTY()
    UAudioComponent* DialogueAudioComponent;

    UPROPERTY()
    AActor* CurrentSpeaker;

    UPROPERTY()
    AActor* CurrentListener;

private:
    void InitializeDefaultDialogues();
    void InitializeDefaultConversations();
    FNarr_ConversationNode* GetConversationNode(const FString& ConversationID, const FString& NodeID);
    void ProcessDialogueConditions(const FNarr_DialogueEntry& Entry);
};