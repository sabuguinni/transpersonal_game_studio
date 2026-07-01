#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CrowdHerdMigration.generated.h"

// ============================================================
// Crowd Herd Migration System — Agent #13
// Simulates Brachiosaurus herd migration with waypoint pathing,
// territorial avoidance of T-Rex zones, and raptor hunt dynamics.
// ============================================================

UENUM(BlueprintType)
enum class ECrowd_HerdState : uint8
{
    Grazing         UMETA(DisplayName = "Grazing"),
    Migrating       UMETA(DisplayName = "Migrating"),
    Fleeing         UMETA(DisplayName = "Fleeing"),
    Drinking        UMETA(DisplayName = "Drinking"),
    Resting         UMETA(DisplayName = "Resting"),
    Stampeding      UMETA(DisplayName = "Stampeding")
};

UENUM(BlueprintType)
enum class ECrowd_HerdMemberRole : uint8
{
    Matriarch       UMETA(DisplayName = "Matriarch"),   // Leads migration
    Scout           UMETA(DisplayName = "Scout"),        // Flanks ahead
    Juvenile        UMETA(DisplayName = "Juvenile"),     // Center of herd
    Rear            UMETA(DisplayName = "Rear"),         // Protects back
    Sentry          UMETA(DisplayName = "Sentry")        // Watches flanks
};

USTRUCT(BlueprintType)
struct FCrowd_HerdMember
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    FVector Position;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    ECrowd_HerdMemberRole Role;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    float Health;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    float Speed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    bool bIsAlive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    int32 MemberIndex;

    FCrowd_HerdMember()
        : Position(FVector::ZeroVector)
        , Role(ECrowd_HerdMemberRole::Juvenile)
        , Health(1000.f)
        , Speed(400.f)
        , bIsAlive(true)
        , MemberIndex(0)
    {}
};

USTRUCT(BlueprintType)
struct FCrowd_MigrationWaypoint
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Migration")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Migration")
    float AcceptanceRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Migration")
    float GrazingDuration;   // Seconds herd grazes at this waypoint

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Migration")
    bool bIsWaterSource;

    FCrowd_MigrationWaypoint()
        : Location(FVector::ZeroVector)
        , AcceptanceRadius(500.f)
        , GrazingDuration(60.f)
        , bIsWaterSource(false)
    {}
};

USTRUCT(BlueprintType)
struct FCrowd_TerritoryZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Territory")
    FVector Center;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Territory")
    float Radius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Territory")
    float DangerLevel;   // 0-1, influences avoidance urgency

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Territory")
    FString OwnerSpecies;

    FCrowd_TerritoryZone()
        : Center(FVector::ZeroVector)
        , Radius(3000.f)
        , DangerLevel(1.0f)
        , OwnerSpecies(TEXT("TRex"))
    {}
};

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API ACrowdHerdMigration : public AActor
{
    GENERATED_BODY()

public:
    ACrowdHerdMigration();

    // --- Herd Configuration ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    int32 HerdSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    float HerdCohesionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    float SeparationRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    float MigrationSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    float FleeSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    ECrowd_HerdState CurrentState;

    // --- Migration Path ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Migration")
    TArray<FCrowd_MigrationWaypoint> MigrationPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Migration")
    int32 CurrentWaypointIndex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Migration")
    bool bLoopMigration;

    // --- Territorial Avoidance ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Territory")
    TArray<FCrowd_TerritoryZone> TerritoryZonesToAvoid;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Territory")
    float TerritoryAvoidanceWeight;

    // --- Members ---
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crowd|Herd")
    TArray<FCrowd_HerdMember> HerdMembers;

    // --- Panic State ---
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crowd|Panic")
    float CurrentPanicLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Panic")
    float PanicDecayRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Panic")
    float StampedeThreshold;

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    // --- Core Herd Functions ---
    UFUNCTION(BlueprintCallable, Category = "Crowd|Herd")
    void InitializeHerd(int32 NumMembers);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Herd")
    void UpdateHerdMovement(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Herd")
    void AdvanceToNextWaypoint();

    UFUNCTION(BlueprintCallable, Category = "Crowd|Herd")
    FVector GetHerdCentroid() const;

    // --- Threat Response ---
    UFUNCTION(BlueprintCallable, Category = "Crowd|Panic")
    void ReactToThreat(FVector ThreatLocation, float ThreatRadius, float PanicAmount);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Panic")
    void TriggerStampede(FVector FleeDirection);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Panic")
    bool IsInTerritoryZone(FVector Location) const;

    // --- Flocking Calculations ---
    UFUNCTION(BlueprintCallable, Category = "Crowd|Flocking")
    FVector CalculateSeparation(int32 MemberIndex) const;

    UFUNCTION(BlueprintCallable, Category = "Crowd|Flocking")
    FVector CalculateCohesion(int32 MemberIndex) const;

    UFUNCTION(BlueprintCallable, Category = "Crowd|Flocking")
    FVector CalculateAlignment(int32 MemberIndex) const;

    UFUNCTION(BlueprintCallable, Category = "Crowd|Flocking")
    FVector CalculateAvoidanceVector(FVector FromLocation) const;

    // --- State Transitions ---
    UFUNCTION(BlueprintCallable, Category = "Crowd|State")
    void SetHerdState(ECrowd_HerdState NewState);

    UFUNCTION(BlueprintCallable, Category = "Crowd|State")
    ECrowd_HerdState EvaluateStateTransition() const;

    // --- Member Management ---
    UFUNCTION(BlueprintCallable, Category = "Crowd|Members")
    void KillMember(int32 MemberIndex);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Members")
    int32 GetAliveCount() const;

    UFUNCTION(BlueprintCallable, Category = "Crowd|Members")
    FVector GetMatriarchPosition() const;

private:
    float GrazingTimer;
    FVector CurrentFleeDirection;
    bool bStampedeActive;
};
