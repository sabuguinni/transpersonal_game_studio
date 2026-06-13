#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "SharedTypes.h"
#include "NPC_BehaviorComponent.generated.h"

UENUM(BlueprintType)
enum class ENPC_BehaviorState : uint8
{
    Idle UMETA(DisplayName = "Idle"),
    Patrolling UMETA(DisplayName = "Patrolling"),
    Investigating UMETA(DisplayName = "Investigating"),
    Fleeing UMETA(DisplayName = "Fleeing"),
    Attacking UMETA(DisplayName = "Attacking"),
    Following UMETA(DisplayName = "Following"),
    Gathering UMETA(DisplayName = "Gathering"),
    Socializing UMETA(DisplayName = "Socializing")
};

USTRUCT(BlueprintType)
struct FNPC_BehaviorMemory
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    TArray<FVector> KnownLocations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    TArray<AActor*> KnownActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float LastPlayerInteraction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    bool bPlayerIsThreat;

    FNPC_BehaviorMemory()
    {
        LastPlayerInteraction = 0.0f;
        bPlayerIsThreat = false;
    }
};

USTRUCT(BlueprintType)
struct FNPC_DailyRoutine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Routine")
    float StartTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Routine")
    float EndTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Routine")
    ENPC_BehaviorState Activity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Routine")
    FVector TargetLocation;

    FNPC_DailyRoutine()
    {
        StartTime = 0.0f;
        EndTime = 24.0f;
        Activity = ENPC_BehaviorState::Idle;
        TargetLocation = FVector::ZeroVector;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPC_BehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_BehaviorComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core behavior state
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    ENPC_BehaviorState CurrentBehaviorState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    UBehaviorTree* BehaviorTreeAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float BehaviorRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float PlayerDetectionRadius;

    // Memory system
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FNPC_BehaviorMemory NPCMemory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float MemoryDuration;

    // Daily routine
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Routine")
    TArray<FNPC_DailyRoutine> DailyRoutines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Routine")
    bool bFollowDailyRoutine;

    // Social behavior
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    TArray<AActor*> KnownNPCs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    float SocialInteractionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    bool bCanSocialize;

    // Behavior functions
    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void SetBehaviorState(ENPC_BehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void StartBehaviorTree();

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void StopBehaviorTree();

    UFUNCTION(BlueprintCallable, Category = "Memory")
    void RememberLocation(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Memory")
    void RememberActor(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Memory")
    bool IsLocationKnown(FVector Location, float Tolerance = 500.0f);

    UFUNCTION(BlueprintCallable, Category = "Routine")
    FNPC_DailyRoutine GetCurrentRoutine();

    UFUNCTION(BlueprintCallable, Category = "Routine")
    void UpdateDailyRoutine(float CurrentTime);

    UFUNCTION(BlueprintCallable, Category = "Social")
    void RegisterNearbyNPC(AActor* NPC);

    UFUNCTION(BlueprintCallable, Category = "Social")
    TArray<AActor*> GetNearbyNPCs();

private:
    AAIController* GetAIController();
    void UpdateMemory(float DeltaTime);
    void ProcessDailyRoutine();
    void HandlePlayerDetection();
    
    float CurrentGameTime;
    AActor* LastKnownPlayer;
    float LastPlayerDetectionTime;
};