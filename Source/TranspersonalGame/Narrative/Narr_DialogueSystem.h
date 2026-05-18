#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "Narr_DialogueSystem.generated.h"

// Dialogue node types for branching conversations
UENUM(BlueprintType)
enum class ENarr_DialogueNodeType : uint8
{
    Standard        UMETA(DisplayName = "Standard"),
    Choice          UMETA(DisplayName = "Player Choice"),
    Conditional     UMETA(DisplayName = "Conditional"),
    QuestGiver      UMETA(DisplayName = "Quest Giver"),
    Trader          UMETA(DisplayName = "Trader"),
    Warning         UMETA(DisplayName = "Warning"),
    Lore            UMETA(DisplayName = "Lore")
};

// Character archetypes for dialogue system
UENUM(BlueprintType)
enum class ENarr_CharacterArchetype : uint8
{
    TribalHunter    UMETA(DisplayName = "Tribal Hunter"),
    TribalGatherer  UMETA(DisplayName = "Tribal Gatherer"),
    TribalElder     UMETA(DisplayName = "Tribal Elder"),
    TribalScout     UMETA(DisplayName = "Tribal Scout"),
    TribalCraftsman UMETA(DisplayName = "Tribal Craftsman"),
    Narrator        UMETA(DisplayName = "Narrator"),
    Survivor        UMETA(DisplayName = "Fellow Survivor"),
    Stranger        UMETA(DisplayName = "Mysterious Stranger")
};

// Dialogue trigger conditions
UENUM(BlueprintType)
enum class ENarr_DialogueTrigger : uint8
{
    Proximity       UMETA(DisplayName = "Player Proximity"),
    QuestComplete   UMETA(DisplayName = "Quest Completed"),
    QuestActive     UMETA(DisplayName = "Quest Active"),
    ItemPossessed   UMETA(DisplayName = "Has Item"),
    TimeOfDay       UMETA(DisplayName = "Time of Day"),
    LocationBased   UMETA(DisplayName = "Location Based"),
    HealthLow       UMETA(DisplayName = "Low Health"),
    FirstVisit      UMETA(DisplayName = "First Visit")
};

// Single dialogue line data structure
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FText DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ENarr_CharacterArchetype SpeakerArchetype;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float DisplayDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString VoiceAudioPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FString> PlayerChoices;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<int32> NextDialogueIDs;

    FNarr_DialogueLine()
    {
        SpeakerName = TEXT("Unknown");
        DialogueText = FText::FromString(TEXT("..."));
        SpeakerArchetype = ENarr_CharacterArchetype::Survivor;
        DisplayDuration = 3.0f;
        VoiceAudioPath = TEXT("");
    }
};

// Complete dialogue tree structure
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueTree
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue Tree")
    FString TreeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue Tree")
    ENarr_DialogueNodeType TreeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue Tree")
    ENarr_DialogueTrigger TriggerCondition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue Tree")
    TArray<FNarr_DialogueLine> DialogueLines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue Tree")
    int32 CurrentLineIndex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue Tree")
    bool bIsRepeatable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue Tree")
    bool bHasBeenTriggered;

    FNarr_DialogueTree()
    {
        TreeName = TEXT("Default_Tree");
        TreeType = ENarr_DialogueNodeType::Standard;
        TriggerCondition = ENarr_DialogueTrigger::Proximity;
        CurrentLineIndex = 0;
        bIsRepeatable = true;
        bHasBeenTriggered = false;
    }
};

// Main dialogue system component
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNarr_DialogueComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNarr_DialogueComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Dialogue management functions
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool StartDialogue(const FString& TreeName);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void EndDialogue();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool AdvanceDialogue();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void SelectPlayerChoice(int32 ChoiceIndex);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    FNarr_DialogueLine GetCurrentDialogueLine() const;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool IsDialogueActive() const { return bDialogueActive; }

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void RegisterDialogueTree(const FNarr_DialogueTree& NewTree);

    // Trigger condition checking
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool CheckTriggerCondition(ENarr_DialogueTrigger TriggerType, const FString& ConditionData = TEXT(""));

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue System")
    TArray<FNarr_DialogueTree> DialogueTrees;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue System")
    FString CurrentTreeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue System")
    bool bDialogueActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue System")
    float ProximityTriggerDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue System")
    AActor* PlayerReference;

private:
    FNarr_DialogueTree* GetDialogueTree(const FString& TreeName);
    void UpdateProximityTriggers();
};

// Dialogue manager actor for scene-wide dialogue coordination
UCLASS()
class TRANSPERSONALGAME_API ANarr_DialogueManager : public AActor
{
    GENERATED_BODY()

public:
    ANarr_DialogueManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Global dialogue management
    UFUNCTION(BlueprintCallable, Category = "Dialogue Manager")
    void InitializeDialogueSystem();

    UFUNCTION(BlueprintCallable, Category = "Dialogue Manager")
    void RegisterDialogueActor(AActor* DialogueActor);

    UFUNCTION(BlueprintCallable, Category = "Dialogue Manager")
    void TriggerLocationBasedDialogue(const FString& LocationName);

    UFUNCTION(BlueprintCallable, Category = "Dialogue Manager")
    void SetGlobalDialogueFlag(const FString& FlagName, bool bValue);

    UFUNCTION(BlueprintCallable, Category = "Dialogue Manager")
    bool GetGlobalDialogueFlag(const FString& FlagName) const;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue Manager")
    TArray<AActor*> RegisteredDialogueActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue Manager")
    TMap<FString, bool> GlobalDialogueFlags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue Manager")
    UNarr_DialogueComponent* DialogueComponent;

private:
    void LoadPresetDialogueTrees();
    void CreateTribalHunterDialogue();
    void CreateTribalGathererDialogue();
    void CreateTribalElderDialogue();
    void CreateNarratorDialogue();
};

#include "Narr_DialogueSystem.generated.h"