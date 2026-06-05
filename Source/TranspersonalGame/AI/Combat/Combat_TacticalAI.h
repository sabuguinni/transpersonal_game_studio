#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISightConfig.h"
#include "Perception/AIHearingConfig.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "SharedTypes.h"
#include "Combat_TacticalAI.generated.h"

UENUM(BlueprintType)
enum class ECombat_TacticalState : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Patrol          UMETA(DisplayName = "Patrol"),
    Hunt            UMETA(DisplayName = "Hunt"),
    Attack          UMETA(DisplayName = "Attack"),
    Defend          UMETA(DisplayName = "Defend"),
    Flee            UMETA(DisplayName = "Flee"),
    PackCoordinate  UMETA(DisplayName = "Pack Coordinate"),
    Territorial     UMETA(DisplayName = "Territorial")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_TacticalData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical")
    float AggressionLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical")
    float TerritorialRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical")
    float PackCoordinationRange = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical")
    float FleeHealthThreshold = 0.25f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical")
    bool bIsPackLeader = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical")
    bool bCanCallForHelp = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical")
    TArray<AActor*> PackMembers;

    FCombat_TacticalData()
    {
        AggressionLevel = 0.5f;
        TerritorialRadius = 1000.0f;
        PackCoordinationRange = 500.0f;
        FleeHealthThreshold = 0.25f;
        bIsPackLeader = false;
        bCanCallForHelp = true;
    }
};

UCLASS(Blueprintable, BlueprintType)
class TRANSPERSONALGAME_API ACombat_TacticalAI : public AAIController
{
    GENERATED_BODY()

public:
    ACombat_TacticalAI();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Core AI Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UBehaviorTreeComponent* BehaviorTreeComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UBlackboardComponent* BlackboardComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UAIPerceptionComponent* AIPerceptionComponent;

    // Tactical Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Tactical")
    FCombat_TacticalData TacticalData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Tactical")
    ECombat_TacticalState CurrentTacticalState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Tactical")
    class UBehaviorTree* CombatBehaviorTree;

    // Perception Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Perception")
    float SightRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Perception")
    float HearingRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Perception")
    float SightAngle = 90.0f;

public:
    // Tactical AI Methods
    UFUNCTION(BlueprintCallable, Category = "Combat Tactical")
    void SetTacticalState(ECombat_TacticalState NewState);

    UFUNCTION(BlueprintCallable, Category = "Combat Tactical")
    ECombat_TacticalState GetTacticalState() const { return CurrentTacticalState; }

    UFUNCTION(BlueprintCallable, Category = "Combat Tactical")
    void InitializePackBehavior(const TArray<AActor*>& PackMembers, bool bAsLeader = false);

    UFUNCTION(BlueprintCallable, Category = "Combat Tactical")
    void CoordinatePackAttack(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat Tactical")
    void CallForPackHelp(AActor* Threat);

    UFUNCTION(BlueprintCallable, Category = "Combat Tactical")
    bool IsInTerritorialRange(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Combat Tactical")
    void DefendTerritory(AActor* Intruder);

    UFUNCTION(BlueprintCallable, Category = "Combat Tactical")
    float CalculateThreatLevel(AActor* Target) const;

    UFUNCTION(BlueprintCallable, Category = "Combat Tactical")
    FVector GetOptimalAttackPosition(AActor* Target) const;

    UFUNCTION(BlueprintCallable, Category = "Combat Tactical")
    bool ShouldFlee() const;

protected:
    // Internal tactical logic
    void UpdateTacticalBehavior(float DeltaTime);
    void ProcessPerceptionUpdates();
    void HandlePackCoordination();
    void EvaluateThreatResponse();

    // Perception callbacks
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

private:
    // Internal state tracking
    float LastTacticalUpdate = 0.0f;
    float TacticalUpdateInterval = 0.5f;
    
    TArray<AActor*> PerceivedThreats;
    TArray<AActor*> PerceivedAllies;
    
    AActor* CurrentTarget = nullptr;
    FVector TerritorialCenter;
    
    bool bInCombat = false;
    float CombatStartTime = 0.0f;
};