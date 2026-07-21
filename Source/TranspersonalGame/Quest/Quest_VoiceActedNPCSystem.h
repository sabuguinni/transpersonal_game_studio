#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "Sound/SoundCue.h"
#include "Quest_VoiceActedNPCSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_NPCDialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString AudioURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString QuestContext;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bIsQuestGiver;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bIsQuestCompleter;

    FQuest_NPCDialogueLine()
    {
        CharacterName = TEXT("");
        DialogueText = TEXT("");
        AudioURL = TEXT("");
        Duration = 0.0f;
        QuestContext = TEXT("");
        bIsQuestGiver = false;
        bIsQuestCompleter = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_NPCProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Profile")
    FString NPCName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Profile")
    FString Role;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Profile")
    FString Specialization;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Profile")
    TArray<FQuest_NPCDialogueLine> DialogueLines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Profile")
    TArray<FString> AvailableQuests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Profile")
    FVector WorldLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Profile")
    float InteractionRadius;

    FQuest_NPCProfile()
    {
        NPCName = TEXT("");
        Role = TEXT("");
        Specialization = TEXT("");
        WorldLocation = FVector::ZeroVector;
        InteractionRadius = 300.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UQuest_VoiceActedNPCSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UQuest_VoiceActedNPCSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // NPC Profile Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC System")
    TArray<FQuest_NPCProfile> NPCProfiles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC System")
    float GlobalInteractionRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC System")
    bool bAutoPlayDialogue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC System")
    float DialogueDelay;

    // Voice Acting Functions
    UFUNCTION(BlueprintCallable, Category = "Voice Acting")
    void PlayNPCDialogue(const FString& NPCName, const FString& DialogueContext);

    UFUNCTION(BlueprintCallable, Category = "Voice Acting")
    void StopCurrentDialogue();

    UFUNCTION(BlueprintCallable, Category = "Voice Acting")
    bool IsDialoguePlaying() const;

    UFUNCTION(BlueprintCallable, Category = "Voice Acting")
    FQuest_NPCProfile GetNPCProfile(const FString& NPCName) const;

    UFUNCTION(BlueprintCallable, Category = "Voice Acting")
    TArray<FString> GetAvailableNPCs() const;

    UFUNCTION(BlueprintCallable, Category = "Voice Acting")
    void RegisterNPCProfile(const FQuest_NPCProfile& NewProfile);

    // Quest Integration
    UFUNCTION(BlueprintCallable, Category = "Quest Integration")
    void TriggerQuestDialogue(const FString& QuestID, const FString& NPCName);

    UFUNCTION(BlueprintCallable, Category = "Quest Integration")
    void CompleteQuestDialogue(const FString& QuestID, const FString& NPCName);

    UFUNCTION(BlueprintCallable, Category = "Quest Integration")
    bool HasQuestDialogue(const FString& NPCName, const FString& QuestID) const;

    // Audio Management
    UFUNCTION(BlueprintCallable, Category = "Audio Management")
    void SetMasterVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio Management")
    void SetDialogueVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio Management")
    void EnableSubtitles(bool bEnable);

protected:
    // Internal state
    UPROPERTY()
    FString CurrentSpeaker;

    UPROPERTY()
    bool bIsCurrentlyPlaying;

    UPROPERTY()
    float CurrentDialogueTime;

    UPROPERTY()
    float MasterVolume;

    UPROPERTY()
    float DialogueVolume;

    UPROPERTY()
    bool bSubtitlesEnabled;

    // Helper functions
    void InitializeNPCProfiles();
    void LoadDialogueAudio(const FString& AudioURL);
    void UpdateDialoguePlayback(float DeltaTime);
    FQuest_NPCDialogueLine FindDialogueLine(const FString& NPCName, const FString& Context) const;
};