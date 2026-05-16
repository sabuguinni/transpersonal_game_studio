#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/WorldSubsystem.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "TimerManager.h"
#include "SharedTypes.h"
#include "NPC_DinosaurBehaviorManager.generated.h"

class ADinosaurBase;
class ANPCBehaviorController;

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_DinosaurState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur State")
    float Health = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur State")
    float Hunger = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur State")
    float Thirst = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur State")
    float Stamina = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur State")
    float Aggression = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur State")
    float Fear = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur State")
    bool bIsHunting = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur State")
    bool bIsFeeding = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur State")
    bool bIsResting = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur State")
    bool bIsPatrolling = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur State")
    FVector LastKnownPlayerLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur State")
    float TimeSincePlayerSeen = 999.0f;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_DinosaurBehaviorData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float PatrolRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float ChaseDistance = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float AttackDistance = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float HearingRange = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float SightRange = 4000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float MovementSpeed = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float ChaseSpeed = 1200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float RestDuration = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float HuntingCooldown = 120.0f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UNPC_DinosaurBehaviorManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UNPC_DinosaurBehaviorManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Dinosaur registration
    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void RegisterDinosaur(ADinosaurBase* Dinosaur);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void UnregisterDinosaur(ADinosaurBase* Dinosaur);

    // Behavior management
    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void UpdateDinosaurBehavior(ADinosaurBase* Dinosaur, float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void SetDinosaurState(ADinosaurBase* Dinosaur, const FNPC_DinosaurState& NewState);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    FNPC_DinosaurState GetDinosaurState(ADinosaurBase* Dinosaur);

    // Player interaction
    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void NotifyPlayerSeen(ADinosaurBase* Dinosaur, const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void NotifyPlayerHeard(ADinosaurBase* Dinosaur, const FVector& SoundLocation);

    // Pack behavior
    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    TArray<ADinosaurBase*> GetNearbyDinosaurs(ADinosaurBase* Dinosaur, float Radius);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void CoordinatePackBehavior(const TArray<ADinosaurBase*>& PackMembers);

    // Behavior tree integration
    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void StartBehaviorTree(ADinosaurBase* Dinosaur);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void StopBehaviorTree(ADinosaurBase* Dinosaur);

protected:
    UPROPERTY()
    TMap<ADinosaurBase*, FNPC_DinosaurState> DinosaurStates;

    UPROPERTY()
    TMap<ADinosaurBase*, FNPC_DinosaurBehaviorData> DinosaurBehaviorData;

    UPROPERTY()
    TArray<ADinosaurBase*> RegisteredDinosaurs;

    FTimerHandle BehaviorUpdateTimer;

    void UpdateAllDinosaurs();
    void ProcessDinosaurAI(ADinosaurBase* Dinosaur);
    void HandleHuntingBehavior(ADinosaurBase* Dinosaur);
    void HandlePatrolBehavior(ADinosaurBase* Dinosaur);
    void HandleRestingBehavior(ADinosaurBase* Dinosaur);
    void UpdateDinosaurStats(ADinosaurBase* Dinosaur, float DeltaTime);

    bool CanSeePlayer(ADinosaurBase* Dinosaur, const FVector& PlayerLocation);
    bool IsPlayerInRange(ADinosaurBase* Dinosaur, const FVector& PlayerLocation, float Range);
    FVector GetRandomPatrolPoint(ADinosaurBase* Dinosaur);
};