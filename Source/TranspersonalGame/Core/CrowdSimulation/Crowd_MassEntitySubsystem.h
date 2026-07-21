#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Engine/World.h"
#include "Engine/TargetPoint.h"
#include "Components/SphereComponent.h"
#include "Crowd_MassEntitySubsystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_EntityData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Crowd")
    FVector Location;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd")
    FVector Velocity;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd")
    float Speed;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd")
    int32 LODLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd")
    bool bIsActive;

    FCrowd_EntityData()
    {
        Location = FVector::ZeroVector;
        Velocity = FVector::ZeroVector;
        Speed = 100.0f;
        LODLevel = 0;
        bIsActive = true;
    }
};

UENUM(BlueprintType)
enum class ECrowd_LODLevel : uint8
{
    High = 0    UMETA(DisplayName = "High Detail"),
    Medium = 1  UMETA(DisplayName = "Medium Detail"),
    Low = 2     UMETA(DisplayName = "Low Detail"),
    Culled = 3  UMETA(DisplayName = "Culled")
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UCrowd_MassEntitySubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual void Tick(float DeltaTime) override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SpawnCrowdEntities(int32 Count, const FVector& Center, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void UpdateCrowdLOD(const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SetCrowdBehavior(int32 EntityIndex, const FVector& TargetLocation);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    int32 GetActiveCrowdCount() const;

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void CullDistantEntities(const FVector& PlayerLocation, float CullDistance);

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Crowd")
    TArray<FCrowd_EntityData> CrowdEntities;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd")
    TArray<ATargetPoint*> Waypoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxCrowdEntities;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LODUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float HighLODDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MediumLODDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float CullDistance;

private:
    float LODUpdateTimer;
    void UpdateEntityLOD(FCrowd_EntityData& Entity, const FVector& PlayerLocation);
    void ProcessCrowdMovement(float DeltaTime);
    FVector CalculateFlockingBehavior(const FCrowd_EntityData& Entity, int32 EntityIndex);
};