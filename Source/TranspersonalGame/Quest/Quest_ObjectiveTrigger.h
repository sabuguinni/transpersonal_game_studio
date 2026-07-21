#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "SharedTypes.h"
#include "Quest_ObjectiveTrigger.generated.h"

UENUM(BlueprintType)
enum class EQuest_TriggerType : uint8
{
    LocationReached     UMETA(DisplayName = "Location Reached"),
    ItemCollected       UMETA(DisplayName = "Item Collected"),
    EnemyDefeated       UMETA(DisplayName = "Enemy Defeated"),
    InteractionComplete UMETA(DisplayName = "Interaction Complete"),
    TimeElapsed         UMETA(DisplayName = "Time Elapsed"),
    VariableCheck       UMETA(DisplayName = "Variable Check")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_TriggerCondition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger Condition")
    EQuest_TriggerType TriggerType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger Condition")
    FString RequiredValue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger Condition")
    int32 RequiredCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger Condition")
    float RequiredTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger Condition")
    bool bInvertCondition;

    FQuest_TriggerCondition()
    {
        TriggerType = EQuest_TriggerType::LocationReached;
        RequiredValue = TEXT("");
        RequiredCount = 1;
        RequiredTime = 0.0f;
        bInvertCondition = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_ObjectiveTrigger : public AActor
{
    GENERATED_BODY()

public:
    AQuest_ObjectiveTrigger();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Quest Trigger")
    void ActivateTrigger();

    UFUNCTION(BlueprintCallable, Category = "Quest Trigger")
    void DeactivateTrigger();

    UFUNCTION(BlueprintCallable, Category = "Quest Trigger")
    bool CheckTriggerConditions();

    UFUNCTION(BlueprintCallable, Category = "Quest Trigger")
    void SetMissionObjective(const FString& MissionID, int32 ObjectiveIndex);

    UFUNCTION(BlueprintCallable, Category = "Quest Trigger")
    void TriggerObjectiveComplete();

    UFUNCTION(BlueprintImplementableEvent, Category = "Quest Trigger")
    void OnObjectiveTriggered();

    UFUNCTION(BlueprintImplementableEvent, Category = "Quest Trigger")
    void OnTriggerActivated();

    UFUNCTION(BlueprintImplementableEvent, Category = "Quest Trigger")
    void OnTriggerDeactivated();

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USphereComponent* TriggerSphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UStaticMeshComponent* VisualMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Configuration")
    FString TargetMissionID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Configuration")
    int32 TargetObjectiveIndex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Configuration")
    TArray<FQuest_TriggerCondition> TriggerConditions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Configuration")
    bool bRequireAllConditions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Configuration")
    bool bSingleUse;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Configuration")
    bool bAutoActivate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Configuration")
    float TriggerRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Configuration")
    FString RequiredPlayerTag;

    UPROPERTY(BlueprintReadOnly, Category = "Quest State")
    bool bIsActive;

    UPROPERTY(BlueprintReadOnly, Category = "Quest State")
    bool bHasBeenTriggered;

    UPROPERTY(BlueprintReadOnly, Category = "Quest State")
    float ActivationTime;

    UPROPERTY(BlueprintReadOnly, Category = "Quest State")
    int32 CurrentTriggerCount;

private:
    UFUNCTION()
    void OnTriggerSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
                                   UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, 
                                   bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnTriggerSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
                                 UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    bool CheckLocationCondition(const FQuest_TriggerCondition& Condition);
    bool CheckItemCondition(const FQuest_TriggerCondition& Condition);
    bool CheckEnemyCondition(const FQuest_TriggerCondition& Condition);
    bool CheckInteractionCondition(const FQuest_TriggerCondition& Condition);
    bool CheckTimeCondition(const FQuest_TriggerCondition& Condition);
    bool CheckVariableCondition(const FQuest_TriggerCondition& Condition);

    void UpdateVisualState();
    class UQuest_MissionManager* GetMissionManager();

    TArray<AActor*> OverlappingActors;
    float LastCheckTime;
    static const float CHECK_INTERVAL;
};