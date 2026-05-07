#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "SharedTypes.h"
#include "Combat_IntelligentPackAI.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_PackFormation
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Formation")
    TArray<FVector> MemberPositions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Formation")
    FVector TargetPosition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Formation")
    float FormationRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Formation")
    int32 PackSize;

    FCombat_PackFormation()
    {
        TargetPosition = FVector::ZeroVector;
        FormationRadius = 500.0f;
        PackSize = 3;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_TacticalRole
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical Role")
    FString RoleName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical Role")
    FVector PreferredPosition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical Role")
    float AttackRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical Role")
    float MovementSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical Role")
    bool bIsLeader;

    FCombat_TacticalRole()
    {
        RoleName = TEXT("Flanker");
        PreferredPosition = FVector::ZeroVector;
        AttackRange = 300.0f;
        MovementSpeed = 600.0f;
        bIsLeader = false;
    }
};

UENUM(BlueprintType)
enum class ECombat_PackState : uint8
{
    Idle UMETA(DisplayName = "Idle"),
    Hunting UMETA(DisplayName = "Hunting"),
    Coordinating UMETA(DisplayName = "Coordinating"),
    Attacking UMETA(DisplayName = "Attacking"),
    Retreating UMETA(DisplayName = "Retreating"),
    Regrouping UMETA(DisplayName = "Regrouping")
};

UENUM(BlueprintType)
enum class ECombat_CommunicationType : uint8
{
    Visual UMETA(DisplayName = "Visual Signal"),
    Audio UMETA(DisplayName = "Audio Call"),
    Pheromone UMETA(DisplayName = "Scent Marking"),
    Movement UMETA(DisplayName = "Body Language")
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCombat_IntelligentPackAI : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombat_IntelligentPackAI();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Pack Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack AI")
    TArray<APawn*> PackMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack AI")
    APawn* PackLeader;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack AI")
    APawn* CurrentTarget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack AI")
    ECombat_PackState CurrentPackState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack AI")
    FCombat_PackFormation CurrentFormation;

    // Intelligence Parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Intelligence", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float PackIntelligence;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Intelligence", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float CoordinationEfficiency;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Intelligence")
    float CommunicationRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Intelligence")
    float TacticalUpdateInterval;

    // Tactical Behavior
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical Behavior")
    TArray<FCombat_TacticalRole> AvailableRoles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical Behavior")
    float FlankingDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical Behavior")
    float MinAttackDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical Behavior")
    float MaxAttackDistance;

    // Pack Communication
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Communication")
    ECombat_CommunicationType PreferredCommunication;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Communication")
    float LastCommunicationTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Communication")
    float CommunicationCooldown;

    // Core AI Functions
    UFUNCTION(BlueprintCallable, Category = "Pack AI")
    void InitializePackAI();

    UFUNCTION(BlueprintCallable, Category = "Pack AI")
    void UpdatePackFormation();

    UFUNCTION(BlueprintCallable, Category = "Pack AI")
    void SetPackTarget(APawn* NewTarget);

    UFUNCTION(BlueprintCallable, Category = "Pack AI")
    void ChangePackState(ECombat_PackState NewState);

    // Tactical Functions
    UFUNCTION(BlueprintCallable, Category = "Tactical AI")
    void CalculateOptimalPositions();

    UFUNCTION(BlueprintCallable, Category = "Tactical AI")
    void AssignTacticalRoles();

    UFUNCTION(BlueprintCallable, Category = "Tactical AI")
    void ExecuteCoordinatedAttack();

    UFUNCTION(BlueprintCallable, Category = "Tactical AI")
    void InitiateFlankingManeuver();

    // Communication Functions
    UFUNCTION(BlueprintCallable, Category = "Communication")
    void SendPackSignal(ECombat_CommunicationType SignalType, const FString& Message);

    UFUNCTION(BlueprintCallable, Category = "Communication")
    void ReceivePackSignal(APawn* Sender, ECombat_CommunicationType SignalType, const FString& Message);

    // Intelligence Functions
    UFUNCTION(BlueprintCallable, Category = "Intelligence")
    void AnalyzeThreatLevel(APawn* Threat);

    UFUNCTION(BlueprintCallable, Category = "Intelligence")
    void AdaptTactics();

    UFUNCTION(BlueprintCallable, Category = "Intelligence")
    bool ShouldRetreat();

    // Pack Management Functions
    UFUNCTION(BlueprintCallable, Category = "Pack Management")
    void AddPackMember(APawn* NewMember);

    UFUNCTION(BlueprintCallable, Category = "Pack Management")
    void RemovePackMember(APawn* MemberToRemove);

    UFUNCTION(BlueprintCallable, Category = "Pack Management")
    void SelectNewLeader();

private:
    // Internal state tracking
    float LastTacticalUpdate;
    TMap<APawn*, FCombat_TacticalRole> MemberRoles;
    FVector LastKnownTargetPosition;
    float ThreatAssessmentTimer;
    
    // Helper functions
    void UpdateMemberPositions();
    void ValidatePackIntegrity();
    FVector CalculateFlankingPosition(APawn* Member, APawn* Target);
    bool IsPositionSafe(const FVector& Position);
    void ExecuteEmergencyRetreat();
};