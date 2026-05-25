#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Combat_TacticalAI.generated.h"

UENUM(BlueprintType)
enum class ECombat_TacticalState : uint8
{
    Idle UMETA(DisplayName = "Idle"),
    Patrol UMETA(DisplayName = "Patrol"),
    Hunt UMETA(DisplayName = "Hunt"),
    Attack UMETA(DisplayName = "Attack"),
    Retreat UMETA(DisplayName = "Retreat"),
    Pack UMETA(DisplayName = "Pack Coordination")
};

USTRUCT(BlueprintType)
struct FCombat_ThreatAssessment
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    float ThreatLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    float Distance;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    AActor* ThreatActor;

    FCombat_ThreatAssessment()
    {
        ThreatLevel = 0.0f;
        Distance = 0.0f;
        ThreatActor = nullptr;
    }
};

UCLASS(ClassGroup=(Combat), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCombat_TacticalAI : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombat_TacticalAI();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    ECombat_TacticalState CurrentState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float DetectionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float AttackRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float RetreatThreshold;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI")
    TArray<FCombat_ThreatAssessment> ThreatList;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void DetectThreats();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void UpdateTacticalState();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void ExecuteAttackBehavior();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void ExecuteRetreatBehavior();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    AActor* GetPrimaryThreat();

private:
    float LastThreatCheck;
    float ThreatCheckInterval;
};