#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CrowdMigrationPath.generated.h"

USTRUCT(BlueprintType)
struct FCrowd_WaypointData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float Radius = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    int32 MaxAgentsAtWaypoint = 20;
};

UENUM(BlueprintType)
enum class ECrowd_HerdType : uint8
{
    Brachiosaurus   UMETA(DisplayName = "Brachiosaurus Herd"),
    Stegosaurus     UMETA(DisplayName = "Stegosaurus Herd"),
    Raptor          UMETA(DisplayName = "Raptor Pack"),
    Mixed           UMETA(DisplayName = "Mixed Herd")
};

/**
 * ACrowdMigrationPath — defines a migration corridor for prehistoric herds.
 * Waypoints are placed in the world; the CrowdSimulationManager reads these
 * to drive Mass AI entity movement along the path.
 */
UCLASS(ClassGroup = (Crowd), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API ACrowdMigrationPath : public AActor
{
    GENERATED_BODY()

public:
    ACrowdMigrationPath();

    /** Ordered list of waypoints defining the migration path */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Path")
    TArray<FCrowd_WaypointData> Waypoints;

    /** Type of herd that uses this path */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Path")
    ECrowd_HerdType HerdType = ECrowd_HerdType::Brachiosaurus;

    /** How many agents travel this path simultaneously */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Path")
    int32 AgentCount = 8;

    /** Speed multiplier for agents on this path (1.0 = normal walk) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Path")
    float SpeedMultiplier = 1.0f;

    /** Whether the path loops back to the start */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Path")
    bool bLooping = true;

    /** Returns the next waypoint index after the given index */
    UFUNCTION(BlueprintCallable, Category = "Crowd|Path")
    int32 GetNextWaypointIndex(int32 CurrentIndex) const;

    /** Returns world location of waypoint at index */
    UFUNCTION(BlueprintCallable, Category = "Crowd|Path")
    FVector GetWaypointLocation(int32 Index) const;

    /** Returns total path length in cm */
    UFUNCTION(BlueprintCallable, Category = "Crowd|Path")
    float GetTotalPathLength() const;

protected:
    virtual void BeginPlay() override;

#if WITH_EDITOR
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
