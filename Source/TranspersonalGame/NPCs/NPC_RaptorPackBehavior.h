#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "GameFramework/Pawn.h"
#include "NPC_RaptorPackBehavior.generated.h"

UENUM(BlueprintType)
enum class ENPC_RaptorPackState : uint8
{
    Hunting     UMETA(DisplayName = "Hunting"),
    Patrolling  UMETA(DisplayName = "Patrolling"),
    Feeding     UMETA(DisplayName = "Feeding"),
    Resting     UMETA(DisplayName = "Resting"),
    Fleeing     UMETA(DisplayName = "Fleeing")
};

USTRUCT(BlueprintType)
struct FNPC_RaptorPackMember
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    APawn* RaptorPawn;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    FVector LastKnownPosition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    float DistanceFromAlpha;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    bool bIsAlive;

    FNPC_RaptorPackMember()
    {
        RaptorPawn = nullptr;
        LastKnownPosition = FVector::ZeroVector;
        DistanceFromAlpha = 0.0f;
        bIsAlive = true;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPC_RaptorPackBehavior : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_RaptorPackBehavior();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Pack Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Behavior")
    TArray<FNPC_RaptorPackMember> PackMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Behavior")
    APawn* AlphaRaptor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Behavior")
    ENPC_RaptorPackState CurrentPackState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Behavior")
    FVector PackCenterLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Behavior")
    float PackCohesionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Behavior")
    float HuntingRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Behavior")
    float FleeDistance;

    // Hunting Behavior
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunting")
    APawn* CurrentTarget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunting")
    float HuntingCooldown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunting")
    float LastHuntTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunting")
    bool bIsHunting;

    // Pack Communication
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Communication")
    float CallRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Communication")
    float LastCallTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Communication")
    float CallCooldown;

    // Behavior Functions
    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void InitializePack(const TArray<APawn*>& Raptors);

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void SetPackState(ENPC_RaptorPackState NewState);

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void UpdatePackCohesion();

    UFUNCTION(BlueprintCallable, Category = "Hunting")
    void StartHunt(APawn* Target);

    UFUNCTION(BlueprintCallable, Category = "Hunting")
    void EndHunt();

    UFUNCTION(BlueprintCallable, Category = "Hunting")
    void ExecuteFlankingManeuver();

    UFUNCTION(BlueprintCallable, Category = "Communication")
    void EmitPackCall();

    UFUNCTION(BlueprintCallable, Category = "Communication")
    void RespondToPackCall(const FVector& CallLocation);

private:
    void UpdateHuntingBehavior();
    void UpdatePatrolBehavior();
    void UpdateFleeingBehavior();
    void UpdatePackFormation();
    void CheckForThreats();
    void AssignRoles();

    FTimerHandle HuntingTimer;
    FTimerHandle PatrolTimer;
    FTimerHandle CommunicationTimer;
};