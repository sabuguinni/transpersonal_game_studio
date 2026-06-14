#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "TimerManager.h"
#include "NPCBehaviorComponent.generated.h"

UENUM(BlueprintType)
enum class ENPC_BehaviorState : uint8
{
    Idle UMETA(DisplayName = "Idle"),
    Patrol UMETA(DisplayName = "Patrol"),
    Chase UMETA(DisplayName = "Chase"),
    Attack UMETA(DisplayName = "Attack"),
    Flee UMETA(DisplayName = "Flee"),
    Dead UMETA(DisplayName = "Dead")
};

UENUM(BlueprintType)
enum class ENPC_DinosaurType : uint8
{
    TRex UMETA(DisplayName = "T-Rex"),
    Velociraptor UMETA(DisplayName = "Velociraptor"),
    Triceratops UMETA(DisplayName = "Triceratops"),
    Brachiosaurus UMETA(DisplayName = "Brachiosaurus"),
    Ankylosaurus UMETA(DisplayName = "Ankylosaurus")
};

USTRUCT(BlueprintType)
struct FNPC_BehaviorStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float PatrolRadius = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float ChaseDistance = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float AttackDistance = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float FleeDistance = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float MovementSpeed = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float Aggression = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float Fear = 0.3f;

    FNPC_BehaviorStats()
    {
        PatrolRadius = 3000.0f;
        ChaseDistance = 2000.0f;
        AttackDistance = 300.0f;
        FleeDistance = 1000.0f;
        MovementSpeed = 600.0f;
        Aggression = 0.7f;
        Fear = 0.3f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPCBehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPCBehaviorComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    ENPC_BehaviorState CurrentState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    ENPC_DinosaurType DinosaurType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    FNPC_BehaviorStats BehaviorStats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    FVector PatrolCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    FVector CurrentTarget;

    UPROPERTY(BlueprintReadOnly, Category = "NPC Behavior")
    AActor* PlayerTarget;

    UPROPERTY(BlueprintReadOnly, Category = "NPC Behavior")
    float DistanceToPlayer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    bool bIsPackHunter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    TArray<AActor*> PackMembers;

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void SetBehaviorState(ENPC_BehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void InitializeBehavior(ENPC_DinosaurType Type);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void UpdateBehavior(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void FindNearestPlayer();

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void PatrolBehavior();

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void ChaseBehavior();

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void AttackBehavior();

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void FleeBehavior();

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void SetupDinosaurStats(ENPC_DinosaurType Type);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void AddPackMember(AActor* Member);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void CoordinatePackBehavior();

private:
    FTimerHandle BehaviorUpdateTimer;
    FVector LastKnownPlayerLocation;
    float StateChangeTimer;
    float PatrolWaitTimer;
    bool bHasValidTarget;
};