#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "../SharedTypes.h"
#include "Narr_DialogueSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FText DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString AudioPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FString> ResponseOptions;

    FNarr_DialogueEntry()
    {
        SpeakerName = TEXT("Unknown");
        DialogueText = FText::FromString(TEXT("..."));
        AudioPath = TEXT("");
        Duration = 3.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueTree
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString TreeID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FNarr_DialogueEntry> Entries;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    int32 CurrentEntryIndex;

    FNarr_DialogueTree()
    {
        TreeID = TEXT("DefaultTree");
        CurrentEntryIndex = 0;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNarr_DialogueComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNarr_DialogueComponent();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FNarr_DialogueTree> DialogueTrees;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString CurrentTreeID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bIsDialogueActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float InteractionRange;

public:
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void StartDialogue(const FString& TreeID);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void EndDialogue();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    FNarr_DialogueEntry GetCurrentEntry();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void AdvanceDialogue();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool CanInteract(AActor* InteractingActor);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void AddDialogueTree(const FNarr_DialogueTree& NewTree);
};

UCLASS()
class TRANSPERSONALGAME_API UNarr_DialogueManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

protected:
    UPROPERTY()
    TMap<FString, FNarr_DialogueTree> GlobalDialogueTrees;

    UPROPERTY()
    TArray<FString> CompletedDialogues;

public:
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void RegisterDialogueTree(const FString& TreeID, const FNarr_DialogueTree& Tree);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    FNarr_DialogueTree GetDialogueTree(const FString& TreeID);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void MarkDialogueCompleted(const FString& TreeID);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool IsDialogueCompleted(const FString& TreeID);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    TArray<FString> GetAvailableDialogues();
};