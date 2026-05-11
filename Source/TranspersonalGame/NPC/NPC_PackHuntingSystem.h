#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "NPCBehaviorTypes.h"
#include "NPC_PackHuntingSystem.generated.h"

class ADinosaurBase;

UENUM(BlueprintType)
enum class ENPC_PackRole : uint8
{
    Leader      UMETA(DisplayName = "Pack Leader"),
    Hunter      UMETA(DisplayName = "Hunter"),
    Scout       UMETA(DisplayName = "Scout"),
    Guardian    UMETA(DisplayName = "Guardian"),
    Lone        UMETA(DisplayName = "Lone Wolf")
};

UENUM(BlueprintType)
enum class ENPC_PackState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Hunting     UMETA(DisplayName = "Hunting"),
    Pursuing    UMETA(DisplayName = "Pursuing"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Retreating  UMETA(DisplayName = "Retreating"),
    Regrouping  UMETA(DisplayName = "Regrouping")
};

USTRUCT(BlueprintType)
struct FNPC_PackMember
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    TWeakObjectPtr<ADinosaurBase> DinosaurRef;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    ENPC_PackRole Role;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    float DistanceFromLeader;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    bool bIsAlive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    float LastSeenTime;

    FNPC_PackMember()
    {
        DinosaurRef = nullptr;
        Role = ENPC_PackRole::Hunter;
        DistanceFromLeader = 0.0f;
        bIsAlive = true;
        LastSeenTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FNPC_HuntTarget
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunting")
    TWeakObjectPtr<AActor> TargetActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunting")
    FVector LastKnownLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunting")
    float ThreatLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunting")
    float LastSeenTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunting")
    bool bIsPlayer;

    FNPC_HuntTarget()
    {
        TargetActor = nullptr;
        LastKnownLocation = FVector::ZeroVector;
        ThreatLevel = 0.0f;
        LastSeenTime = 0.0f;
        bIsPlayer = false;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPC_PackHuntingSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_PackHuntingSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Pack Management
    UFUNCTION(BlueprintCallable, Category = "Pack Hunting")
    void InitializePack(ADinosaurBase* PackLeader);

    UFUNCTION(BlueprintCallable, Category = "Pack Hunting")
    void AddPackMember(ADinosaurBase* NewMember, ENPC_PackRole Role);

    UFUNCTION(BlueprintCallable, Category = "Pack Hunting")
    void RemovePackMember(ADinosaurBase* Member);

    UFUNCTION(BlueprintCallable, Category = "Pack Hunting")
    void SetPackState(ENPC_PackState NewState);

    // Hunting Behavior
    UFUNCTION(BlueprintCallable, Category = "Pack Hunting")
    void StartHunt(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Pack Hunting")
    void AssignHuntingPositions();

    UFUNCTION(BlueprintCallable, Category = "Pack Hunting")
    void ExecuteCoordinatedAttack();

    UFUNCTION(BlueprintCallable, Category = "Pack Hunting")
    void RetreatFromDanger();

    // Communication
    UFUNCTION(BlueprintCallable, Category = "Pack Hunting")
    void BroadcastPackSignal(const FString& SignalType);

    UFUNCTION(BlueprintCallable, Category = "Pack Hunting")
    void RespondToPackSignal(const FString& SignalType, ADinosaurBase* Sender);

    // Getters
    UFUNCTION(BlueprintPure, Category = "Pack Hunting")
    ENPC_PackState GetPackState() const { return CurrentPackState; }

    UFUNCTION(BlueprintPure, Category = "Pack Hunting")
    int32 GetPackSize() const { return PackMembers.Num(); }

    UFUNCTION(BlueprintPure, Category = "Pack Hunting")
    bool IsPackLeader() const { return bIsPackLeader; }

protected:
    // Pack Data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Settings")
    bool bIsPackLeader;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Settings")
    TWeakObjectPtr<ADinosaurBase> PackLeaderRef;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Settings")
    TArray<FNPC_PackMember> PackMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Settings")
    ENPC_PackState CurrentPackState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Settings")
    int32 MaxPackSize;

    // Hunting Data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunting Settings")
    FNPC_HuntTarget CurrentTarget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunting Settings")
    TArray<FNPC_HuntTarget> PotentialTargets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunting Settings")
    float HuntingRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunting Settings")
    float CoordinationRadius;

    // Behavior Parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float PackCohesionStrength;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float HuntingAggression;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float RetreatThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float CommunicationRange;

private:
    // Internal Methods
    void UpdatePackCohesion();
    void ScanForTargets();
    void EvaluateTargetThreat(AActor* Target, FNPC_HuntTarget& HuntTarget);
    void UpdatePackMemberPositions();
    void CheckPackMemberStatus();
    FVector CalculateFlankingPosition(const FVector& TargetLocation, int32 MemberIndex);
    bool IsTargetWithinRange(AActor* Target) const;
    void CleanupInvalidTargets();

    // Timers
    float LastTargetScanTime;
    float LastCohesionUpdateTime;
    float LastCommunicationTime;
    
    // Constants
    static constexpr float TARGET_SCAN_INTERVAL = 2.0f;
    static constexpr float COHESION_UPDATE_INTERVAL = 1.0f;
    static constexpr float COMMUNICATION_INTERVAL = 3.0f;
};