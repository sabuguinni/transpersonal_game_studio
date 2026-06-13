#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Combat_TacticalAI.generated.h"

UENUM(BlueprintType)
enum class ECombat_TacticalState : uint8
{
    Idle UMETA(DisplayName = "Idle"),
    Hunting UMETA(DisplayName = "Hunting"), 
    Stalking UMETA(DisplayName = "Stalking"),
    Attacking UMETA(DisplayName = "Attacking"),
    Fleeing UMETA(DisplayName = "Fleeing"),
    Circling UMETA(DisplayName = "Circling"),
    Ambushing UMETA(DisplayName = "Ambushing"),
    Retreating UMETA(DisplayName = "Retreating")
};

UENUM(BlueprintType)
enum class ECombat_DinosaurType : uint8
{
    Predator UMETA(DisplayName = "Predator"),
    Herbivore UMETA(DisplayName = "Herbivore"),
    Scavenger UMETA(DisplayName = "Scavenger"),
    PackHunter UMETA(DisplayName = "Pack Hunter"),
    Apex UMETA(DisplayName = "Apex Predator")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_TacticalProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    ECombat_DinosaurType DinosaurType = ECombat_DinosaurType::Predator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Aggression = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Intelligence = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float PackCoordination = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float AttackRange = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float FleeThreshold = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float CirclingDistance = 800.0f;

    FCombat_TacticalProfile()
    {
        DinosaurType = ECombat_DinosaurType::Predator;
        Aggression = 0.7f;
        Intelligence = 0.5f;
        PackCoordination = 0.3f;
        AttackRange = 500.0f;
        FleeThreshold = 0.2f;
        CirclingDistance = 800.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
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
    FCombat_TacticalProfile TacticalProfile;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI")
    ECombat_TacticalState CurrentState;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI")
    AActor* CurrentTarget;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI")
    TArray<AActor*> PackMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float StateChangeTimer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float LastAttackTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    FVector LastKnownTargetLocation;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetTacticalState(ECombat_TacticalState NewState);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetTarget(AActor* NewTarget);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void UpdateTacticalDecision(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void ExecuteAttackPattern();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void ExecuteFlankingManeuver();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void CoordinateWithPack();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool IsTargetInRange() const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool ShouldFlee() const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    FVector CalculateOptimalPosition() const;

private:
    void UpdateStateTimer(float DeltaTime);
    void ProcessTacticalLogic();
    void HandlePackBehavior();
    void UpdateTargetTracking();
};

#include "Combat_TacticalAI.generated.h"