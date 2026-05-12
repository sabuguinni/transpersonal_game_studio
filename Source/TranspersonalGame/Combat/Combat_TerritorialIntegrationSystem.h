#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "SharedTypes.h"
#include "Combat_TerritorialIntegrationSystem.generated.h"

// Forward declarations
class UCombat_TacticalAIController;
class UNPC_TerritorialBehaviorSystem;

UENUM(BlueprintType)
enum class ECombat_TerritorialResponse : uint8
{
    Ignore          UMETA(DisplayName = "Ignore"),
    Investigate     UMETA(DisplayName = "Investigate"),
    Warning         UMETA(DisplayName = "Warning Display"),
    Aggressive      UMETA(DisplayName = "Aggressive Pursuit"),
    Coordinated     UMETA(DisplayName = "Coordinated Attack"),
    Defensive       UMETA(DisplayName = "Defensive Formation")
};

UENUM(BlueprintType)
enum class ECombat_EncounterType : uint8
{
    Ambush          UMETA(DisplayName = "Ambush"),
    Coordinated     UMETA(DisplayName = "Coordinated Hunt"),
    Warning         UMETA(DisplayName = "Territorial Warning"),
    Resource        UMETA(DisplayName = "Resource Competition"),
    Territorial     UMETA(DisplayName = "Territory Defense"),
    Pack            UMETA(DisplayName = "Pack Coordination")
};

USTRUCT(BlueprintType)
struct FCombat_TerritorialZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    FVector Center;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    float Radius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    ETerritoryType TerritoryType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    ECombat_TerritorialResponse DefaultResponse;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AggressionMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    TArray<FVector> TacticalPositions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    bool bRequiresPackCoordination;

    FCombat_TerritorialZone()
    {
        Center = FVector::ZeroVector;
        Radius = 1000.0f;
        TerritoryType = ETerritoryType::Neutral;
        DefaultResponse = ECombat_TerritorialResponse::Investigate;
        AggressionMultiplier = 1.0f;
        bRequiresPackCoordination = false;
    }
};

USTRUCT(BlueprintType)
struct FCombat_EncounterTrigger
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter")
    FVector Position;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter")
    ECombat_EncounterType EncounterType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter")
    float TriggerRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter")
    TArray<AActor*> InvolvedActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter")
    bool bIsActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter")
    float CooldownTime;

    FCombat_EncounterTrigger()
    {
        Position = FVector::ZeroVector;
        EncounterType = ECombat_EncounterType::Warning;
        TriggerRadius = 500.0f;
        bIsActive = true;
        CooldownTime = 30.0f;
    }
};

UCLASS(ClassGroup=(Combat), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCombat_TerritorialIntegrationSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombat_TerritorialIntegrationSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Territory Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    TArray<FCombat_TerritorialZone> CombatZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounters")
    TArray<FCombat_EncounterTrigger> EncounterTriggers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float ThreatDetectionRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float CombatEngagementRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    int32 MaxPackSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    float PackCoordinationRadius;

    // Combat Integration Functions
    UFUNCTION(BlueprintCallable, Category = "Combat")
    ECombat_TerritorialResponse DetermineTerritorialResponse(AActor* Intruder, const FVector& IntruderLocation);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool InitiateCombatEncounter(ECombat_EncounterType EncounterType, AActor* Target, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Territory")
    FCombat_TerritorialZone* FindTerritorialZone(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Pack")
    TArray<AActor*> GetPackMembersInRange(const FVector& Location, float Range);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool SetupTacticalPositions(const FCombat_TerritorialZone& Zone, TArray<AActor*> PackMembers);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void UpdateCombatState(AActor* CombatActor, ECombatState NewState);

    UFUNCTION(BlueprintCallable, Category = "Encounters")
    void ProcessEncounterTriggers(AActor* Player, const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    float CalculateAggressionLevel(AActor* Defender, AActor* Intruder, const FVector& Location);

    // Pack Coordination
    UFUNCTION(BlueprintCallable, Category = "Pack")
    void CoordinatePackAttack(TArray<AActor*> PackMembers, AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Pack")
    void AssignPackRoles(TArray<AActor*> PackMembers, const FVector& TargetLocation);

    UFUNCTION(BlueprintCallable, Category = "Territory")
    void DefendTerritory(AActor* Defender, AActor* Intruder, const FCombat_TerritorialZone& Territory);

    // Integration with NPC Territorial System
    UFUNCTION(BlueprintCallable, Category = "Integration")
    void IntegrateWithNPCTerritorialSystem();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void SynchronizeTerritorialBehaviors(UNPC_TerritorialBehaviorSystem* NPCSystem);

private:
    // Internal state tracking
    UPROPERTY()
    TMap<AActor*, ECombat_TerritorialResponse> ActiveResponses;

    UPROPERTY()
    TMap<AActor*, float> LastEncounterTimes;

    UPROPERTY()
    TArray<AActor*> ActiveCombatActors;

    // Helper functions
    bool IsInTerritorialZone(const FVector& Location, const FCombat_TerritorialZone& Zone) const;
    float GetDistanceToZoneCenter(const FVector& Location, const FCombat_TerritorialZone& Zone) const;
    void UpdateEncounterCooldowns(float DeltaTime);
    void ProcessTacticalMovement(TArray<AActor*> PackMembers, const FVector& TargetLocation);
};