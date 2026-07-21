#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CrowdWaypointSystem.generated.h"

UENUM(BlueprintType)
enum class ECrowd_WaypointType : uint8
{
    WaterHole       UMETA(DisplayName = "Water Hole"),
    ForestEdge      UMETA(DisplayName = "Forest Edge"),
    RockShelter     UMETA(DisplayName = "Rock Shelter"),
    Clearing        UMETA(DisplayName = "Clearing"),
    Hilltop         UMETA(DisplayName = "Hilltop"),
    RiverBank       UMETA(DisplayName = "River Bank"),
    CaveEntrance    UMETA(DisplayName = "Cave Entrance"),
    HuntingGround   UMETA(DisplayName = "Hunting Ground"),
};

USTRUCT(BlueprintType)
struct FCrowd_WaypointData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    ECrowd_WaypointType WaypointType = ECrowd_WaypointType::Clearing;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float Radius = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    int32 MaxOccupants = 10;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float DangerLevel = 0.0f;
};

UCLASS(ClassGroup = (Crowd), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API ACrowd_WaypointActor : public AActor
{
    GENERATED_BODY()

public:
    ACrowd_WaypointActor();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Waypoint")
    FCrowd_WaypointData WaypointData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Waypoint")
    TArray<AActor*> CurrentOccupants;

    UFUNCTION(BlueprintCallable, Category = "Crowd|Waypoint")
    bool IsAvailable() const;

    UFUNCTION(BlueprintCallable, Category = "Crowd|Waypoint")
    bool RegisterOccupant(AActor* Occupant);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Waypoint")
    void UnregisterOccupant(AActor* Occupant);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Waypoint")
    float GetOccupancyRatio() const;

protected:
    virtual void BeginPlay() override;
};
