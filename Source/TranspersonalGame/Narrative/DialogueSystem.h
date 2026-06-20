#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "DialogueSystem.generated.h"

// ─── Enums ───────────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class ENarr_QuestType : uint8
{
    Hunt        UMETA(DisplayName = "Hunt"),
    Gather      UMETA(DisplayName = "Gather"),
    Defend      UMETA(DisplayName = "Defend"),
    Explore     UMETA(DisplayName = "Explore")
};

UENUM(BlueprintType)
enum class ENarr_DialogueState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Greeting    UMETA(DisplayName = "Greeting"),
    QuestOffer  UMETA(DisplayName = "QuestOffer"),
    QuestActive UMETA(DisplayName = "QuestActive"),
    QuestDone   UMETA(DisplayName = "QuestDone")
};

// ─── Structs ─────────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString LineText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_DialogueState TriggerState;

    FNarr_DialogueLine()
        : SpeakerName(TEXT("Unknown"))
        , LineText(TEXT("..."))
        , TriggerState(ENarr_DialogueState::Idle)
    {}
};

USTRUCT(BlueprintType)
struct FNarr_QuestDialogue
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString QuestID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_QuestType QuestType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_DialogueLine> Lines;

    FNarr_QuestDialogue()
        : QuestID(TEXT(""))
        , QuestType(ENarr_QuestType::Hunt)
    {}
};

// ─── NPC Dialogue Actor ───────────────────────────────────────────────────────

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ANarr_DialogueNPC : public AActor
{
    GENERATED_BODY()

public:
    ANarr_DialogueNPC();

    // Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components",
        meta = (AllowPrivateAccess = "true"))
    UStaticMeshComponent* MeshComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components",
        meta = (AllowPrivateAccess = "true"))
    USphereComponent* InteractionRadius;

    // Dialogue data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString NPCName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_DialogueState CurrentState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_QuestDialogue> QuestDialogues;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bQuestAvailable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bQuestCompleted;

    // Functions
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void InitializeDialogues();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    FNarr_DialogueLine GetCurrentLine() const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void AdvanceDialogue();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void OnQuestCompleted(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    TArray<FNarr_DialogueLine> GetLinesForState(ENarr_DialogueState State) const;

protected:
    virtual void BeginPlay() override;

    UFUNCTION()
    void OnPlayerEnterRange(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);

private:
    int32 CurrentLineIndex;
};

// ─── Dialogue Manager (World Subsystem) ──────────────────────────────────────

UCLASS()
class TRANSPERSONALGAME_API UNarr_DialogueManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void RegisterNPC(ANarr_DialogueNPC* NPC);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void UnregisterNPC(ANarr_DialogueNPC* NPC);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    ANarr_DialogueNPC* FindNPCByName(const FString& Name) const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void NotifyQuestCompleted(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    int32 GetRegisteredNPCCount() const;

private:
    UPROPERTY()
    TArray<ANarr_DialogueNPC*> RegisteredNPCs;
};
