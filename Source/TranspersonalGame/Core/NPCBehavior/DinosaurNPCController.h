#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
// DISABLED: #include "Perception/PerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
// FIXME: Missing header - #include "NPCBehaviorSystem.h"
#include "Perception/AIPerceptionComponent.h"
#include "DinosaurNPCController.generated.h"

/**
 * AI Controller specifically designed for dinosaur NPCs
 * Handles complex behavioral patterns, social interactions, and survival instincts
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ADinosaurNPCController : public AAIController
{
    GENERATED_BODY()

public:
    ADinosaurNPCController();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime);
    virtual void OnPossess(APawn* InPawn) override;

    // Core AI Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Components")
    class UNPCBehaviorComponent* BehaviorComponent;

    // SHADOWED: UPROPERTY class UAIPerceptionComponent* PerceptionComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Components")
    class UBehaviorTreeComponent* BehaviorTreeComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Components")
    class UBlackboardComponent* BlackboardComponent;

    // Behavior Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    class UBehaviorTree* DefaultBehaviorTree;

// [UHT-FIX]     UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    class UBlackboardData* DefaultBlackboard;

    // Perception Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
    float SightRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
    float LoseSightRadius = 1600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
    float PeripheralVisionAngleDegrees = 90.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
    float HearingRange = 1200.0f;

    // Dinosaur-Specific Behavior
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Behavior")
    bool bIsHerbivore = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Behavior")
    bool bIsPackAnimal = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Behavior")
    bool bIsTerritorial = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Behavior")
    float TerritoryRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Behavior")
    float ComfortDistance = 200.0f; // Distance to maintain from threats

    // Social Behavior
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    TArray<ADinosaurNPCController*> PackMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    ADinosaurNPCController* PackLeader;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    bool bIsPackLeader = false;

    // Current State
    UPROPERTY(BlueprintReadOnly, Category = "State")
    AActor* CurrentTarget;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    FVector HomeLocation;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bIsAlerted = false;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    float LastThreatTime = 0.0f;

public:
    // Perception Events
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

    // Behavior Functions
    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void SetAlertState(bool bAlert);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void FleeFromThreat(AActor* Threat);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void InvestigateLocation(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void ReturnToHome();

    // Social Functions
    UFUNCTION(BlueprintCallable, Category = "Social")
    void JoinPack(ADinosaurNPCController* Leader);

    UFUNCTION(BlueprintCallable, Category = "Social")
    void LeavePack();

    UFUNCTION(BlueprintCallable, Category = "Social")
    void AlertPackMembers(AActor* Threat);

    UFUNCTION(BlueprintCallable, Category = "Social")
    bool IsInPack() const { return PackLeader != nullptr || bIsPackLeader; }

    // Utility Functions
    UFUNCTION(BlueprintCallable, Category = "Utility")
    float GetDistanceToHome() const;

    UFUNCTION(BlueprintCallable, Category = "Utility")
    bool IsLocationSafe(FVector Location) const;

    UFUNCTION(BlueprintCallable, Category = "Utility")
    FVector FindSafeLocation() const;

    UFUNCTION(BlueprintCallable, Category = "Utility")
    AActor* GetNearestThreat() const;

protected:
    // Internal Functions
    void InitializePerception();
    void InitializeBehaviorTree();
    void UpdateBlackboardValues();
    void ProcessPerceptionUpdate(AActor* Actor, bool bCanSee, bool bCanHear);
    bool IsThreat(AActor* Actor) const;
    void HandleThreatDetected(AActor* Threat);
    void HandleThreatLost(AActor* Threat);
    void UpdatePackBehavior();

private:
    // Cached references
    class APawn* ControlledDinosaur;
    
    // Timing
    float LastPerceptionUpdate = 0.0f;
    float PerceptionUpdateInterval = 0.2f;
    
    // Threat tracking
    TArray<AActor*> KnownThreats;
    TMap<AActor*, float> ThreatLastSeenTime;
    
    // Pack communication
    float LastPackCommunication = 0.0f;
    float PackCommunicationInterval = 1.0f;
};