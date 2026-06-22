// DialogueSystem.h
// Agent #15 — Narrative & Dialogue Agent
// Prehistoric survival dialogue system — no spiritual content, pure survival realism

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SharedTypes.h"
#include "DialogueSystem.generated.h"

// ── Enums (global scope, Narr_ prefix) ──────────────────────────────────────

UENUM(BlueprintType)
enum class ENarr_DialogueType : uint8
{
    Warning         UMETA(DisplayName = "Warning"),
    QuestGive       UMETA(DisplayName = "QuestGive"),
    QuestComplete   UMETA(DisplayName = "QuestComplete"),
    Lore            UMETA(DisplayName = "Lore"),
    Ambient         UMETA(DisplayName = "Ambient"),
    Urgent          UMETA(DisplayName = "Urgent")
};

UENUM(BlueprintType)
enum class ENarr_NPCRole : uint8
{
    TribalElder     UMETA(DisplayName = "TribalElder"),
    Scout           UMETA(DisplayName = "Scout"),
    Hunter          UMETA(DisplayName = "Hunter"),
    Craftsman       UMETA(DisplayName = "Craftsman"),
    Survivor        UMETA(DisplayName = "Survivor")
};

// ── Structs (global scope) ───────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString LineText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_DialogueType DialogueType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float DisplayDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bRequiresPlayerResponse;

    FNarr_DialogueLine()
        : SpeakerName(TEXT("Unknown"))
        , LineText(TEXT(""))
        , DialogueType(ENarr_DialogueType::Ambient)
        , DisplayDuration(4.0f)
        , bRequiresPlayerResponse(false)
    {}
};

USTRUCT(BlueprintType)
struct FNarr_DialogueTree
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FName TreeID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_DialogueLine> Lines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FName NextTreeID;

    FNarr_DialogueTree()
        : TreeID(NAME_None)
        , bIsCompleted(false)
        , NextTreeID(NAME_None)
    {}
};

// ── NPC Dialogue Actor ───────────────────────────────────────────────────────

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ANarr_DialogueNPC : public AActor
{
    GENERATED_BODY()

public:
    ANarr_DialogueNPC();

    // NPC identity
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|NPC")
    FString NPCName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|NPC")
    ENarr_NPCRole NPCRole;

    // Dialogue data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    TArray<FNarr_DialogueTree> DialogueTrees;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    FName ActiveTreeID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    float InteractionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    bool bIsInDialogue;

    // Mesh component
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components",
        meta = (AllowPrivateAccess = "true"))
    UStaticMeshComponent* NPCMesh;

    // Sphere collision for interaction
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components",
        meta = (AllowPrivateAccess = "true"))
    class USphereComponent* InteractionSphere;

    // Methods
    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    void StartDialogue(AActor* Initiator);

    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    void AdvanceDialogue();

    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    void EndDialogue();

    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    FNarr_DialogueLine GetCurrentLine() const;

    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    bool HasActiveDialogue() const;

    UFUNCTION(BlueprintPure, Category = "Narrative|NPC")
    FString GetNPCDisplayName() const;

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

private:
    int32 CurrentLineIndex;
    int32 CurrentTreeIndex;

    void LoadDefaultDialogue();
    FNarr_DialogueTree* FindTree(FName TreeID);
};

// ── Dialogue Manager (World Subsystem) ──────────────────────────────────────

UCLASS()
class TRANSPERSONALGAME_API UNarr_DialogueManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Narrative|Manager")
    void RegisterNPC(ANarr_DialogueNPC* NPC);

    UFUNCTION(BlueprintCallable, Category = "Narrative|Manager")
    void UnregisterNPC(ANarr_DialogueNPC* NPC);

    UFUNCTION(BlueprintCallable, Category = "Narrative|Manager")
    ANarr_DialogueNPC* FindNearestNPC(FVector PlayerLocation, float MaxRadius = 500.0f) const;

    UFUNCTION(BlueprintCallable, Category = "Narrative|Manager")
    bool IsAnyDialogueActive() const;

    UFUNCTION(BlueprintPure, Category = "Narrative|Manager")
    int32 GetRegisteredNPCCount() const;

private:
    UPROPERTY()
    TArray<ANarr_DialogueNPC*> RegisteredNPCs;

    bool bAnyDialogueActive;
};
