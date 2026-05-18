#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "Components/ActorComponent.h"
#include "../Core/SharedTypes.h"
#include "NPC_BehaviorTreeManager.generated.h"

UENUM(BlueprintType)
enum class ENPC_BehaviorState : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Patrolling      UMETA(DisplayName = "Patrolling"),
    Hunting         UMETA(DisplayName = "Hunting"),
    Fleeing         UMETA(DisplayName = "Fleeing"),
    Grazing         UMETA(DisplayName = "Grazing"),
    Sleeping        UMETA(DisplayName = "Sleeping"),
    Territorial     UMETA(DisplayName = "Territorial"),
    PackHunting     UMETA(DisplayName = "Pack Hunting"),
    Mating          UMETA(DisplayName = "Mating"),
    Nesting         UMETA(DisplayName = "Nesting")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_BehaviorData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    ENPC_BehaviorState CurrentState = ENPC_BehaviorState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float AggressionLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float FearLevel = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float HungerLevel = 0.4f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float TerritorialRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float DetectionRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    bool bIsPackAnimal = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    int32 PackSize = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float StateChangeTimer = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    FVector HomeLocation = FVector::ZeroVector;

    FNPC_BehaviorData()
    {
        CurrentState = ENPC_BehaviorState::Idle;
        AggressionLevel = 0.5f;
        FearLevel = 0.3f;
        HungerLevel = 0.4f;
        TerritorialRadius = 2000.0f;
        DetectionRadius = 1500.0f;
        bIsPackAnimal = false;
        PackSize = 1;
        StateChangeTimer = 0.0f;
        HomeLocation = FVector::ZeroVector;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPC_BehaviorTreeManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_BehaviorTreeManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Behavior Management
    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void InitializeBehavior(ENPC_DinosaurSpecies Species);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void UpdateBehaviorState(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void SetBehaviorState(ENPC_BehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    ENPC_BehaviorState GetCurrentBehaviorState() const;

    // Pack Behavior
    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void JoinPack(UNPC_BehaviorTreeManager* PackLeader);

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void LeavePack();

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    bool IsInPack() const;

    // Territory Management
    UFUNCTION(BlueprintCallable, Category = "Territory")
    void SetTerritorialBehavior(FVector Center, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Territory")
    bool IsInTerritory(FVector Location) const;

    // Detection and Response
    UFUNCTION(BlueprintCallable, Category = "Detection")
    AActor* DetectNearbyThreats();

    UFUNCTION(BlueprintCallable, Category = "Detection")
    AActor* DetectNearbyPrey();

    UFUNCTION(BlueprintCallable, Category = "Response")
    void RespondToThreat(AActor* Threat);

    UFUNCTION(BlueprintCallable, Category = "Response")
    void RespondToPrey(AActor* Prey);

    // Behavior Tree Integration
    UFUNCTION(BlueprintCallable, Category = "AI")
    void StartBehaviorTree();

    UFUNCTION(BlueprintCallable, Category = "AI")
    void StopBehaviorTree();

    UFUNCTION(BlueprintCallable, Category = "AI")
    void UpdateBlackboard();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Data")
    FNPC_BehaviorData BehaviorData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    class UBehaviorTree* BehaviorTreeAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    class UBlackboardComponent* BlackboardComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    TArray<UNPC_BehaviorTreeManager*> PackMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    UNPC_BehaviorTreeManager* PackLeader;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection")
    TArray<AActor*> DetectedActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
    float BehaviorUpdateInterval = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
    float LastBehaviorUpdate = 0.0f;

private:
    void UpdateIdleBehavior(float DeltaTime);
    void UpdatePatrolBehavior(float DeltaTime);
    void UpdateHuntingBehavior(float DeltaTime);
    void UpdateFleeingBehavior(float DeltaTime);
    void UpdateGrazingBehavior(float DeltaTime);
    void UpdateTerritorialBehavior(float DeltaTime);
    void UpdatePackBehavior(float DeltaTime);

    bool ShouldChangeState() const;
    ENPC_BehaviorState DetermineNewState() const;
    void TransitionToState(ENPC_BehaviorState NewState);

    FVector GetRandomPatrolPoint() const;
    bool IsNearHomeLocation() const;
    float GetDistanceToHome() const;
};