#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "DinosaurCombatAIController.generated.h"

UENUM(BlueprintType)
enum class ECombat_DinoAggroState : uint8
{
    Passive     UMETA(DisplayName = "Passive"),
    Alert       UMETA(DisplayName = "Alert"),
    Aggressive  UMETA(DisplayName = "Aggressive"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Retreating  UMETA(DisplayName = "Retreating"),
    Fleeing     UMETA(DisplayName = "Fleeing")
};

UENUM(BlueprintType)
enum class ECombat_DinoSpecies : uint8
{
    TyrannosaurusRex    UMETA(DisplayName = "T-Rex"),
    Velociraptor        UMETA(DisplayName = "Velociraptor"),
    Triceratops         UMETA(DisplayName = "Triceratops"),
    Brachiosaurus       UMETA(DisplayName = "Brachiosaurus"),
    Pterodactyl         UMETA(DisplayName = "Pterodactyl")
};

USTRUCT(BlueprintType)
struct FCombat_DinoSpeciesTraits
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Traits")
    float DetectionRadius = 1200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Traits")
    float AggroRadius = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Traits")
    float AttackRange = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Traits")
    float MoveSpeed = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Traits")
    float AttackDamage = 40.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Traits")
    float AttackCooldown = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Traits")
    bool bIsPack = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Traits")
    int32 PackSize = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Traits")
    float FleeHealthThreshold = 0.2f;
};

USTRUCT(BlueprintType)
struct FCombat_ThreatRecord
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Threat")
    AActor* ThreatActor = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Threat")
    float ThreatLevel = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Threat")
    FVector LastKnownLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Threat")
    float TimeLastSeen = 0.0f;
};

UCLASS(ClassGroup = (AI), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API ADinosaurCombatAIController : public AAIController
{
    GENERATED_BODY()

public:
    ADinosaurCombatAIController();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void OnPossess(APawn* InPawn) override;

    // Species configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Species")
    ECombat_DinoSpecies Species = ECombat_DinoSpecies::TyrannosaurusRex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Species")
    FCombat_DinoSpeciesTraits SpeciesTraits;

    // Current AI state
    UPROPERTY(BlueprintReadOnly, Category = "Combat|State")
    ECombat_DinoAggroState AggroState = ECombat_DinoAggroState::Passive;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|State")
    AActor* CurrentTarget = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|State")
    float CurrentHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|State")
    float MaxHealth = 100.0f;

    // Patrol waypoints (set from editor or by pack leader)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Patrol")
    TArray<AActor*> PatrolWaypoints;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Patrol")
    int32 CurrentWaypointIndex = 0;

    // Pack coordination
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Pack")
    bool bIsPackLeader = false;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Pack")
    TArray<ADinosaurCombatAIController*> PackMembers;

    // Threat tracking
    UPROPERTY(BlueprintReadOnly, Category = "Combat|Threat")
    TArray<FCombat_ThreatRecord> ThreatList;

    // Audio alert URLs (wired from TTS pipeline)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Audio")
    FString AlertAudioURL_TRex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Audio")
    FString AlertAudioURL_Raptor;

    // Core combat functions
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetAggroState(ECombat_DinoAggroState NewState);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void DetectThreats();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ExecuteAttack(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    float TakeDamage_Combat(float DamageAmount, AActor* DamageInstigator);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void BroadcastAlertToPackMembers(AActor* Threat);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void MoveToNextWaypoint();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void InitSpeciesTraits();

    UFUNCTION(BlueprintPure, Category = "Combat")
    bool IsPlayerInAggroRange() const;

    UFUNCTION(BlueprintPure, Category = "Combat")
    float GetDistanceToTarget() const;

    UFUNCTION(BlueprintPure, Category = "Combat")
    ECombat_DinoAggroState GetCurrentAggroState() const { return AggroState; }

private:
    float AttackCooldownRemaining = 0.0f;
    float StateTickInterval = 0.25f;
    float StateTickAccumulator = 0.0f;

    void TickPassive(float DeltaTime);
    void TickAlert(float DeltaTime);
    void TickAggressive(float DeltaTime);
    void TickAttacking(float DeltaTime);
    void TickRetreating(float DeltaTime);

    AActor* FindNearestThreat() const;
    void NotifyNearbyNPCsOfThreat(AActor* Threat);
};
