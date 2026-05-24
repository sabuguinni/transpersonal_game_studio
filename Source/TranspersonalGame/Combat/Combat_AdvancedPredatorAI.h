#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "Components/SphereComponent.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "TranspersonalGame/TranspersonalGame.h"
#include "Combat_AdvancedPredatorAI.generated.h"

// Forward declarations
class UBehaviorTreeComponent;
class UBlackboardComponent;
class UAIPerceptionComponent;

UENUM(BlueprintType)
enum class ECombat_PredatorState : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Patrol          UMETA(DisplayName = "Patrol"),
    Investigate     UMETA(DisplayName = "Investigate"),
    Stalk           UMETA(DisplayName = "Stalk"),
    Hunt            UMETA(DisplayName = "Hunt"),
    Attack          UMETA(DisplayName = "Attack"),
    Retreat         UMETA(DisplayName = "Retreat"),
    Defend          UMETA(DisplayName = "Defend")
};

UENUM(BlueprintType)
enum class ECombat_HuntingPattern : uint8
{
    CircleAndClose      UMETA(DisplayName = "Circle and Close"),
    PackFlanking        UMETA(DisplayName = "Pack Flanking"),
    AmbushFromCover     UMETA(DisplayName = "Ambush from Cover"),
    DirectAssault       UMETA(DisplayName = "Direct Assault"),
    TerritorialDefense  UMETA(DisplayName = "Territorial Defense")
};

UENUM(BlueprintType)
enum class ECombat_ThreatLevel : uint8
{
    None        UMETA(DisplayName = "No Threat"),
    Low         UMETA(DisplayName = "Low Threat"),
    Medium      UMETA(DisplayName = "Medium Threat"),
    High        UMETA(DisplayName = "High Threat"),
    Extreme     UMETA(DisplayName = "Extreme Threat")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_PredatorBehavior
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float Aggression = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float Intelligence = 0.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    bool bPackBehavior = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float TerritorySize = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    ECombat_HuntingPattern PreferredHuntingPattern = ECombat_HuntingPattern::CircleAndClose;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float FearThreshold = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float PatrolRadius = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float DetectionRange = 1200.0f;

    FCombat_PredatorBehavior()
    {
        Aggression = 0.7f;
        Intelligence = 0.6f;
        bPackBehavior = false;
        TerritorySize = 1000.0f;
        PreferredHuntingPattern = ECombat_HuntingPattern::CircleAndClose;
        FearThreshold = 0.3f;
        PatrolRadius = 800.0f;
        DetectionRange = 1200.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_EnvironmentalAwareness
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float VegetationDensity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float WaterProximity = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float ElevationAdvantage = 0.4f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float WeatherInfluence = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float OverallAwareness = 0.5f;

    FCombat_EnvironmentalAwareness()
    {
        VegetationDensity = 0.5f;
        WaterProximity = 0.3f;
        ElevationAdvantage = 0.4f;
        WeatherInfluence = 0.5f;
        OverallAwareness = 0.5f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCombat_AdvancedPredatorAI : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombat_AdvancedPredatorAI();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core AI Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Predator AI")
    FCombat_PredatorBehavior PredatorBehavior;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Predator AI")
    FCombat_EnvironmentalAwareness EnvironmentalAwareness;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Predator AI")
    ECombat_PredatorState CurrentState = ECombat_PredatorState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Predator AI")
    ECombat_ThreatLevel CurrentThreatLevel = ECombat_ThreatLevel::None;

    // Territory and Patrol
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    FVector TerritoryCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    TArray<FVector> PatrolPoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    int32 CurrentPatrolIndex = 0;

    // Target Tracking
    UPROPERTY(BlueprintReadOnly, Category = "Targeting")
    AActor* CurrentTarget = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "Targeting")
    FVector LastKnownTargetLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Targeting")
    float TimeTargetLastSeen = 0.0f;

    // Pack Behavior
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Behavior")
    TArray<UCombat_AdvancedPredatorAI*> PackMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Behavior")
    bool bIsPackLeader = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Behavior")
    float PackCommunicationRange = 800.0f;

    // AI Functions
    UFUNCTION(BlueprintCallable, Category = "Predator AI")
    void SetPredatorState(ECombat_PredatorState NewState);

    UFUNCTION(BlueprintCallable, Category = "Predator AI")
    void AssessThreatLevel(AActor* PotentialThreat);

    UFUNCTION(BlueprintCallable, Category = "Predator AI")
    void UpdateEnvironmentalAwareness();

    UFUNCTION(BlueprintCallable, Category = "Predator AI")
    void InitializeTerritory();

    UFUNCTION(BlueprintCallable, Category = "Predator AI")
    void GeneratePatrolPoints();

    UFUNCTION(BlueprintCallable, Category = "Predator AI")
    FVector GetNextPatrolPoint();

    UFUNCTION(BlueprintCallable, Category = "Predator AI")
    bool IsInTerritory(FVector Location) const;

    UFUNCTION(BlueprintCallable, Category = "Predator AI")
    void ExecuteHuntingPattern();

    UFUNCTION(BlueprintCallable, Category = "Predator AI")
    void CoordinateWithPack();

    UFUNCTION(BlueprintCallable, Category = "Predator AI")
    void HandleTargetDetection(AActor* DetectedTarget);

    UFUNCTION(BlueprintCallable, Category = "Predator AI")
    void HandleTargetLost();

    UFUNCTION(BlueprintPure, Category = "Predator AI")
    float GetDistanceToTarget() const;

    UFUNCTION(BlueprintPure, Category = "Predator AI")
    bool ShouldEngageTarget() const;

    UFUNCTION(BlueprintPure, Category = "Predator AI")
    bool ShouldRetreat() const;

private:
    // Internal state tracking
    float StateTimer = 0.0f;
    float LastEnvironmentUpdate = 0.0f;
    float LastPackCommunication = 0.0f;
    
    // AI decision making
    void UpdateAIBehavior(float DeltaTime);
    void ProcessCurrentState(float DeltaTime);
    void MakeStateTransition();
    
    // Environmental analysis
    void AnalyzeLocalEnvironment();
    float CalculateEnvironmentalAdvantage(FVector Location) const;
    
    // Pack coordination
    void SendPackSignal(const FString& SignalType);
    void ReceivePackSignal(const FString& SignalType, UCombat_AdvancedPredatorAI* Sender);
};