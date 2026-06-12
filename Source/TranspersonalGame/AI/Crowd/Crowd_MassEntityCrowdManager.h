#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/WorldSubsystem.h"
#include "MassEntityTypes.h"
#include "MassEntitySubsystem.h"
#include "MassSpawningSubsystem.h"
#include "MassMovementFragments.h"
#include "MassCommonFragments.h"
#include "Crowd_MassEntityCrowdManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_CrowdMemberData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    FVector CurrentLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float MovementSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    int32 BehaviorState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float StateTimer;

    FCrowd_CrowdMemberData()
        : CurrentLocation(FVector::ZeroVector)
        , TargetLocation(FVector::ZeroVector)
        , MovementSpeed(150.0f)
        , BehaviorState(0)
        , StateTimer(0.0f)
    {}
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_CrowdZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Zone")
    FVector CenterLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Zone")
    float Radius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Zone")
    int32 MaxCapacity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Zone")
    int32 CurrentOccupancy;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Zone")
    FString ZoneName;

    FCrowd_CrowdZone()
        : CenterLocation(FVector::ZeroVector)
        , Radius(500.0f)
        , MaxCapacity(50)
        , CurrentOccupancy(0)
        , ZoneName(TEXT("DefaultZone"))
    {}
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCrowd_MassEntityCrowdManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UCrowd_MassEntityCrowdManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    // Crowd Management
    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    void InitializeCrowdSystem();

    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    void SpawnCrowdMembers(int32 Count, FVector SpawnCenter, float SpawnRadius);

    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    void UpdateCrowdBehavior(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    void SetCrowdDensity(float NewDensity);

    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    void AddCrowdZone(const FCrowd_CrowdZone& NewZone);

    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    void RemoveCrowdZone(const FString& ZoneName);

    // Mass Entity Integration
    UFUNCTION(BlueprintCallable, Category = "Mass Entity")
    void CreateMassEntityArchetype();

    UFUNCTION(BlueprintCallable, Category = "Mass Entity")
    void ProcessCrowdEntities();

    UFUNCTION(BlueprintCallable, Category = "Mass Entity")
    int32 GetActiveCrowdMemberCount() const;

    // Crowd Behavior
    UFUNCTION(BlueprintCallable, Category = "Crowd Behavior")
    void SetGlobalCrowdBehavior(int32 BehaviorMode);

    UFUNCTION(BlueprintCallable, Category = "Crowd Behavior")
    void TriggerCrowdEvent(const FString& EventName, FVector EventLocation);

    UFUNCTION(BlueprintCallable, Category = "Crowd Behavior")
    void SetCrowdAvoidanceRadius(float NewRadius);

protected:
    // Mass Entity System
    UPROPERTY()
    class UMassEntitySubsystem* MassEntitySubsystem;

    UPROPERTY()
    class UMassSpawningSubsystem* MassSpawningSubsystem;

    // Crowd Data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Settings")
    TArray<FCrowd_CrowdMemberData> CrowdMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Settings")
    TArray<FCrowd_CrowdZone> CrowdZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Settings")
    float CrowdDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Settings")
    float UpdateFrequency;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Settings")
    int32 MaxCrowdMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Settings")
    float AvoidanceRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Settings")
    int32 GlobalBehaviorMode;

    // Internal State
    UPROPERTY()
    float LastUpdateTime;

    UPROPERTY()
    bool bIsInitialized;

    UPROPERTY()
    FMassEntityHandle CrowdArchetype;

private:
    void InitializeMassEntitySystem();
    void CreateCrowdArchetype();
    void UpdateCrowdMovement(float DeltaTime);
    void ProcessCrowdZones();
    void HandleCrowdCollisions();
    FVector FindNearestCrowdZone(const FVector& Location) const;
    bool IsLocationInCrowdZone(const FVector& Location, const FCrowd_CrowdZone& Zone) const;
};