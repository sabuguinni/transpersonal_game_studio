#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "SharedTypes.h"
#include "NPC_PackCoordinationSystem.generated.h"

class ANPC_TribalNPCBase;

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_PackMember
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Member")
    TWeakObjectPtr<ANPC_TribalNPCBase> NPCRef;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Member")
    ENPC_PackRole Role;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Member")
    float DistanceFromLeader;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Member")
    float LastCommunicationTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Member")
    bool bIsAlive;

    FNPC_PackMember()
    {
        NPCRef = nullptr;
        Role = ENPC_PackRole::Follower;
        DistanceFromLeader = 0.0f;
        LastCommunicationTime = 0.0f;
        bIsAlive = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_HuntingFormation
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    ENPC_HuntingStrategy Strategy;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    TArray<FVector> MemberPositions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    float FormationRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    float OptimalDistance;

    FNPC_HuntingFormation()
    {
        Strategy = ENPC_HuntingStrategy::Surround;
        FormationRadius = 500.0f;
        OptimalDistance = 300.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPC_PackCoordinationSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_PackCoordinationSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Pack Management
    UFUNCTION(BlueprintCallable, Category = "Pack Coordination")
    void InitializeAsPack(TArray<ANPC_TribalNPCBase*> PackMembers, ANPC_TribalNPCBase* PackLeader);

    UFUNCTION(BlueprintCallable, Category = "Pack Coordination")
    void AddPackMember(ANPC_TribalNPCBase* NewMember, ENPC_PackRole Role);

    UFUNCTION(BlueprintCallable, Category = "Pack Coordination")
    void RemovePackMember(ANPC_TribalNPCBase* MemberToRemove);

    UFUNCTION(BlueprintCallable, Category = "Pack Coordination")
    bool IsPackLeader() const;

    UFUNCTION(BlueprintCallable, Category = "Pack Coordination")
    ANPC_TribalNPCBase* GetPackLeader() const;

    // Hunting Coordination
    UFUNCTION(BlueprintCallable, Category = "Hunting")
    void StartPackHunt(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Hunting")
    void UpdateHuntingFormation(ENPC_HuntingStrategy Strategy);

    UFUNCTION(BlueprintCallable, Category = "Hunting")
    FVector GetOptimalPositionForMember(ANPC_TribalNPCBase* Member) const;

    UFUNCTION(BlueprintCallable, Category = "Hunting")
    void CoordinateAttack();

    // Communication
    UFUNCTION(BlueprintCallable, Category = "Communication")
    void SendPackSignal(ENPC_PackSignal Signal, const FVector& TargetLocation = FVector::ZeroVector);

    UFUNCTION(BlueprintCallable, Category = "Communication")
    void ReceivePackSignal(ENPC_PackSignal Signal, const FVector& SignalLocation, ANPC_TribalNPCBase* Sender);

    // Pack Behavior
    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void UpdatePackCohesion();

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void HandleMemberDeath(ANPC_TribalNPCBase* DeadMember);

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void ElectNewLeader();

    // Getters
    UFUNCTION(BlueprintPure, Category = "Pack Coordination")
    int32 GetPackSize() const { return PackMembers.Num(); }

    UFUNCTION(BlueprintPure, Category = "Pack Coordination")
    float GetPackCohesion() const { return PackCohesion; }

    UFUNCTION(BlueprintPure, Category = "Pack Coordination")
    bool IsHunting() const { return bIsHunting; }

protected:
    // Pack Data
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pack Data")
    TArray<FNPC_PackMember> PackMembers;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pack Data")
    TWeakObjectPtr<ANPC_TribalNPCBase> PackLeader;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pack Data")
    TWeakObjectPtr<AActor> CurrentTarget;

    // Hunting Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunting Config")
    FNPC_HuntingFormation CurrentFormation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunting Config")
    float MaxHuntingDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunting Config")
    float AttackCoordinationDelay;

    // Pack Behavior
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pack Behavior")
    float PackCohesion;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pack Behavior")
    bool bIsHunting;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pack Behavior")
    float LastSignalTime;

    // Communication
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Communication")
    float CommunicationRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Communication")
    float SignalCooldown;

private:
    // Internal Methods
    void CalculatePackCohesion();
    void UpdateMemberPositions();
    FVector CalculateFormationPosition(const FNPC_PackMember& Member) const;
    bool IsWithinCommunicationRange(const ANPC_TribalNPCBase* Member) const;
    void BroadcastToPackMembers(ENPC_PackSignal Signal, const FVector& Location);

public:
    UNPC_PackCoordinationSystem()
    {
        PrimaryComponentTick.bCanEverTick = true;
        MaxHuntingDistance = 2000.0f;
        AttackCoordinationDelay = 2.0f;
        PackCohesion = 1.0f;
        bIsHunting = false;
        LastSignalTime = 0.0f;
        CommunicationRange = 1500.0f;
        SignalCooldown = 3.0f;
    }
};