#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "../../SharedTypes.h"
#include "NPC_DinosaurPackManager.generated.h"

class ANPC_DinosaurAI;

USTRUCT(BlueprintType)
struct FNPC_PackMember
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TWeakObjectPtr<ANPC_DinosaurAI> DinosaurRef;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DistanceFromLeader;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector RelativePosition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsAlpha;

    FNPC_PackMember()
    {
        DinosaurRef = nullptr;
        DistanceFromLeader = 0.0f;
        RelativePosition = FVector::ZeroVector;
        bIsAlpha = false;
    }
};

USTRUCT(BlueprintType)
struct FNPC_PackBehaviorState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ENPC_PackActivity CurrentActivity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector PackCenterLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PackRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ThreatLevel;

    FNPC_PackBehaviorState()
    {
        CurrentActivity = ENPC_PackActivity::Idle;
        PackCenterLocation = FVector::ZeroVector;
        TargetLocation = FVector::ZeroVector;
        PackRadius = 1000.0f;
        ThreatLevel = 0.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPC_DinosaurPackManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_DinosaurPackManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Pack Management
    UFUNCTION(BlueprintCallable, Category = "Pack Management")
    void AddPackMember(ANPC_DinosaurAI* NewMember, bool bAsAlpha = false);

    UFUNCTION(BlueprintCallable, Category = "Pack Management")
    void RemovePackMember(ANPC_DinosaurAI* MemberToRemove);

    UFUNCTION(BlueprintCallable, Category = "Pack Management")
    void SetPackLeader(ANPC_DinosaurAI* NewLeader);

    UFUNCTION(BlueprintCallable, Category = "Pack Management")
    ANPC_DinosaurAI* GetPackLeader() const;

    UFUNCTION(BlueprintCallable, Category = "Pack Management")
    TArray<ANPC_DinosaurAI*> GetPackMembers() const;

    UFUNCTION(BlueprintCallable, Category = "Pack Management")
    int32 GetPackSize() const;

    // Pack Behavior
    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void SetPackActivity(ENPC_PackActivity NewActivity);

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void UpdatePackFormation();

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void MovePackToLocation(const FVector& TargetLocation);

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void InitiatePackHunt(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void AlertPackToThreat(AActor* ThreatActor, float ThreatLevel);

    // Pack Communication
    UFUNCTION(BlueprintCallable, Category = "Pack Communication")
    void BroadcastPackSignal(ENPC_PackSignal Signal);

    UFUNCTION(BlueprintCallable, Category = "Pack Communication")
    void HandlePackMemberSignal(ANPC_DinosaurAI* Sender, ENPC_PackSignal Signal);

    // Pack State Queries
    UFUNCTION(BlueprintPure, Category = "Pack State")
    FVector GetPackCenterLocation() const;

    UFUNCTION(BlueprintPure, Category = "Pack State")
    float GetPackRadius() const;

    UFUNCTION(BlueprintPure, Category = "Pack State")
    bool IsPackInCombat() const;

    UFUNCTION(BlueprintPure, Category = "Pack State")
    bool IsPackHunting() const;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pack Data")
    TArray<FNPC_PackMember> PackMembers;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pack Data")
    FNPC_PackBehaviorState PackState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Settings")
    int32 MaxPackSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Settings")
    float PackCohesionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Settings")
    float PackSeparationRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Settings")
    float PackAlignmentRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Settings")
    float LeaderFollowDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Settings")
    float FormationUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Settings")
    float ThreatResponseRadius;

private:
    TWeakObjectPtr<ANPC_DinosaurAI> PackLeader;
    float LastFormationUpdate;
    FVector LastKnownThreatLocation;
    float CurrentThreatLevel;

    // Internal Methods
    void UpdatePackCohesion();
    void UpdatePackSeparation();
    void UpdatePackAlignment();
    void CalculatePackCenter();
    void AssignFormationPositions();
    FVector CalculateFlockingForce(ANPC_DinosaurAI* Member);
    bool ValidatePackMember(ANPC_DinosaurAI* Member) const;
};