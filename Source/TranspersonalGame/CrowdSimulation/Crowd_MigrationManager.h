#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "SharedTypes.h"
#include "Crowd_MigrationManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_MigrationRoute
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration")
    TArray<FVector> WayPoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration")
    float RouteLength;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration")
    EBiomeType StartBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration")
    EBiomeType EndBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration")
    int32 MaxGroupSize;

    FCrowd_MigrationRoute()
    {
        RouteLength = 0.0f;
        StartBiome = EBiomeType::Savanna;
        EndBiome = EBiomeType::Forest;
        MaxGroupSize = 50;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_MigrationGroup
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration")
    TArray<AActor*> GroupMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration")
    FVector CurrentDestination;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration")
    int32 CurrentWayPointIndex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration")
    float MovementSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration")
    bool bIsActive;

    FCrowd_MigrationGroup()
    {
        CurrentDestination = FVector::ZeroVector;
        CurrentWayPointIndex = 0;
        MovementSpeed = 300.0f;
        bIsActive = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACrowd_MigrationManager : public AActor
{
    GENERATED_BODY()

public:
    ACrowd_MigrationManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration Routes")
    TArray<FCrowd_MigrationRoute> MigrationRoutes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration Groups")
    TArray<FCrowd_MigrationGroup> ActiveMigrationGroups;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration Settings")
    float SeasonalMigrationInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration Settings")
    int32 MaxSimultaneousMigrations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration Settings")
    float GroupFormationRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration Settings")
    bool bEnableSeasonalMigration;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Migration")
    void StartMigration(const FCrowd_MigrationRoute& Route, const TArray<AActor*>& Participants);

    UFUNCTION(BlueprintCallable, Category = "Migration")
    void StopMigration(int32 GroupIndex);

    UFUNCTION(BlueprintCallable, Category = "Migration")
    void AddMigrationRoute(const FCrowd_MigrationRoute& NewRoute);

    UFUNCTION(BlueprintCallable, Category = "Migration")
    void UpdateGroupMovement(FCrowd_MigrationGroup& Group, float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Migration")
    bool IsRouteBlocked(const FCrowd_MigrationRoute& Route);

    UFUNCTION(BlueprintCallable, Category = "Migration")
    void TriggerSeasonalMigration();

    UFUNCTION(BlueprintCallable, Category = "Migration")
    TArray<AActor*> FindNearbyActors(FVector Location, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Migration")
    void FormMigrationGroup(const TArray<AActor*>& Actors, FVector RallyPoint);

    UFUNCTION(BlueprintCallable, Category = "Migration")
    void HandleGroupArrival(FCrowd_MigrationGroup& Group);

    UFUNCTION(BlueprintCallable, Category = "Migration")
    void OptimizeMigrationPaths();

private:
    float LastSeasonalCheck;
    int32 ActiveMigrationCount;

    void UpdateSeasonalMigrations(float DeltaTime);
    void CheckGroupProgress();
    void HandleGroupCollisions(FCrowd_MigrationGroup& Group);
    FVector CalculateGroupCenterOfMass(const FCrowd_MigrationGroup& Group);
};