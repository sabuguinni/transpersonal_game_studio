#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "../SharedTypes.h"
#include "Quest_PrehistoricQuestTriggerSystem.generated.h"

UENUM(BlueprintType)
enum class EQuest_TriggerType : uint8
{
    Location        UMETA(DisplayName = "Location Based"),
    Proximity       UMETA(DisplayName = "Proximity to Actor"),
    Interaction     UMETA(DisplayName = "Interaction with Object"),
    Timer           UMETA(DisplayName = "Time Based"),
    Resource        UMETA(DisplayName = "Resource Collection"),
    Combat          UMETA(DisplayName = "Combat Event"),
    Discovery       UMETA(DisplayName = "Discovery Event"),
    Survival        UMETA(DisplayName = "Survival Condition")
};

USTRUCT(BlueprintType)
struct FQuest_TriggerCondition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Trigger")
    EQuest_TriggerType TriggerType = EQuest_TriggerType::Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Trigger")
    FVector TargetLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Trigger")
    float TriggerRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Trigger")
    FString TargetActorTag = "";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Trigger")
    int32 RequiredQuantity = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Trigger")
    float TimerDuration = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Trigger")
    bool bIsActive = true;

    FQuest_TriggerCondition()
    {
        TriggerType = EQuest_TriggerType::Location;
        TargetLocation = FVector::ZeroVector;
        TriggerRadius = 500.0f;
        TargetActorTag = "";
        RequiredQuantity = 1;
        TimerDuration = 60.0f;
        bIsActive = true;
    }
};

UCLASS(ClassGroup=(Quest), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UQuest_PrehistoricQuestTriggerSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UQuest_PrehistoricQuestTriggerSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Quest trigger management
    UFUNCTION(BlueprintCallable, Category = "Quest Trigger")
    void RegisterQuestTrigger(const FString& QuestID, const FQuest_TriggerCondition& TriggerCondition);

    UFUNCTION(BlueprintCallable, Category = "Quest Trigger")
    void UnregisterQuestTrigger(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest Trigger")
    bool CheckTriggerCondition(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest Trigger")
    void ActivateTrigger(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest Trigger")
    void DeactivateTrigger(const FString& QuestID);

    // Specific trigger types
    UFUNCTION(BlueprintCallable, Category = "Quest Trigger")
    bool CheckLocationTrigger(const FQuest_TriggerCondition& Condition);

    UFUNCTION(BlueprintCallable, Category = "Quest Trigger")
    bool CheckProximityTrigger(const FQuest_TriggerCondition& Condition);

    UFUNCTION(BlueprintCallable, Category = "Quest Trigger")
    bool CheckResourceTrigger(const FQuest_TriggerCondition& Condition);

    UFUNCTION(BlueprintCallable, Category = "Quest Trigger")
    bool CheckTimerTrigger(const FString& QuestID, const FQuest_TriggerCondition& Condition);

    // Event handling
    UFUNCTION(BlueprintCallable, Category = "Quest Trigger")
    void OnPlayerLocationChanged(const FVector& NewLocation);

    UFUNCTION(BlueprintCallable, Category = "Quest Trigger")
    void OnResourceCollected(const FString& ResourceType, int32 Quantity);

    UFUNCTION(BlueprintCallable, Category = "Quest Trigger")
    void OnCombatEvent(const FString& EventType, AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Quest Trigger")
    void OnDiscoveryEvent(const FString& DiscoveryType, const FVector& Location);

    // Utility functions
    UFUNCTION(BlueprintCallable, Category = "Quest Trigger")
    TArray<FString> GetActiveQuestTriggers();

    UFUNCTION(BlueprintCallable, Category = "Quest Trigger")
    int32 GetTriggerCount();

    UFUNCTION(BlueprintCallable, Category = "Quest Trigger")
    void ClearAllTriggers();

    // Debug functions
    UFUNCTION(BlueprintCallable, Category = "Quest Trigger", CallInEditor = true)
    void DebugPrintActiveTriggers();

    UFUNCTION(BlueprintCallable, Category = "Quest Trigger", CallInEditor = true)
    void TestLocationTrigger(const FVector& TestLocation, float TestRadius);

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest Trigger")
    TMap<FString, FQuest_TriggerCondition> ActiveTriggers;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest Trigger")
    TMap<FString, float> TriggerTimers;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest Trigger")
    FVector LastPlayerLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Trigger")
    float LocationCheckInterval = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Trigger")
    bool bEnableDebugOutput = false;

private:
    float LocationCheckTimer = 0.0f;
    
    void UpdateTimerTriggers(float DeltaTime);
    void CheckLocationTriggers();
    AActor* GetPlayerActor();
    TArray<AActor*> GetActorsWithTag(const FString& Tag);
};