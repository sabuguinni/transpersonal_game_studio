#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "DialogueSystem.generated.h"

// ============================================================
// Enums — global scope (UHT requirement)
// ============================================================

UENUM(BlueprintType)
enum class ENarr_DialogueState : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Greeting        UMETA(DisplayName = "Greeting"),
    QuestGive       UMETA(DisplayName = "QuestGive"),
    QuestProgress   UMETA(DisplayName = "QuestProgress"),
    QuestComplete   UMETA(DisplayName = "QuestComplete"),
    Lore            UMETA(DisplayName = "Lore"),
    Warning         UMETA(DisplayName = "Warning")
};

UENUM(BlueprintType)
enum class ENarr_NPCRole : uint8
{
    TribeElder      UMETA(DisplayName = "TribeElder"),
    Scout           UMETA(DisplayName = "Scout"),
    Crafter         UMETA(DisplayName = "Crafter"),
    Hunter          UMETA(DisplayName = "Hunter"),
    Survivor        UMETA(DisplayName = "Survivor")
};

// ============================================================
// Structs — global scope
// ============================================================

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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float DisplayDuration;

    FNarr_DialogueLine()
        : SpeakerName(TEXT("Elder"))
        , LineText(TEXT(""))
        , TriggerState(ENarr_DialogueState::Idle)
        , DisplayDuration(5.0f)
    {}
};

USTRUCT(BlueprintType)
struct FNarr_DialogueTree
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString TreeID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_DialogueLine> Lines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bHasBeenTriggered;

    FNarr_DialogueTree()
        : TreeID(TEXT(""))
        , bHasBeenTriggered(false)
    {}
};

// ============================================================
// ANarr_TribeElderNPC — Tribe Elder NPC actor
// ============================================================

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ANarr_TribeElderNPC : public AActor
{
    GENERATED_BODY()

public:
    ANarr_TribeElderNPC();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    UStaticMeshComponent* BodyMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    USphereComponent* InteractionSphere;

    // Dialogue state
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_DialogueState CurrentState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_NPCRole NPCRole;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString NPCName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float InteractionRadius;

    // Dialogue trees
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_DialogueTree> DialogueTrees;

    // Quest state flags (set by Quest system)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Quest")
    bool bPlayerHasStoneAxe;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Quest")
    bool bRaptorQuestActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Quest")
    bool bRaptorQuestComplete;

    // Functions
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void InitDialogueTrees();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    FNarr_DialogueLine GetCurrentLine() const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void AdvanceDialogueState();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void OnPlayerEnterRange();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void OnPlayerExitRange();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    FString GetNPCDisplayName() const;

private:
    int32 CurrentLineIndex;
    bool bPlayerInRange;

    UFUNCTION()
    void OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};

// ============================================================
// ANarr_DialogueTrigger — proximity dialogue trigger volume
// ============================================================

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ANarr_DialogueTrigger : public AActor
{
    GENERATED_BODY()

public:
    ANarr_DialogueTrigger();

protected:
    virtual void BeginPlay() override;

public:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    USphereComponent* TriggerSphere;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString TriggerID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString NarrationText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bOneShot;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bHasTriggered;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void FireNarration();

private:
    UFUNCTION()
    void OnTriggerOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);
};
