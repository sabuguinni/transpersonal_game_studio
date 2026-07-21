#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "SharedTypes.h"
#include "NPC_DinosaurMigrationSystem.generated.h"

UENUM(BlueprintType)
enum class ENPC_MigrationSeason : uint8
{
    Spring      UMETA(DisplayName = "Spring"),
    Summer      UMETA(DisplayName = "Summer"),
    Autumn      UMETA(DisplayName = "Autumn"),
    Winter      UMETA(DisplayName = "Winter")
};

UENUM(BlueprintType)
enum class ENPC_MigrationState : uint8
{
    Stationary      UMETA(DisplayName = "Stationary"),
    Preparing       UMETA(DisplayName = "Preparing"),
    Migrating       UMETA(DisplayName = "Migrating"),
    Arriving        UMETA(DisplayName = "Arriving"),
    Settled         UMETA(DisplayName = "Settled")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_MigrationRoute
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration")
    FString RouteName = TEXT("Default Route");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration")
    TArray<FVector> Waypoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration")
    ENPC_MigrationSeason StartSeason = ENPC_MigrationSeason::Spring;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration")
    ENPC_MigrationSeason EndSeason = ENPC_MigrationSeason::Autumn;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration")
    float TotalDistance = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration")
    float EstimatedDuration = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration")
    bool bIsCircularRoute = true;

    FNPC_MigrationRoute()
    {
        RouteName = TEXT("Default Route");
        StartSeason = ENPC_MigrationSeason::Spring;
        EndSeason = ENPC_MigrationSeason::Autumn;
        TotalDistance = 0.0f;
        EstimatedDuration = 0.0f;
        bIsCircularRoute = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_MigrationProgress
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration")
    int32 CurrentWaypointIndex = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration")
    float DistanceTraveled = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration")
    float MigrationStartTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration")
    ENPC_MigrationState CurrentState = ENPC_MigrationState::Stationary;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration")
    FVector LastKnownPosition = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration")
    bool bIsMigrationActive = false;

    FNPC_MigrationProgress()
    {
        CurrentWaypointIndex = 0;
        DistanceTraveled = 0.0f;
        MigrationStartTime = 0.0f;
        CurrentState = ENPC_MigrationState::Stationary;
        LastKnownPosition = FVector::ZeroVector;
        bIsMigrationActive = false;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPC_DinosaurMigrationSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_DinosaurMigrationSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Migration Control
    UFUNCTION(BlueprintCallable, Category = "Migration")
    void StartMigration();

    UFUNCTION(BlueprintCallable, Category = "Migration")
    void StopMigration();

    UFUNCTION(BlueprintCallable, Category = "Migration")
    void PauseMigration();

    UFUNCTION(BlueprintCallable, Category = "Migration")
    void ResumeMigration();

    // Route Management
    UFUNCTION(BlueprintCallable, Category = "Migration")
    void SetMigrationRoute(const FNPC_MigrationRoute& NewRoute);

    UFUNCTION(BlueprintCallable, Category = "Migration")
    void AddWaypoint(const FVector& Waypoint);

    UFUNCTION(BlueprintCallable, Category = "Migration")
    void RemoveWaypoint(int32 WaypointIndex);

    UFUNCTION(BlueprintCallable, Category = "Migration")
    FVector GetCurrentTarget() const;

    UFUNCTION(BlueprintCallable, Category = "Migration")
    FVector GetNextWaypoint() const;

    // Migration Status
    UFUNCTION(BlueprintCallable, Category = "Migration")
    bool IsMigrating() const;

    UFUNCTION(BlueprintCallable, Category = "Migration")
    float GetMigrationProgress() const;

    UFUNCTION(BlueprintCallable, Category = "Migration")
    ENPC_MigrationState GetMigrationState() const;

    UFUNCTION(BlueprintCallable, Category = "Migration")
    float GetDistanceToNextWaypoint() const;

    // Seasonal Migration
    UFUNCTION(BlueprintCallable, Category = "Migration")
    void CheckSeasonalMigration();

    UFUNCTION(BlueprintCallable, Category = "Migration")
    bool ShouldStartSeasonalMigration() const;

    UFUNCTION(BlueprintCallable, Category = "Migration")
    ENPC_MigrationSeason GetCurrentSeason() const;

    // Group Migration
    UFUNCTION(BlueprintCallable, Category = "Migration")
    void JoinMigrationGroup(UNPC_DinosaurMigrationSystem* LeaderMigration);

    UFUNCTION(BlueprintCallable, Category = "Migration")
    void LeaveMigrationGroup();

    UFUNCTION(BlueprintCallable, Category = "Migration")
    bool IsGroupLeader() const;

    UFUNCTION(BlueprintCallable, Category = "Migration")
    void UpdateGroupFormation();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration", meta = (AllowPrivateAccess = "true"))
    FNPC_MigrationRoute MigrationRoute;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration", meta = (AllowPrivateAccess = "true"))
    FNPC_MigrationProgress MigrationProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration", meta = (AllowPrivateAccess = "true"))
    float MigrationSpeed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration", meta = (AllowPrivateAccess = "true"))
    float WaypointReachThreshold = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration", meta = (AllowPrivateAccess = "true"))
    bool bUseSeasonalMigration = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration", meta = (AllowPrivateAccess = "true"))
    float SeasonDuration = 300.0f; // 5 minutes per season for testing

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration", meta = (AllowPrivateAccess = "true"))
    bool bIsGroupMigration = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration", meta = (AllowPrivateAccess = "true"))
    UNPC_DinosaurMigrationSystem* GroupLeader = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration", meta = (AllowPrivateAccess = "true"))
    TArray<UNPC_DinosaurMigrationSystem*> GroupMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration", meta = (AllowPrivateAccess = "true"))
    FVector GroupFormationOffset = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration", meta = (AllowPrivateAccess = "true"))
    float GroupCohesionRadius = 1000.0f;

private:
    void UpdateMigrationMovement(float DeltaTime);
    void AdvanceToNextWaypoint();
    void CalculateRouteDistance();
    ENPC_MigrationSeason CalculateCurrentSeason() const;
    void UpdateMigrationState();
    void HandleGroupMigration();
    FVector CalculateGroupFormationPosition() const;
};