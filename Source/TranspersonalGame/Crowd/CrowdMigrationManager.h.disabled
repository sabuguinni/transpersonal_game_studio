#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "SharedTypes.h"
#include "CrowdMigrationManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_MigrationRoute
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration")
    TArray<FVector> WayPoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration")
    float RouteSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration")
    int32 MaxHerdSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration")
    EBiomeType StartBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration")
    EBiomeType EndBiome;

    FCrowd_MigrationRoute()
    {
        RouteSpeed = 300.0f;
        MaxHerdSize = 50;
        StartBiome = EBiomeType::Savana;
        EndBiome = EBiomeType::Forest;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_HerdData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd")
    TArray<AActor*> HerdMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd")
    FVector HerdCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd")
    float HerdRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd")
    FVector TargetDestination;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd")
    int32 CurrentWayPointIndex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd")
    bool bIsActive;

    FCrowd_HerdData()
    {
        HerdCenter = FVector::ZeroVector;
        HerdRadius = 1000.0f;
        TargetDestination = FVector::ZeroVector;
        CurrentWayPointIndex = 0;
        bIsActive = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACrowdMigrationManager : public AActor
{
    GENERATED_BODY()

public:
    ACrowdMigrationManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Migration Routes Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration Routes")
    TArray<FCrowd_MigrationRoute> MigrationRoutes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration Routes")
    float SeasonalMigrationTimer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration Routes")
    float MigrationInterval;

    // Active Herds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Active Herds")
    TArray<FCrowd_HerdData> ActiveHerds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Active Herds")
    int32 MaxActiveHerds;

    // Dinosaur Species for Migration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    TArray<FString> MigratorySpecies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    TMap<FString, float> SpeciesMovementSpeed;

    // Performance Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LODDistance1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LODDistance2;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxVisibleHerdMembers;

    // Migration Functions
    UFUNCTION(BlueprintCallable, Category = "Migration")
    void StartSeasonalMigration();

    UFUNCTION(BlueprintCallable, Category = "Migration")
    void CreateMigrationHerd(const FString& Species, const FVector& StartLocation, const FCrowd_MigrationRoute& Route);

    UFUNCTION(BlueprintCallable, Category = "Migration")
    void UpdateHerdMovement(FCrowd_HerdData& HerdData, const FCrowd_MigrationRoute& Route, float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Migration")
    void SpawnHerdMember(const FString& Species, const FVector& Location, FCrowd_HerdData& HerdData);

    // Route Management
    UFUNCTION(BlueprintCallable, Category = "Routes")
    void AddMigrationRoute(const FCrowd_MigrationRoute& NewRoute);

    UFUNCTION(BlueprintCallable, Category = "Routes")
    FCrowd_MigrationRoute GetRouteForBiomes(EBiomeType StartBiome, EBiomeType EndBiome);

    // Performance Optimization
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdateHerdLOD(FCrowd_HerdData& HerdData, const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void CullDistantHerds(const FVector& PlayerLocation);

private:
    void InitializeDefaultRoutes();
    void InitializeSpeciesData();
    FVector CalculateHerdCenter(const FCrowd_HerdData& HerdData);
    bool IsHerdAtDestination(const FCrowd_HerdData& HerdData, const FVector& Destination);
};