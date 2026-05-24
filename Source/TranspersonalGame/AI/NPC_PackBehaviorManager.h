#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "../SharedTypes.h"
#include "NPC_PackBehaviorManager.generated.h"

USTRUCT(BlueprintType)
struct FNPC_PackMember
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    TWeakObjectPtr<APawn> Member;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    ENPC_PackRole Role;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    float DistanceFromLeader;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    bool bIsAlive;

    FNPC_PackMember()
    {
        Member = nullptr;
        Role = ENPC_PackRole::Follower;
        DistanceFromLeader = 0.0f;
        bIsAlive = true;
    }
};

USTRUCT(BlueprintType)
struct FNPC_HuntingTarget
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunting")
    TWeakObjectPtr<APawn> Target;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunting")
    FVector LastKnownLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunting")
    float ThreatLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunting")
    float TimeSinceLastSeen;

    FNPC_HuntingTarget()
    {
        Target = nullptr;
        LastKnownLocation = FVector::ZeroVector;
        ThreatLevel = 0.0f;
        TimeSinceLastSeen = 0.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPC_PackBehaviorManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_PackBehaviorManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Pack Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Behavior")
    TArray<FNPC_PackMember> PackMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Behavior")
    TWeakObjectPtr<APawn> PackLeader;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Behavior")
    ENPC_PackRole MyRole;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Behavior")
    float PackCohesionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Behavior")
    float PackSeparationRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Behavior")
    int32 MaxPackSize;

    // Hunting Coordination
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunting")
    TArray<FNPC_HuntingTarget> SharedTargets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunting")
    ENPC_HuntingStrategy CurrentStrategy;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunting")
    float HuntingRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunting")
    bool bIsActivelyHunting;

    // Communication
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Communication")
    float CommunicationRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Communication")
    TArray<ENPC_CommunicationType> RecentCommunications;

public:
    // Pack Management Functions
    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void JoinPack(APawn* NewLeader);

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void LeavePack();

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void AddPackMember(APawn* NewMember, ENPC_PackRole Role);

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void RemovePackMember(APawn* Member);

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    bool IsPackLeader() const;

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    FVector GetPackCenterLocation() const;

    // Hunting Coordination
    UFUNCTION(BlueprintCallable, Category = "Hunting")
    void ShareTargetWithPack(APawn* Target, float ThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Hunting")
    void SetHuntingStrategy(ENPC_HuntingStrategy Strategy);

    UFUNCTION(BlueprintCallable, Category = "Hunting")
    APawn* GetBestHuntingTarget() const;

    UFUNCTION(BlueprintCallable, Category = "Hunting")
    void CoordinateAttack(APawn* Target);

    // Communication
    UFUNCTION(BlueprintCallable, Category = "Communication")
    void SendCommunication(ENPC_CommunicationType Type, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Communication")
    void ReceiveCommunication(ENPC_CommunicationType Type, const FVector& Location, APawn* Sender);

    // Behavior Queries
    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    bool ShouldFollowLeader() const;

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    bool ShouldMaintainFormation() const;

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    FVector GetFormationPosition() const;

private:
    void UpdatePackCohesion(float DeltaTime);
    void UpdateHuntingTargets(float DeltaTime);
    void ProcessCommunications(float DeltaTime);
    void ManagePackLeadership();
    
    float LastCommunicationTime;
    float LastTargetUpdateTime;
    float LastCohesionUpdateTime;
};