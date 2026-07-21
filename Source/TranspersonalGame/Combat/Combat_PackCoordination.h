#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"
#include "../SharedTypes.h"
#include "NPC/NPC_PackCommunication.h"
#include "Combat_PackCoordination.generated.h"

UENUM(BlueprintType)
enum class ECombat_PackRole : uint8
{
    Alpha       UMETA(DisplayName = "Alpha Leader"),
    Beta        UMETA(DisplayName = "Beta Second"),
    Flanker     UMETA(DisplayName = "Flanker"),
    Distraction UMETA(DisplayName = "Distraction"),
    Ambusher    UMETA(DisplayName = "Ambusher"),
    Scout       UMETA(DisplayName = "Scout")
};

UENUM(BlueprintType)
enum class ECombat_TacticalState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Stalking    UMETA(DisplayName = "Stalking"),
    Positioning UMETA(DisplayName = "Positioning"),
    Coordinated UMETA(DisplayName = "Coordinated Attack"),
    Flanking    UMETA(DisplayName = "Flanking"),
    Retreating  UMETA(DisplayName = "Retreating"),
    Regrouping  UMETA(DisplayName = "Regrouping")
};

USTRUCT(BlueprintType)
struct FCombat_PackMember
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    APawn* MemberPawn;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    ECombat_PackRole Role;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    FVector AssignedPosition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    bool bInPosition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    float DistanceToTarget;

    FCombat_PackMember()
    {
        MemberPawn = nullptr;
        Role = ECombat_PackRole::Scout;
        AssignedPosition = FVector::ZeroVector;
        bInPosition = false;
        DistanceToTarget = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FCombat_AttackFormation
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    FString FormationName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    TArray<FVector> RelativePositions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    float MinDistanceToTarget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    float MaxDistanceToTarget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    bool bRequiresLineOfSight;

    FCombat_AttackFormation()
    {
        FormationName = TEXT("Default");
        MinDistanceToTarget = 500.0f;
        MaxDistanceToTarget = 2000.0f;
        bRequiresLineOfSight = true;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCombat_PackCoordination : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombat_PackCoordination();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Pack Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Management")
    TArray<FCombat_PackMember> PackMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Management")
    APawn* PackLeader;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Management")
    APawn* CurrentTarget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Management")
    ECombat_TacticalState CurrentTacticalState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Management")
    int32 MaxPackSize;

    // Formation System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    TArray<FCombat_AttackFormation> AvailableFormations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    FCombat_AttackFormation CurrentFormation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    float FormationRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    float PositionTolerance;

    // Tactical Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactics")
    float StallingDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactics")
    float AttackRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactics")
    float FlankingAngle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactics")
    float CoordinationDelay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactics")
    bool bUseAdvancedTactics;

    // Communication Integration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Communication")
    class UNPC_PackCommunication* PackCommunicationComponent;

    // Public Functions
    UFUNCTION(BlueprintCallable, Category = "Pack Management")
    bool AddPackMember(APawn* NewMember, ECombat_PackRole Role);

    UFUNCTION(BlueprintCallable, Category = "Pack Management")
    bool RemovePackMember(APawn* Member);

    UFUNCTION(BlueprintCallable, Category = "Pack Management")
    void SetPackTarget(APawn* Target);

    UFUNCTION(BlueprintCallable, Category = "Formation")
    void SetFormation(const FCombat_AttackFormation& NewFormation);

    UFUNCTION(BlueprintCallable, Category = "Formation")
    void CalculateFormationPositions();

    UFUNCTION(BlueprintCallable, Category = "Tactics")
    void InitiateCoordinatedAttack();

    UFUNCTION(BlueprintCallable, Category = "Tactics")
    void ExecuteFlankingManeuver();

    UFUNCTION(BlueprintCallable, Category = "Tactics")
    void OrderRetreat();

    UFUNCTION(BlueprintCallable, Category = "Tactics")
    void RegroupPack();

    UFUNCTION(BlueprintCallable, Category = "Communication")
    void BroadcastTacticalCommand(const FString& Command);

private:
    // Internal Functions
    void UpdatePackCoordination(float DeltaTime);
    void CheckFormationPositions();
    void AssignPackRoles();
    void UpdateTacticalState();
    bool IsPackInPosition();
    bool HasLineOfSightToTarget(APawn* Member);
    FVector CalculateFlankingPosition(APawn* Member);
    void ExecuteTacticalAI();

    // Timers
    float CoordinationTimer;
    float LastCommandTime;
    float FormationUpdateTimer;
};