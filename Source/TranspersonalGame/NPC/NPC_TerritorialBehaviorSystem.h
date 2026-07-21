#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "SharedTypes.h"
#include "NPC_TerritorialBehaviorSystem.generated.h"

UENUM(BlueprintType)
enum class ENPC_TerritoryType : uint8
{
    ApexPredator     UMETA(DisplayName = "Apex Predator Territory"),
    PackHunter       UMETA(DisplayName = "Pack Hunter Zone"),
    Herbivore        UMETA(DisplayName = "Herbivore Grazing Area"),
    WaterSource      UMETA(DisplayName = "Water Source Territory"),
    NeutralZone      UMETA(DisplayName = "Neutral Zone")
};

UENUM(BlueprintType)
enum class ENPC_TerritorialResponse : uint8
{
    Ignore           UMETA(DisplayName = "Ignore Intruder"),
    Investigate      UMETA(DisplayName = "Investigate Cautiously"),
    Warning          UMETA(DisplayName = "Issue Warning"),
    Aggressive       UMETA(DisplayName = "Aggressive Response"),
    Flee             UMETA(DisplayName = "Flee Territory")
};

USTRUCT(BlueprintType)
struct FNPC_TerritoryZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    FVector CenterLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    float Radius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    ENPC_TerritoryType TerritoryType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    TArray<FVector> PatrolPoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    TArray<AActor*> TerritoryOwners;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    float AggressionLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    bool bActiveTerritory;

    FNPC_TerritoryZone()
    {
        CenterLocation = FVector::ZeroVector;
        Radius = 1000.0f;
        TerritoryType = ENPC_TerritoryType::NeutralZone;
        AggressionLevel = 0.5f;
        bActiveTerritory = true;
    }
};

USTRUCT(BlueprintType)
struct FNPC_PatrolBehavior
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol")
    TArray<FVector> PatrolRoute;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol")
    int32 CurrentPatrolIndex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol")
    float PatrolSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol")
    float WaitTimeAtPoint;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol")
    bool bReverseRoute;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol")
    float LastPatrolTime;

    FNPC_PatrolBehavior()
    {
        CurrentPatrolIndex = 0;
        PatrolSpeed = 300.0f;
        WaitTimeAtPoint = 2.0f;
        bReverseRoute = false;
        LastPatrolTime = 0.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPC_TerritorialBehaviorSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_TerritorialBehaviorSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Territory Management
    UFUNCTION(BlueprintCallable, Category = "Territorial Behavior")
    void SetTerritoryZone(const FNPC_TerritoryZone& NewTerritory);

    UFUNCTION(BlueprintCallable, Category = "Territorial Behavior")
    bool IsInTerritory(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Territorial Behavior")
    float GetDistanceFromTerritoryCenter(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Territorial Behavior")
    ENPC_TerritorialResponse GetResponseToIntruder(AActor* Intruder) const;

    // Patrol Behavior
    UFUNCTION(BlueprintCallable, Category = "Territorial Behavior")
    void StartPatrolling();

    UFUNCTION(BlueprintCallable, Category = "Territorial Behavior")
    void StopPatrolling();

    UFUNCTION(BlueprintCallable, Category = "Territorial Behavior")
    FVector GetNextPatrolPoint();

    UFUNCTION(BlueprintCallable, Category = "Territorial Behavior")
    void AdvancePatrolPoint();

    // Territory Defense
    UFUNCTION(BlueprintCallable, Category = "Territorial Behavior")
    void DefendTerritory(AActor* Threat);

    UFUNCTION(BlueprintCallable, Category = "Territorial Behavior")
    void IssueWarning(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Territorial Behavior")
    void ReturnToTerritory();

    // Territory Sharing (for pack animals)
    UFUNCTION(BlueprintCallable, Category = "Territorial Behavior")
    void ShareTerritoryWith(AActor* PackMember);

    UFUNCTION(BlueprintCallable, Category = "Territorial Behavior")
    void RemoveFromTerritory(AActor* FormerMember);

    UFUNCTION(BlueprintCallable, Category = "Territorial Behavior")
    bool IsPackMember(AActor* Actor) const;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory", meta = (AllowPrivateAccess = "true"))
    FNPC_TerritoryZone TerritoryData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol", meta = (AllowPrivateAccess = "true"))
    FNPC_PatrolBehavior PatrolBehavior;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory", meta = (AllowPrivateAccess = "true"))
    bool bIsPatrolling;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory", meta = (AllowPrivateAccess = "true"))
    bool bDefendingTerritory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory", meta = (AllowPrivateAccess = "true"))
    AActor* CurrentThreat;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory", meta = (AllowPrivateAccess = "true"))
    float ThreatDetectionRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory", meta = (AllowPrivateAccess = "true"))
    float WarningDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory", meta = (AllowPrivateAccess = "true"))
    float AttackDistance;

private:
    void UpdatePatrolBehavior(float DeltaTime);
    void CheckForIntruders();
    void HandleTerritorialResponse(AActor* Intruder, ENPC_TerritorialResponse Response);
    bool ShouldRespondToActor(AActor* Actor) const;
    float CalculateAggressionModifier(AActor* Target) const;
};