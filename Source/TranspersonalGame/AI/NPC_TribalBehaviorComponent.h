#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "TranspersonalGame/SharedTypes.h"
#include "NPC_TribalBehaviorComponent.generated.h"

UENUM(BlueprintType)
enum class ENPC_TribalState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrolling  UMETA(DisplayName = "Patrolling"),
    Hunting     UMETA(DisplayName = "Hunting"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Gathering   UMETA(DisplayName = "Gathering"),
    Socializing UMETA(DisplayName = "Socializing"),
    Sleeping    UMETA(DisplayName = "Sleeping")
};

USTRUCT(BlueprintType)
struct FNPC_TribalMemory
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Memory")
    FVector LastKnownPlayerLocation;

    UPROPERTY(BlueprintReadWrite, Category = "Memory")
    float TimeSincePlayerSeen;

    UPROPERTY(BlueprintReadWrite, Category = "Memory")
    TArray<FVector> DangerousLocations;

    UPROPERTY(BlueprintReadWrite, Category = "Memory")
    TArray<FVector> SafeLocations;

    UPROPERTY(BlueprintReadWrite, Category = "Memory")
    float LastMealTime;

    UPROPERTY(BlueprintReadWrite, Category = "Memory")
    bool bHasSeenDinosaur;

    FNPC_TribalMemory()
    {
        LastKnownPlayerLocation = FVector::ZeroVector;
        TimeSincePlayerSeen = 999.0f;
        LastMealTime = 0.0f;
        bHasSeenDinosaur = false;
    }
};

USTRUCT(BlueprintType)
struct FNPC_TribalPersonality
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Personality")
    float Courage; // 0.0 = coward, 1.0 = fearless

    UPROPERTY(BlueprintReadWrite, Category = "Personality")
    float Curiosity; // 0.0 = cautious, 1.0 = explorer

    UPROPERTY(BlueprintReadWrite, Category = "Personality")
    float Sociability; // 0.0 = loner, 1.0 = social

    UPROPERTY(BlueprintReadWrite, Category = "Personality")
    float Aggression; // 0.0 = peaceful, 1.0 = hostile

    FNPC_TribalPersonality()
    {
        Courage = 0.5f;
        Curiosity = 0.5f;
        Sociability = 0.5f;
        Aggression = 0.3f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPC_TribalBehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_TribalBehaviorComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Estado actual do NPC
    UPROPERTY(BlueprintReadWrite, Category = "Behavior")
    ENPC_TribalState CurrentState;

    // Personalidade do NPC
    UPROPERTY(BlueprintReadWrite, Category = "Behavior")
    FNPC_TribalPersonality Personality;

    // Memória do NPC
    UPROPERTY(BlueprintReadWrite, Category = "Behavior")
    FNPC_TribalMemory Memory;

    // Configurações de comportamento
    UPROPERTY(BlueprintReadWrite, Category = "Behavior")
    float DetectionRadius;

    UPROPERTY(BlueprintReadWrite, Category = "Behavior")
    float FleeRadius;

    UPROPERTY(BlueprintReadWrite, Category = "Behavior")
    float PatrolRadius;

    UPROPERTY(BlueprintReadWrite, Category = "Behavior")
    float MovementSpeed;

    // Pontos de patrulha
    UPROPERTY(BlueprintReadWrite, Category = "Behavior")
    TArray<FVector> PatrolPoints;

    UPROPERTY(BlueprintReadWrite, Category = "Behavior")
    int32 CurrentPatrolIndex;

    // Funções de comportamento
    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void SetState(ENPC_TribalState NewState);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void UpdateMemory(const FVector& PlayerLocation, bool bPlayerVisible);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void AddDangerousLocation(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void AddSafeLocation(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    FVector GetNextPatrolPoint();

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    bool ShouldFlee() const;

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    bool ShouldApproachPlayer() const;

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    FVector GetFleeDirection() const;

protected:
    // Funções internas
    void UpdateBehavior(float DeltaTime);
    void HandleIdleState(float DeltaTime);
    void HandlePatrollingState(float DeltaTime);
    void HandleFleeingState(float DeltaTime);
    void HandleGatheringState(float DeltaTime);
    
    APawn* GetOwnerPawn() const;
    APawn* FindNearestPlayer() const;
    bool IsPlayerVisible(APawn* Player) const;
    float GetDistanceToPlayer() const;

private:
    float StateTimer;
    float LastBehaviorUpdate;
};

#include "NPC_TribalBehaviorComponent.generated.h"