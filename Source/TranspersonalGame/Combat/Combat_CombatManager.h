#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "SharedTypes.h"
#include "Combat_CombatManager.generated.h"

class ATranspersonalCharacter;
class APawn;

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_ThreatData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    AActor* ThreatActor;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    float ThreatLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    float Distance;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    float LastSeenTime;

    FCombat_ThreatData()
    {
        ThreatActor = nullptr;
        ThreatLevel = 0.0f;
        Distance = 0.0f;
        LastSeenTime = 0.0f;
    }
};

UENUM(BlueprintType)
enum class ECombat_CombatState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Searching   UMETA(DisplayName = "Searching"),
    Stalking    UMETA(DisplayName = "Stalking"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Defending   UMETA(DisplayName = "Defending")
};

UCLASS(ClassGroup=(Combat), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCombat_CombatManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombat_CombatManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Combat state management
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetCombatState(ECombat_CombatState NewState);

    UFUNCTION(BlueprintPure, Category = "Combat")
    ECombat_CombatState GetCombatState() const { return CurrentCombatState; }

    // Threat detection and management
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ScanForThreats();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void AddThreat(AActor* ThreatActor, float ThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void RemoveThreat(AActor* ThreatActor);

    UFUNCTION(BlueprintPure, Category = "Combat")
    AActor* GetPrimaryThreat() const;

    UFUNCTION(BlueprintPure, Category = "Combat")
    float GetThreatLevel(AActor* Actor) const;

    // Combat actions
    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool CanAttackTarget(AActor* Target) const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void InitiateAttack(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ExecuteAttack();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void DefendPosition();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void FleeFromThreat();

    // Pack coordination (for pack hunters like raptors)
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void CoordinatePackAttack(const TArray<APawn*>& PackMembers, AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetPackLeader(APawn* Leader);

    UFUNCTION(BlueprintPure, Category = "Combat")
    bool IsPackLeader() const { return bIsPackLeader; }

protected:
    // Combat state
    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    ECombat_CombatState CurrentCombatState;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    TArray<FCombat_ThreatData> KnownThreats;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    AActor* CurrentTarget;

    // Combat parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    float DetectionRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    float AttackRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    float AttackDamage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    float AttackCooldown;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    float LastAttackTime;

    // Pack behavior
    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    bool bIsPackLeader;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    APawn* PackLeader;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    TArray<APawn*> PackMembers;

    // Internal functions
    void UpdateThreatTracking(float DeltaTime);
    void ProcessCombatState(float DeltaTime);
    bool IsValidThreat(AActor* Actor) const;
    float CalculateThreatLevel(AActor* Actor) const;
};