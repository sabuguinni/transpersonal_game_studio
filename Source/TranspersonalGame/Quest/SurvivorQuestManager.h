#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "SurvivorQuestManager.generated.h"

// ============================================================
// Enums — global scope (RULE 1: no nesting inside UCLASS)
// ============================================================

UENUM(BlueprintType)
enum class EQuest_SurvivorState : uint8
{
    Inactive        UMETA(DisplayName = "Inactive"),
    WatchingHerd    UMETA(DisplayName = "Watching Herd"),
    StampedeActive  UMETA(DisplayName = "Stampede Active"),
    EscapeWindow    UMETA(DisplayName = "Escape Window"),
    Completed       UMETA(DisplayName = "Completed"),
    Failed          UMETA(DisplayName = "Failed")
};

UENUM(BlueprintType)
enum class EQuest_ObjectiveStatus : uint8
{
    Pending     UMETA(DisplayName = "Pending"),
    Active      UMETA(DisplayName = "Active"),
    Completed   UMETA(DisplayName = "Completed"),
    Failed      UMETA(DisplayName = "Failed")
};

// ============================================================
// Structs — global scope
// ============================================================

USTRUCT(BlueprintType)
struct FQuest_SurvivorObjective
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Quest|Survivor")
    FString ObjectiveText;

    UPROPERTY(BlueprintReadWrite, Category = "Quest|Survivor")
    EQuest_ObjectiveStatus Status;

    UPROPERTY(BlueprintReadWrite, Category = "Quest|Survivor")
    float ProgressCurrent;

    UPROPERTY(BlueprintReadWrite, Category = "Quest|Survivor")
    float ProgressRequired;

    FQuest_SurvivorObjective()
        : ObjectiveText(TEXT(""))
        , Status(EQuest_ObjectiveStatus::Pending)
        , ProgressCurrent(0.f)
        , ProgressRequired(1.f)
    {}
};

USTRUCT(BlueprintType)
struct FQuest_SurvivorReward
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Quest|Survivor")
    int32 SurvivalXP;

    UPROPERTY(BlueprintReadWrite, Category = "Quest|Survivor")
    int32 CraftingXP;

    UPROPERTY(BlueprintReadWrite, Category = "Quest|Survivor")
    FString UnlockedRecipe;

    FQuest_SurvivorReward()
        : SurvivalXP(150)
        , CraftingXP(50)
        , UnlockedRecipe(TEXT("Bone Spear"))
    {}
};

// ============================================================
// AQuest_SurvivorQuestManager — placed actor in MinPlayableMap
// ============================================================

UCLASS(ClassGroup = "Quest", BlueprintType, Blueprintable,
       meta = (DisplayName = "Survivor Quest Manager"))
class TRANSPERSONALGAME_API AQuest_SurvivorQuestManager : public AActor
{
    GENERATED_BODY()

public:
    AQuest_SurvivorQuestManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // ── Collision zones ──────────────────────────────────────
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest|Zones",
              meta = (AllowPrivateAccess = "true"))
    USphereComponent* HerdZoneTrigger;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest|Zones",
              meta = (AllowPrivateAccess = "true"))
    USphereComponent* SafeZoneTrigger;

    // ── Quest state ──────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Quest|State",
              meta = (AllowPrivateAccess = "true"))
    EQuest_SurvivorState CurrentState;

    UPROPERTY(BlueprintReadOnly, Category = "Quest|State",
              meta = (AllowPrivateAccess = "true"))
    bool bPlayerInHerdZone;

    UPROPERTY(BlueprintReadOnly, Category = "Quest|State",
              meta = (AllowPrivateAccess = "true"))
    bool bPlayerInSafeZone;

    UPROPERTY(BlueprintReadOnly, Category = "Quest|State",
              meta = (AllowPrivateAccess = "true"))
    float StampedeElapsedTime;

    // ── Quest config ─────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Config")
    float HerdZoneRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Config")
    float SafeZoneRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Config")
    float StampedeTimeLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Config")
    float StampedeActivationDelay;

    // ── Objectives ───────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Quest|Objectives",
              meta = (AllowPrivateAccess = "true"))
    TArray<FQuest_SurvivorObjective> Objectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Rewards")
    FQuest_SurvivorReward QuestReward;

    // ── Audio ─────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Audio")
    FString WarningSoundURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Audio")
    FString CompleteSoundURL;

public:
    // ── Blueprint callable interface ─────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Quest|Survivor")
    void ActivateQuest();

    UFUNCTION(BlueprintCallable, Category = "Quest|Survivor")
    void TriggerStampedePhase();

    UFUNCTION(BlueprintCallable, Category = "Quest|Survivor")
    void CompleteQuest();

    UFUNCTION(BlueprintCallable, Category = "Quest|Survivor")
    void FailQuest();

    UFUNCTION(BlueprintPure, Category = "Quest|Survivor")
    EQuest_SurvivorState GetCurrentState() const { return CurrentState; }

    UFUNCTION(BlueprintPure, Category = "Quest|Survivor")
    bool IsStampedeActive() const { return CurrentState == EQuest_SurvivorState::StampedeActive; }

    UFUNCTION(BlueprintPure, Category = "Quest|Survivor")
    float GetStampedeTimeRemaining() const;

    UFUNCTION(BlueprintPure, Category = "Quest|Survivor")
    TArray<FQuest_SurvivorObjective> GetObjectives() const { return Objectives; }

    UFUNCTION(BlueprintCallable, Category = "Quest|Survivor")
    void UpdateObjectiveProgress(int32 ObjectiveIndex, float Progress);

    UFUNCTION(BlueprintCallable, Category = "Quest|Debug", CallInEditor)
    void DebugPrintQuestState();

private:
    void InitialiseObjectives();
    void CheckObjectiveCompletion();
    void AwardRewards();

    UFUNCTION()
    void OnHerdZoneBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                                 UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
                                 bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnHerdZoneEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                               UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    UFUNCTION()
    void OnSafeZoneBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                                 UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
                                 bool bFromSweep, const FHitResult& SweepResult);

    float ActivationTimer;
    bool bQuestInitialised;
};
