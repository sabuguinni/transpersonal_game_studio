#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "SharedTypes.h"
#include "DinosaurCombatAI.generated.h"

UENUM(BlueprintType)
enum class ECombat_DinosaurCombatState : uint8
{
    Idle UMETA(DisplayName = "Idle"),
    Hunting UMETA(DisplayName = "Hunting"),
    Stalking UMETA(DisplayName = "Stalking"),
    Attacking UMETA(DisplayName = "Attacking"),
    Flanking UMETA(DisplayName = "Flanking"),
    Retreating UMETA(DisplayName = "Retreating"),
    Territorial UMETA(DisplayName = "Territorial"),
    PackCoordination UMETA(DisplayName = "Pack Coordination")
};

UENUM(BlueprintType)
enum class ECombat_DinosaurSpecies : uint8
{
    Velociraptor UMETA(DisplayName = "Velociraptor"),
    TRex UMETA(DisplayName = "T-Rex"),
    Triceratops UMETA(DisplayName = "Triceratops"),
    Brachiosaurus UMETA(DisplayName = "Brachiosaurus"),
    Ankylosaurus UMETA(DisplayName = "Ankylosaurus"),
    Parasaurolophus UMETA(DisplayName = "Parasaurolophus")
};

USTRUCT(BlueprintType)
struct FCombat_ThreatAssessment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat")
    AActor* ThreatActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat")
    float ThreatLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat")
    float Distance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat")
    FVector LastKnownLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat")
    float LastSeenTime;

    FCombat_ThreatAssessment()
    {
        ThreatActor = nullptr;
        ThreatLevel = 0.0f;
        Distance = 0.0f;
        LastKnownLocation = FVector::ZeroVector;
        LastSeenTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FCombat_PackCoordination
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    TArray<AActor*> PackMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    AActor* PackLeader;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    AActor* PrimaryTarget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    FVector FlankingPositionLeft;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    FVector FlankingPositionRight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    bool bCoordinatedAttack;

    FCombat_PackCoordination()
    {
        PackLeader = nullptr;
        PrimaryTarget = nullptr;
        FlankingPositionLeft = FVector::ZeroVector;
        FlankingPositionRight = FVector::ZeroVector;
        bCoordinatedAttack = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ADinosaurCombatAI : public AActor
{
    GENERATED_BODY()

public:
    ADinosaurCombatAI();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Core Combat Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    ECombat_DinosaurCombatState CurrentCombatState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    ECombat_DinosaurSpecies DinosaurSpecies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float AggressionLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float IntelligenceLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float TerritorialRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float DetectionRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float AttackRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float FlankingDistance;

    // Threat Assessment
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Assessment")
    TArray<FCombat_ThreatAssessment> KnownThreats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Assessment")
    AActor* PrimaryTarget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Assessment")
    float ThreatMemoryDuration;

    // Pack Behavior
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Behavior")
    FCombat_PackCoordination PackData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Behavior")
    bool bIsPackLeader;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Behavior")
    bool bUsesPackTactics;

    // Combat Timers
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Timing")
    float LastAttackTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Timing")
    float AttackCooldown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Timing")
    float StateChangeTime;

    // Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* DetectionSphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* AttackSphere;

    // Combat AI Functions
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void UpdateCombatState(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void ScanForThreats();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void AssessThreat(AActor* PotentialThreat);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void UpdateThreatMemory(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SelectPrimaryTarget();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void ExecuteCombatBehavior(float DeltaTime);

    // Pack Coordination Functions
    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void InitializePackBehavior();

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void CoordinatePackAttack();

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void AssignFlankingPositions();

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void CommunicateWithPack(const FString& Message);

    // Tactical Functions
    UFUNCTION(BlueprintCallable, Category = "Combat Tactics")
    void ExecuteHuntingBehavior();

    UFUNCTION(BlueprintCallable, Category = "Combat Tactics")
    void ExecuteStalkingBehavior();

    UFUNCTION(BlueprintCallable, Category = "Combat Tactics")
    void ExecuteAttackBehavior();

    UFUNCTION(BlueprintCallable, Category = "Combat Tactics")
    void ExecuteFlankingBehavior();

    UFUNCTION(BlueprintCallable, Category = "Combat Tactics")
    void ExecuteRetreatBehavior();

    UFUNCTION(BlueprintCallable, Category = "Combat Tactics")
    void ExecuteTerritorialBehavior();

    // Utility Functions
    UFUNCTION(BlueprintCallable, Category = "Combat Utilities")
    float CalculateThreatLevel(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat Utilities")
    FVector CalculateFlankingPosition(AActor* Target, bool bLeftFlank);

    UFUNCTION(BlueprintCallable, Category = "Combat Utilities")
    bool IsInAttackRange(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat Utilities")
    bool IsInTerritorialRange(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat Utilities")
    void LogCombatEvent(const FString& EventDescription);

protected:
    // Internal state tracking
    float StateTimer;
    float ThreatScanTimer;
    float PackCoordinationTimer;
    FVector LastKnownTargetLocation;
    bool bHasValidTarget;

    // Species-specific behavior modifiers
    void ApplySpeciesBehaviorModifiers();
    void UpdateSpeciesSpecificBehavior(float DeltaTime);
};