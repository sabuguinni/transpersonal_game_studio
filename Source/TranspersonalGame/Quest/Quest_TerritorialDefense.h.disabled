#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/TriggerVolume.h"
#include "../SharedTypes.h"
#include "Quest_TerritorialDefense.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_TerritoryData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    FVector TerritoryCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    float TerritoryRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    TArray<AActor*> DefendingDinosaurs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    TArray<AActor*> IntruderDinosaurs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    float DefenseStartTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    bool bIsActive;

    FQuest_TerritoryData()
    {
        TerritoryCenter = FVector::ZeroVector;
        TerritoryRadius = 5000.0f;
        DefenseStartTime = 0.0f;
        bIsActive = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_DefenseObjective
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    FString ObjectiveText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    int32 RequiredDefenses;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    int32 CurrentDefenses;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    float TimeLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    bool bCompleted;

    FQuest_DefenseObjective()
    {
        ObjectiveText = TEXT("Defend territory from intruders");
        RequiredDefenses = 3;
        CurrentDefenses = 0;
        TimeLimit = 300.0f; // 5 minutes
        bCompleted = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_TerritorialDefense : public AActor
{
    GENERATED_BODY()

public:
    AQuest_TerritorialDefense();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Core Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* QuestTrigger;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* QuestMarker;

    // Quest Data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FQuest_TerritoryData TerritoryData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FQuest_DefenseObjective DefenseObjective;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FVector> PatrolPoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float QuestActivationRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bQuestActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bQuestCompleted;

    // Territory Management
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void StartTerritorialDefenseQuest();

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void EndTerritorialDefenseQuest(bool bSuccess);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void DetectTerritorialConflict();

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void RegisterDefenseAction(AActor* Defender, AActor* Intruder);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool ValidateDefenseSuccess();

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void UpdateQuestProgress();

    // Territory Analysis
    UFUNCTION(BlueprintCallable, Category = "Quest")
    TArray<AActor*> FindDinosaursInTerritory(FVector Center, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool IsTerritorialSpecies(AActor* DinosaurActor);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void SetupPatrolRoute();

    // Quest Events
    UFUNCTION(BlueprintImplementableEvent, Category = "Quest")
    void OnQuestStarted();

    UFUNCTION(BlueprintImplementableEvent, Category = "Quest")
    void OnDefenseRegistered();

    UFUNCTION(BlueprintImplementableEvent, Category = "Quest")
    void OnQuestCompleted(bool bSuccess);

    UFUNCTION(BlueprintImplementableEvent, Category = "Quest")
    void OnQuestFailed();

    // Trigger Events
    UFUNCTION()
    void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:
    float QuestTimer;
    float ConflictDetectionTimer;
    bool bPlayerInRange;

    void InitializeQuestMarker();
    void UpdateQuestMarkerVisibility();
    void CheckQuestCompletion();
    void HandleQuestTimeout();
};