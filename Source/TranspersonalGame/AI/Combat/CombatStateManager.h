#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "SharedTypes.h"
#include "CombatStateManager.generated.h"

UENUM(BlueprintType)
enum class ECombat_CombatState : uint8
{
    Idle UMETA(DisplayName = "Idle"),
    Patrol UMETA(DisplayName = "Patrol"),
    Alert UMETA(DisplayName = "Alert"),
    Hunting UMETA(DisplayName = "Hunting"),
    Attacking UMETA(DisplayName = "Attacking"),
    Fleeing UMETA(DisplayName = "Fleeing"),
    Dead UMETA(DisplayName = "Dead")
};

UENUM(BlueprintType)
enum class ECombat_ThreatLevel : uint8
{
    None UMETA(DisplayName = "None"),
    Low UMETA(DisplayName = "Low"),
    Medium UMETA(DisplayName = "Medium"),
    High UMETA(DisplayName = "High"),
    Critical UMETA(DisplayName = "Critical")
};

USTRUCT(BlueprintType)
struct FCombat_ThreatData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    AActor* ThreatActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    ECombat_ThreatLevel ThreatLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float Distance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float LastSeenTime;

    FCombat_ThreatData()
    {
        ThreatActor = nullptr;
        ThreatLevel = ECombat_ThreatLevel::None;
        Distance = 0.0f;
        LastSeenTime = 0.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCombatStateManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombatStateManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Combat State Management
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetCombatState(ECombat_CombatState NewState);

    UFUNCTION(BlueprintPure, Category = "Combat")
    ECombat_CombatState GetCombatState() const { return CurrentCombatState; }

    // Threat Detection
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void RegisterThreat(AActor* ThreatActor, ECombat_ThreatLevel Level);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void RemoveThreat(AActor* ThreatActor);

    UFUNCTION(BlueprintPure, Category = "Combat")
    AActor* GetPrimaryThreat() const;

    UFUNCTION(BlueprintPure, Category = "Combat")
    float GetThreatDistance(AActor* ThreatActor) const;

    // Combat Parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float DetectionRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackRange = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float FleeHealthThreshold = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AggressionLevel = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    bool bIsPackHunter = false;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
    ECombat_CombatState CurrentCombatState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
    TArray<FCombat_ThreatData> KnownThreats;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
    AActor* PrimaryThreat;

    // Internal functions
    void UpdateThreatDetection();
    void UpdateCombatLogic();
    void ProcessStateTransitions();
    ECombat_ThreatLevel CalculateThreatLevel(AActor* ThreatActor, float Distance) const;
};