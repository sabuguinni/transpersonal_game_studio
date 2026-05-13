#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "Narr_DialogueSystem.generated.h"

// Forward declarations
class UNarr_DialogueNode;
class UNarr_CharacterProfile;

UENUM(BlueprintType)
enum class ENarr_DialogueState : uint8
{
    Inactive,
    WaitingForInput,
    Playing,
    Completed,
    Interrupted
};

UENUM(BlueprintType)
enum class ENarr_CharacterRole : uint8
{
    Elder,
    Scout,
    Tracker,
    FireKeeper,
    Hunter,
    Crafter,
    Healer,
    Survivor
};

USTRUCT(BlueprintType)
struct FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FText DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ENarr_CharacterRole SpeakerRole;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString AudioCuePath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bRequiresPlayerResponse;

    FNarr_DialogueLine()
    {
        SpeakerName = TEXT("Unknown");
        DialogueText = FText::FromString(TEXT("..."));
        SpeakerRole = ENarr_CharacterRole::Survivor;
        Duration = 3.0f;
        AudioCuePath = TEXT("");
        bRequiresPlayerResponse = false;
    }
};

USTRUCT(BlueprintType)
struct FNarr_DialogueChoice
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FText ChoiceText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    int32 NextNodeID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bRequiresItem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString RequiredItemName;

    FNarr_DialogueChoice()
    {
        ChoiceText = FText::FromString(TEXT("Continue"));
        NextNodeID = -1;
        bRequiresItem = false;
        RequiredItemName = TEXT("");
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UNarr_DialogueNode : public UObject
{
    GENERATED_BODY()

public:
    UNarr_DialogueNode();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    int32 NodeID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FNarr_DialogueLine DialogueLine;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FNarr_DialogueChoice> PlayerChoices;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bIsQuestNode;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString QuestID;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool HasValidChoices() const;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    FNarr_DialogueChoice GetChoiceByIndex(int32 Index) const;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UNarr_CharacterProfile : public UObject
{
    GENERATED_BODY()

public:
    UNarr_CharacterProfile();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    FString CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    ENarr_CharacterRole Role;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    FText CharacterDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    TArray<FString> VoiceLinePaths;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    FString DefaultGreeting;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    TArray<int32> AvailableDialogueNodes;
};

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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    UNarr_CharacterProfile* CharacterProfile;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<UNarr_DialogueNode*> DialogueNodes;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    ENarr_DialogueState CurrentState;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    int32 CurrentNodeID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float InteractionRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bCanRepeatDialogues;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool StartDialogue(int32 NodeID = 0);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void EndDialogue();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool SelectChoice(int32 ChoiceIndex);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    UNarr_DialogueNode* GetCurrentNode() const;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool IsInRange(AActor* Player) const;

    UFUNCTION(BlueprintImplementableEvent, Category = "Dialogue")
    void OnDialogueStarted(const FNarr_DialogueLine& DialogueLine);

    UFUNCTION(BlueprintImplementableEvent, Category = "Dialogue")
    void OnDialogueEnded();

    UFUNCTION(BlueprintImplementableEvent, Category = "Dialogue")
    void OnChoicesPresented(const TArray<FNarr_DialogueChoice>& Choices);

private:
    void ProcessCurrentNode();
    UNarr_DialogueNode* FindNodeByID(int32 NodeID) const;
};

UCLASS()
class TRANSPERSONALGAME_API UNarr_DialogueSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    TArray<UNarr_CharacterProfile*> RegisteredCharacters;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void RegisterCharacter(UNarr_CharacterProfile* Character);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    UNarr_CharacterProfile* GetCharacterByName(const FString& Name) const;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    TArray<UNarr_CharacterProfile*> GetCharactersByRole(ENarr_CharacterRole Role) const;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void InitializeDefaultCharacters();

private:
    void CreateElderKarak();
    void CreateScoutZara();
    void CreateTrackerJoren();
    void CreateFireKeeperMira();
};