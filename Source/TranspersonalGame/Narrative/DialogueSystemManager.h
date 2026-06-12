#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "DialogueSystemManager.generated.h"

UENUM(BlueprintType)
enum class ENarr_DialogueType : uint8
{
    Story           UMETA(DisplayName = "Story Dialogue"),
    Quest           UMETA(DisplayName = "Quest Dialogue"), 
    Tutorial        UMETA(DisplayName = "Tutorial Dialogue"),
    Ambient         UMETA(DisplayName = "Ambient Dialogue"),
    Warning         UMETA(DisplayName = "Warning Dialogue")
};

UENUM(BlueprintType)
enum class ENarr_NPCType : uint8
{
    Elder           UMETA(DisplayName = "Tribal Elder"),
    Hunter          UMETA(DisplayName = "Hunter"),
    Gatherer        UMETA(DisplayName = "Gatherer"),
    Shaman          UMETA(DisplayName = "Shaman"),
    Child           UMETA(DisplayName = "Child"),
    Warrior         UMETA(DisplayName = "Warrior")
};

USTRUCT(BlueprintType)
struct FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ENarr_DialogueType DialogueType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ENarr_NPCType SpeakerType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundCue> VoiceAudio;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Conditions")
    TArray<FString> RequiredConditions;

    FNarr_DialogueLine()
    {
        DialogueText = TEXT("");
        DialogueType = ENarr_DialogueType::Ambient;
        SpeakerType = ENarr_NPCType::Elder;
        Duration = 3.0f;
        VoiceAudio = nullptr;
    }
};

USTRUCT(BlueprintType)
struct FNarr_DialogueTree
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue Tree")
    FString TreeID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue Tree")
    TArray<FNarr_DialogueLine> DialogueLines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue Tree")
    TArray<FString> PlayerChoices;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue Tree")
    bool bIsRepeatable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue Tree")
    int32 Priority;

    FNarr_DialogueTree()
    {
        TreeID = TEXT("");
        bIsRepeatable = false;
        Priority = 1;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ADialogueSystemManager : public AActor
{
    GENERATED_BODY()

public:
    ADialogueSystemManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue System")
    TArray<FNarr_DialogueTree> DialogueTrees;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue System")
    TMap<FString, bool> DialogueConditions;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UAudioComponent* AudioComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float DialogueVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float DialogueFadeTime;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Dialogue System")
    void StartDialogue(const FString& TreeID, ENarr_NPCType NPCType);

    UFUNCTION(BlueprintCallable, Category = "Dialogue System")
    void StopDialogue();

    UFUNCTION(BlueprintCallable, Category = "Dialogue System")
    bool IsDialogueActive() const;

    UFUNCTION(BlueprintCallable, Category = "Dialogue System")
    void SetDialogueCondition(const FString& ConditionName, bool bValue);

    UFUNCTION(BlueprintCallable, Category = "Dialogue System")
    bool CheckDialogueCondition(const FString& ConditionName) const;

    UFUNCTION(BlueprintCallable, Category = "Dialogue System")
    TArray<FNarr_DialogueTree> GetAvailableDialogues(ENarr_NPCType NPCType) const;

    UFUNCTION(BlueprintCallable, Category = "Dialogue System")
    void RegisterDialogueTree(const FNarr_DialogueTree& NewTree);

    UFUNCTION(BlueprintCallable, Category = "Dialogue System")
    void PlayDialogueAudio(const FNarr_DialogueLine& DialogueLine);

protected:
    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bIsDialogueActive;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    FString CurrentTreeID;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    int32 CurrentLineIndex;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    float DialogueTimer;

    UFUNCTION(BlueprintImplementableEvent, Category = "Dialogue Events")
    void OnDialogueStarted(const FString& TreeID);

    UFUNCTION(BlueprintImplementableEvent, Category = "Dialogue Events")
    void OnDialogueLineChanged(const FNarr_DialogueLine& CurrentLine);

    UFUNCTION(BlueprintImplementableEvent, Category = "Dialogue Events")
    void OnDialogueEnded(const FString& TreeID);

private:
    void ProcessDialogueLine();
    bool CheckLineConditions(const FNarr_DialogueLine& Line) const;
    void InitializeDefaultDialogues();
};