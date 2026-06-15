#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/SceneComponent.h"
#include "Crowd_MigrationManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_MigrationRoute
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration")
    FVector StartLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration")
    FVector EndLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration")
    float RouteDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration")
    FString SpeciesType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration")
    int32 MaxGroupSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration")
    float MigrationSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration")
    bool bIsActive;

    FCrowd_MigrationRoute()
    {
        StartLocation = FVector::ZeroVector;
        EndLocation = FVector::ZeroVector;
        RouteDistance = 0.0f;
        SpeciesType = TEXT("Herbivore");
        MaxGroupSize = 20;
        MigrationSpeed = 300.0f;
        bIsActive = true;
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
    int32 RouteIndex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration")
    float GroupCohesion;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration")
    bool bReachedDestination;

    FCrowd_MigrationGroup()
    {
        CurrentDestination = FVector::ZeroVector;
        RouteIndex = 0;
        GroupCohesion = 0.8f;
        bReachedDestination = false;
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

public:
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration Routes")
    TArray<FCrowd_MigrationRoute> MigrationRoutes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration Groups")
    TArray<FCrowd_MigrationGroup> ActiveMigrationGroups;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration Settings")
    float RouteUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration Settings")
    float GroupFormationRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration Settings")
    int32 MaxSimultaneousGroups;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration Settings")
    bool bEnableSeasonalMigration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration Settings")
    float SeasonalMigrationChance;

    UFUNCTION(BlueprintCallable, Category = "Migration")
    void InitializeMigrationRoutes();

    UFUNCTION(BlueprintCallable, Category = "Migration")
    void StartMigration(const FString& SpeciesType, int32 GroupSize);

    UFUNCTION(BlueprintCallable, Category = "Migration")
    void UpdateMigrationGroups(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Migration")
    void CreateMigrationGroup(const FCrowd_MigrationRoute& Route, int32 GroupSize);

    UFUNCTION(BlueprintCallable, Category = "Migration")
    void MoveMigrationGroup(FCrowd_MigrationGroup& Group, float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Migration")
    bool IsGroupAtDestination(const FCrowd_MigrationGroup& Group, float Tolerance = 500.0f);

    UFUNCTION(BlueprintCallable, Category = "Migration")
    void AdvanceGroupToNextWaypoint(FCrowd_MigrationGroup& Group);

    UFUNCTION(BlueprintCallable, Category = "Migration")
    void HandleSeasonalMigration();

    UFUNCTION(BlueprintCallable, Category = "Migration")
    TArray<FVector> GenerateWaypointsForRoute(const FCrowd_MigrationRoute& Route);

    UFUNCTION(BlueprintCallable, Category = "Migration")
    void CleanupCompletedMigrations();

private:
    float LastRouteUpdateTime;
    float LastSeasonalCheckTime;

    UFUNCTION()
    void OnMigrationGroupReachedDestination(FCrowd_MigrationGroup& Group);
};

#include "Crowd_MigrationManager.generated.h"