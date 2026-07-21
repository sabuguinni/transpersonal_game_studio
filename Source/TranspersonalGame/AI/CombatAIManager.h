#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/TriggerVolume.h"
#include "Components/StaticMeshComponent.h"
#include "SharedTypes.h"
#include "CombatAIManager.generated.h"

UENUM(BlueprintType)
enum class ECombat_ThreatLevel : uint8
{
    Passive     UMETA(DisplayName = "Passive"),
    Cautious    UMETA(DisplayName = "Cautious"), 
    Aggressive  UMETA(DisplayName = "Aggressive"),
    Enraged     UMETA(DisplayName = "Enraged"),
    Hunting     UMETA(DisplayName = "Hunting")
};

UENUM(BlueprintType)
enum class ECombat_Formation : uint8
{
    None        UMETA(DisplayName = "None"),
    Circle      UMETA(DisplayName = "Circle"),
    Line        UMETA(DisplayName = "Line"),
    Triangle    UMETA(DisplayName = "Triangle"),
    Ambush      UMETA(DisplayName = "Ambush")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_TacticalData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    ECombat_ThreatLevel ThreatLevel = ECombat_ThreatLevel::Passive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    ECombat_Formation Formation = ECombat_Formation::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AggressionRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float FleeRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    int32 PackSize = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackDamage = 50.0f;

    FCombat_TacticalData()
    {
        ThreatLevel = ECombat_ThreatLevel::Passive;
        Formation = ECombat_Formation::None;
        AggressionRadius = 1000.0f;
        FleeRadius = 2000.0f;
        PackSize = 1;
        AttackDamage = 50.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACombatAIManager : public AActor
{
    GENERATED_BODY()

public:
    ACombatAIManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Combat zone management
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void RegisterCombatZone(ATriggerVolume* Zone, ECombat_ThreatLevel ThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void UnregisterCombatZone(ATriggerVolume* Zone);

    // Pack behavior
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void FormPack(const TArray<AActor*>& PackMembers, ECombat_Formation Formation);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void DisbandPack(const TArray<AActor*>& PackMembers);

    // Threat assessment
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    ECombat_ThreatLevel AssessThreat(AActor* Target, AActor* Observer);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool ShouldEngageTarget(AActor* Predator, AActor* Target);

    // Combat coordination
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    FVector GetOptimalAttackPosition(AActor* Attacker, AActor* Target, ECombat_Formation Formation);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void CoordinatePackAttack(const TArray<AActor*>& PackMembers, AActor* Target);

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI")
    TMap<ATriggerVolume*, ECombat_ThreatLevel> CombatZones;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI")
    TMap<AActor*, FCombat_TacticalData> CombatData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float CombatUpdateInterval = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float MaxCombatDistance = 5000.0f;

private:
    float LastCombatUpdate = 0.0f;
    
    void UpdateCombatStates();
    void ProcessCombatZones();
    void HandlePackBehavior();
    FVector CalculateFlankPosition(AActor* Target, AActor* Flanker, float Distance);
};