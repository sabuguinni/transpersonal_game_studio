#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "NPCBehaviorTreeComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPC_BehaviorTreeComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_BehaviorTreeComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Behavior Tree Assets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Behavior")
    class UBehaviorTree* IdleBehaviorTree;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Behavior")
    class UBehaviorTree* PatrolBehaviorTree;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Behavior")
    class UBehaviorTree* WorkBehaviorTree;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Behavior")
    class UBehaviorTree* SocializeBehaviorTree;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Behavior")
    class UBehaviorTree* FleeBehaviorTree;

    // Blackboard Assets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Behavior")
    class UBlackboardData* NPCBlackboard;

    // Current Behavior State
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Behavior")
    ENPC_BehaviorState CurrentBehaviorState;

    // AI Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Components")
    class UBehaviorTreeComponent* BehaviorTreeComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Components")
    class UBlackboardComponent* BlackboardComponent;

    // Behavior Parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Behavior")
    float PatrolRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Behavior")
    float WorkDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Behavior")
    float SocialDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Behavior")
    float FleeDistance;

    // Memory System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Memory")
    TArray<FVector> ThreatLocations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Memory")
    TArray<AActor*> KnownThreats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Memory")
    float ThreatMemoryDuration;

public:
    // Behavior Control Functions
    UFUNCTION(BlueprintCallable, Category = "AI Behavior")
    void SetBehaviorState(ENPC_BehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "AI Behavior")
    void StartBehaviorTree(UBehaviorTree* TreeToStart);

    UFUNCTION(BlueprintCallable, Category = "AI Behavior")
    void StopBehaviorTree();

    UFUNCTION(BlueprintCallable, Category = "AI Behavior")
    void UpdateBlackboardValue(const FString& KeyName, const FVector& Value);

    UFUNCTION(BlueprintCallable, Category = "AI Behavior")
    void UpdateBlackboardActor(const FString& KeyName, AActor* Actor);

    // Memory Functions
    UFUNCTION(BlueprintCallable, Category = "AI Memory")
    void AddThreatLocation(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "AI Memory")
    void AddKnownThreat(AActor* Threat);

    UFUNCTION(BlueprintCallable, Category = "AI Memory")
    void ClearOldThreats();

    UFUNCTION(BlueprintCallable, Category = "AI Memory")
    bool IsLocationThreatening(const FVector& Location, float Radius = 500.0f);

    // Utility Functions
    UFUNCTION(BlueprintCallable, Category = "AI Utility")
    FVector GetRandomPatrolPoint();

    UFUNCTION(BlueprintCallable, Category = "AI Utility")
    AActor* FindNearestNPC(float MaxDistance = 1000.0f);

    UFUNCTION(BlueprintCallable, Category = "AI Utility")
    bool IsPlayerNearby(float DetectionRadius = 800.0f);

private:
    void InitializeBehaviorTree();
    void UpdateBehaviorBasedOnTime();
    void HandleThreatDetection();

    float LastThreatCheck;
    float ThreatCheckInterval;
};