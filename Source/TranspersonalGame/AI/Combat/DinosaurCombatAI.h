#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "AIController.h"
#include "SharedTypes.h"
#include "DinosaurCombatAI.generated.h"

// ============================================================
// Enums — Combat & Enemy AI Agent #12
// ============================================================

UENUM(BlueprintType)
enum class ECombat_DinoThreatLevel : uint8
{
    None        UMETA(DisplayName = "No Threat"),
    Curious     UMETA(DisplayName = "Curious"),
    Stalking    UMETA(DisplayName = "Stalking"),
    Aggressive  UMETA(DisplayName = "Aggressive"),
    Charging    UMETA(DisplayName = "Charging"),
    Retreating  UMETA(DisplayName = "Retreating")
};

UENUM(BlueprintType)
enum class ECombat_DinoSpecies : uint8
{
    Velociraptor    UMETA(DisplayName = "Velociraptor"),
    TRex            UMETA(DisplayName = "T-Rex"),
    Triceratops     UMETA(DisplayName = "Triceratops"),
    Brachiosaurus   UMETA(DisplayName = "Brachiosaurus"),
    Pterodactyl     UMETA(DisplayName = "Pterodactyl"),
    Spinosaurus     UMETA(DisplayName = "Spinosaurus")
};

UENUM(BlueprintType)
enum class ECombat_AttackType : uint8
{
    Bite        UMETA(DisplayName = "Bite"),
    Claw        UMETA(DisplayName = "Claw Swipe"),
    Charge      UMETA(DisplayName = "Body Charge"),
    Tail        UMETA(DisplayName = "Tail Sweep"),
    Stomp       UMETA(DisplayName = "Stomp"),
    Pounce      UMETA(DisplayName = "Pounce")
};

UENUM(BlueprintType)
enum class ECombat_FlankRole : uint8
{
    None        UMETA(DisplayName = "No Role"),
    Decoy       UMETA(DisplayName = "Decoy — draws attention"),
    Flanker     UMETA(DisplayName = "Flanker — attacks from side"),
    Ambusher    UMETA(DisplayName = "Ambusher — waits in cover"),
    Chaser      UMETA(DisplayName = "Chaser — pursues fleeing prey")
};

// ============================================================
// Structs
// ============================================================

USTRUCT(BlueprintType)
struct FCombat_DinoStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float MaxHealth = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float CurrentHealth = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float AttackDamage = 75.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float AttackRange = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float DetectionRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float ChaseSpeed = 700.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float PatrolSpeed = 250.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float StaminaMax = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float StaminaCurrent = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float AttackCooldown = 1.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    bool bIsPack = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    int32 PackSize = 1;
};

USTRUCT(BlueprintType)
struct FCombat_FlankingData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Flanking")
    ECombat_FlankRole AssignedRole = ECombat_FlankRole::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Flanking")
    FVector FlankTargetPosition = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Flanking")
    float FlankAngleDegrees = 90.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Flanking")
    bool bFlankPositionReached = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Flanking")
    float CoordinationDelay = 0.0f;
};

USTRUCT(BlueprintType)
struct FCombat_ThreatMemory
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Memory")
    AActor* ThreatActor = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Memory")
    FVector LastKnownLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Memory")
    float ThreatScore = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Memory")
    float TimeSinceLastSeen = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Memory")
    bool bIsCurrentlyVisible = false;
};

// ============================================================
// ADinosaurCombatAI — Main Combat AI Actor
// ============================================================

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API ADinosaurCombatAI : public AActor
{
    GENERATED_BODY()

public:
    ADinosaurCombatAI();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    // ---- Species & Identity ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Identity")
    ECombat_DinoSpecies Species = ECombat_DinoSpecies::Velociraptor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Identity")
    FName DinoID;

    // ---- Combat State ----
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|State")
    ECombat_DinoThreatLevel ThreatLevel = ECombat_DinoThreatLevel::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    FCombat_DinoStats DinoStats;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|State")
    FCombat_FlankingData FlankingData;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Memory")
    FCombat_ThreatMemory PrimaryThreat;

    // ---- Pack Coordination ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Pack")
    TArray<ADinosaurCombatAI*> PackMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Pack")
    bool bIsPackLeader = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Pack")
    float PackCoordinationRadius = 800.0f;

    // ---- Behavior Tree ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|AI")
    UBehaviorTree* CombatBehaviorTree;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|AI")
    UBlackboardData* CombatBlackboard;

    // ---- Components ----
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Components")
    UAIPerceptionComponent* PerceptionComponent;

    // ---- Combat Functions ----
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void DetectThreats();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ExecuteAttack(AActor* Target, ECombat_AttackType AttackType);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void AssignFlankRoles();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void CoordinatePackAttack(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    float CalculateThreatScore(AActor* PotentialThreat) const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void UpdateThreatLevel();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool IsTargetInAttackRange(AActor* Target) const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ApplyDamageToTarget(AActor* Target, float Damage);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void OnTakeDamage(float DamageAmount, AActor* DamageSource);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void Retreat();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    FVector CalculateFlankPosition(AActor* Target, float AngleDegrees) const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void BroadcastPackAlert(AActor* DetectedThreat);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void OnPackAlertReceived(AActor* Threat, FVector ThreatLocation);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool ShouldRetreat() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    float GetHealthPercent() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void RegenerateStamina(float DeltaTime);

private:
    float AttackCooldownTimer = 0.0f;
    float ThreatUpdateTimer = 0.0f;
    float StaminaRegenRate = 15.0f;

    void TickCombatState(float DeltaTime);
    void TickPackCoordination(float DeltaTime);
};
