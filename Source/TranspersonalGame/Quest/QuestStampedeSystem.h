#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "QuestStampedeSystem.generated.h"

// ============================================================
// Quest: "The Stampede"
// Player must reach high ground before the fleeing herd
// overruns their camp. Wires to CrowdHerdBehavior::TriggerFlee.
// ============================================================

UENUM(BlueprintType)
enum class EQuest_StampedeState : uint8
{
    Inactive     UMETA(DisplayName = "Inactive"),
    Active       UMETA(DisplayName = "Active"),
    Succeeded    UMETA(DisplayName = "Succeeded"),
    Failed       UMETA(DisplayName = "Failed")
};

USTRUCT(BlueprintType)
struct FQuest_StampedeObjective
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString ObjectiveText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FVector TargetLocation;

    FQuest_StampedeObjective()
        : ObjectiveText(TEXT("Reach high ground"))
        , bCompleted(false)
        , TargetLocation(FVector::ZeroVector)
    {}
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_StampedeManager : public AActor
{
    GENERATED_BODY()

public:
    AQuest_StampedeManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // --- Quest State ---
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest|State")
    EQuest_StampedeState QuestState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Config")
    float TimeLimit;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest|State")
    float TimeRemaining;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Config")
    float HighGroundMinZ;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Config")
    float HighGroundRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Config")
    FVector HighGroundCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Config")
    FVector CampLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Config")
    float CampDangerRadius;

    // --- Herd reference (set by CrowdHerdBehavior) ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Herd")
    AActor* HerdActor;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest|Herd")
    bool bHerdTriggered;

    // --- Objectives ---
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest|Objectives")
    TArray<FQuest_StampedeObjective> Objectives;

    // --- UFUNCTIONs ---
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void StartQuest();

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void TriggerHerdStampede();

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void CompleteQuest(bool bSuccess);

    UFUNCTION(BlueprintPure, Category = "Quest")
    bool IsPlayerOnHighGround() const;

    UFUNCTION(BlueprintPure, Category = "Quest")
    bool IsHerdNearCamp() const;

    UFUNCTION(BlueprintPure, Category = "Quest")
    float GetTimeRemainingNormalized() const;

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void ResetQuest();

private:
    void CheckQuestConditions(float DeltaTime);
    APawn* GetPlayerPawn() const;
};
