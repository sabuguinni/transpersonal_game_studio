#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "../SharedTypes.h"
#include "CombatManager.generated.h"

UENUM(BlueprintType)
enum class ECombat_ThreatLevel : uint8
{
    None        UMETA(DisplayName = "None"),
    Low         UMETA(DisplayName = "Low"),
    Medium      UMETA(DisplayName = "Medium"),
    High        UMETA(DisplayName = "High"),
    Extreme     UMETA(DisplayName = "Extreme")
};

UENUM(BlueprintType)
enum class ECombat_EngagementType : uint8
{
    Passive     UMETA(DisplayName = "Passive"),
    Defensive   UMETA(DisplayName = "Defensive"),
    Aggressive  UMETA(DisplayName = "Aggressive"),
    Territorial UMETA(DisplayName = "Territorial"),
    Pack        UMETA(DisplayName = "Pack"),
    Ambush      UMETA(DisplayName = "Ambush")
};

USTRUCT(BlueprintType)
struct FCombat_ThreatData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat")
    AActor* ThreatActor = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat")
    ECombat_ThreatLevel ThreatLevel = ECombat_ThreatLevel::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat")
    ECombat_EngagementType EngagementType = ECombat_EngagementType::Passive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat")
    float Distance = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat")
    float LastSeenTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat")
    FVector LastKnownLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat")
    bool bIsHostile = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat")
    bool bInCombat = false;

    FCombat_ThreatData()
    {
        ThreatActor = nullptr;
        ThreatLevel = ECombat_ThreatLevel::None;
        EngagementType = ECombat_EngagementType::Passive;
        Distance = 0.0f;
        LastSeenTime = 0.0f;
        LastKnownLocation = FVector::ZeroVector;
        bIsHostile = false;
        bInCombat = false;
    }
};

USTRUCT(BlueprintType)
struct FCombat_EncounterData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter")
    TArray<AActor*> Participants;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter")
    AActor* PrimaryTarget = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter")
    FVector EncounterLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter")
    float StartTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter")
    float Duration = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter")
    bool bIsActive = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter")
    ECombat_ThreatLevel MaxThreatLevel = ECombat_ThreatLevel::None;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCombatStateChanged, AActor*, Actor, bool, bInCombat);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnThreatDetected, AActor*, ThreatActor, ECombat_ThreatLevel, ThreatLevel, float, Distance);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDamageDealt, AActor*, Attacker, AActor*, Victim);

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UCombatManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UCombatManager();

    // Subsystem Interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Combat Management
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void RegisterCombatActor(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void UnregisterCombatActor(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void StartCombatEncounter(AActor* Instigator, AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void EndCombatEncounter(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool IsActorInCombat(AActor* Actor) const;

    // Threat Assessment
    UFUNCTION(BlueprintCallable, Category = "Combat")
    ECombat_ThreatLevel AssessThreatLevel(AActor* ThreatActor, AActor* Observer) const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    TArray<FCombat_ThreatData> GetNearbyThreats(AActor* Observer, float MaxDistance = 5000.0f) const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    AActor* GetNearestThreat(AActor* Observer, float MaxDistance = 5000.0f) const;

    // Damage System
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void DealDamage(AActor* Attacker, AActor* Victim, float Damage, const FVector& HitLocation);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool CanActorAttack(AActor* Attacker, AActor* Target) const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    float CalculateDamageMultiplier(AActor* Attacker, AActor* Victim) const;

    // AI Coordination
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void CoordinatePackAttack(const TArray<AActor*>& PackMembers, AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void RequestBackup(AActor* Caller, const FVector& Location, ECombat_ThreatLevel ThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    TArray<AActor*> GetAvailableAllies(AActor* Caller, float MaxDistance = 3000.0f) const;

    // Territory Management
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void DefendTerritory(AActor* Defender, const FVector& TerritoryCenter, float TerritoryRadius);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool IsLocationInTerritory(const FVector& Location, AActor* TerritoryOwner) const;

    // Combat Events
    UPROPERTY(BlueprintAssignable, Category = "Combat Events")
    FOnCombatStateChanged OnCombatStateChanged;

    UPROPERTY(BlueprintAssignable, Category = "Combat Events")
    FOnThreatDetected OnThreatDetected;

    UPROPERTY(BlueprintAssignable, Category = "Combat Events")
    FOnDamageDealt OnDamageDealt;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Settings")
    float CombatTimeout = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Settings")
    float ThreatUpdateInterval = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Settings")
    float MaxCombatDistance = 10000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Settings")
    bool bEnablePackCoordination = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Settings")
    bool bEnableTerritorialBehavior = true;

protected:
    // Internal Data
    UPROPERTY()
    TArray<AActor*> RegisteredCombatActors;

    UPROPERTY()
    TArray<FCombat_EncounterData> ActiveEncounters;

    UPROPERTY()
    TMap<AActor*, FCombat_ThreatData> ThreatDatabase;

    // Update Functions
    void UpdateCombatSystem(float DeltaTime);
    void UpdateThreatAssessment();
    void UpdateActiveEncounters(float DeltaTime);
    void CleanupInvalidActors();

    // Helper Functions
    bool IsValidCombatActor(AActor* Actor) const;
    float CalculateDistanceBetweenActors(AActor* ActorA, AActor* ActorB) const;
    bool HasLineOfSight(AActor* Observer, AActor* Target) const;
    ECombat_EngagementType DetermineEngagementType(AActor* Actor) const;

private:
    FTimerHandle CombatUpdateTimer;
    float LastThreatUpdateTime = 0.0f;
};