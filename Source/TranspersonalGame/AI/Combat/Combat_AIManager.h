#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/TriggerBox.h"
#include "Components/SphereComponent.h"
#include "../../SharedTypes.h"
#include "Combat_AIManager.generated.h"

UENUM(BlueprintType)
enum class ECombat_ThreatLevel : uint8
{
    None        UMETA(DisplayName = "No Threat"),
    Low         UMETA(DisplayName = "Low Threat"),
    Medium      UMETA(DisplayName = "Medium Threat"),
    High        UMETA(DisplayName = "High Threat"),
    Critical    UMETA(DisplayName = "Critical Threat")
};

UENUM(BlueprintType)
enum class ECombat_AIState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrol      UMETA(DisplayName = "Patrol"),
    Alert       UMETA(DisplayName = "Alert"),
    Hunting     UMETA(DisplayName = "Hunting"),
    Combat      UMETA(DisplayName = "Combat"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Wounded     UMETA(DisplayName = "Wounded")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_TacticalData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackRange = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float DetectionRadius = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float FlankingDistance = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    int32 PackSize = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    bool bCanUsePackTactics = true;

    FCombat_TacticalData()
    {
        AttackRange = 300.0f;
        DetectionRadius = 800.0f;
        FlankingDistance = 500.0f;
        PackSize = 3;
        bCanUsePackTactics = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACombat_AIManager : public AActor
{
    GENERATED_BODY()

public:
    ACombat_AIManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    USphereComponent* DetectionSphere;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    ECombat_ThreatLevel CurrentThreatLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    FCombat_TacticalData TacticalSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    TArray<AActor*> RegisteredCombatants;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    TArray<ATriggerBox*> CombatZones;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void RegisterCombatant(AActor* Combatant);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void UnregisterCombatant(AActor* Combatant);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    ECombat_ThreatLevel CalculateThreatLevel(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void InitializeCombatZones();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    TArray<FVector> GenerateFlankingPositions(FVector TargetLocation, int32 NumPositions);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void UpdateCombatStates();

private:
    float LastThreatUpdate;
    float ThreatUpdateInterval;

    void UpdateThreatAssessment();
    void ManagePackCoordination();
};