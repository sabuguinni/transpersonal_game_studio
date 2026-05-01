#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "../SharedTypes.h"
#include "TribalNPCController.generated.h"

class UBehaviorTree;
class UBlackboardData;

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ATribalNPCController : public AAIController
{
    GENERATED_BODY()

public:
    ATribalNPCController();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // AI Behavior
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
    UBehaviorTree* BehaviorTree;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
    UBlackboardData* BlackboardAsset;

    // AI Perception
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    UAIPerceptionComponent* AIPerceptionComponent;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
    UAISenseConfig_Sight* SightConfig;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
    UAISenseConfig_Hearing* HearingConfig;

    // Tribal Behavior Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Behavior")
    ENPC_TribalRole TribalRole;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Behavior")
    float FearLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Behavior")
    float CourageLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Behavior")
    float SocialBondStrength;

    // Survival Instincts
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float HungerLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float ThirstLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float EnergyLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float HealthLevel;

    // Patrol and Territory
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    TArray<FVector> PatrolPoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    float TerritoryRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    FVector HomeLocation;

    // Threat Detection
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Detection")
    float DinosaurDetectionRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Detection")
    float PlayerDetectionRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Detection")
    float ThreatResponseTime;

public:
    // AI Behavior Functions
    UFUNCTION(BlueprintCallable, Category = "AI")
    void StartBehaviorTree();

    UFUNCTION(BlueprintCallable, Category = "AI")
    void StopBehaviorTree();

    UFUNCTION(BlueprintCallable, Category = "AI")
    void SetTribalRole(ENPC_TribalRole NewRole);

    // Survival Functions
    UFUNCTION(BlueprintCallable, Category = "Survival")
    void UpdateSurvivalNeeds(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    bool IsInDanger() const;

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void ReactToThreat(AActor* ThreatActor);

    // Social Functions
    UFUNCTION(BlueprintCallable, Category = "Social")
    void CallForHelp();

    UFUNCTION(BlueprintCallable, Category = "Social")
    void RespondToDistressCall(ATribalNPCController* CallingNPC);

    // Patrol Functions
    UFUNCTION(BlueprintCallable, Category = "Patrol")
    void SetPatrolPoints(const TArray<FVector>& NewPatrolPoints);

    UFUNCTION(BlueprintCallable, Category = "Patrol")
    FVector GetNextPatrolPoint();

    UFUNCTION(BlueprintCallable, Category = "Patrol")
    void ReturnToHome();

protected:
    // Perception Events
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    // Internal State
    int32 CurrentPatrolIndex;
    float LastThreatTime;
    AActor* CurrentThreat;
    bool bIsAlarmed;
    bool bIsReturningHome;

private:
    void InitializeAIPerception();
    void UpdateBlackboardValues();
    void ProcessThreatDetection();
    void HandleRoleSpecificBehavior();
};