#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SplineComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/DataTable.h"
#include "TrafficManager.generated.h"

USTRUCT(BlueprintType)
struct FVehicleData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UStaticMesh* VehicleMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Acceleration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BrakingDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString VehicleType;

    FVehicleData()
    {
        VehicleMesh = nullptr;
        MaxSpeed = 800.0f;
        Acceleration = 200.0f;
        BrakingDistance = 300.0f;
        VehicleType = "Car";
    }
};

USTRUCT(BlueprintType)
struct FTrafficVehicle
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Position;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Velocity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CurrentSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TargetSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 CurrentLane;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DistanceOnSpline;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 VehicleTypeIndex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsChangingLanes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 TargetLane;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LaneChangeProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float StressLevel;

    FTrafficVehicle()
    {
        Position = FVector::ZeroVector;
        Velocity = FVector::ZeroVector;
        CurrentSpeed = 0.0f;
        TargetSpeed = 600.0f;
        CurrentLane = 0;
        DistanceOnSpline = 0.0f;
        VehicleTypeIndex = 0;
        bIsChangingLanes = false;
        TargetLane = 0;
        LaneChangeProgress = 0.0f;
        StressLevel = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FTrafficLight
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Position;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DistanceOnSpline;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsGreen;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Timer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float GreenDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float RedDuration;

    FTrafficLight()
    {
        Position = FVector::ZeroVector;
        DistanceOnSpline = 0.0f;
        bIsGreen = true;
        Timer = 0.0f;
        GreenDuration = 30.0f;
        RedDuration = 20.0f;
    }
};

UCLASS()
class TRANSPERSONALGAME_API ATrafficManager : public AActor
{
    GENERATED_BODY()

public:
    ATrafficManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Traffic Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic Settings")
    int32 MaxVehicles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic Settings")
    float SpawnRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic Settings")
    int32 NumberOfLanes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic Settings")
    float LaneWidth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic Settings")
    UDataTable* VehicleDataTable;

    // Behavior Parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float FollowingDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float LaneChangeDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float LaneChangeSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float AggressionLevel;

    // Consciousness Integration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness")
    float ConsciousnessInfluenceRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness")
    float StressInfluenceRadius;

    // Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USplineComponent* TrafficSpline;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TArray<UInstancedStaticMeshComponent*> VehicleMeshComponents;

    // Traffic Data
    UPROPERTY(BlueprintReadOnly, Category = "Traffic Data")
    TArray<FTrafficVehicle> TrafficVehicles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic Lights")
    TArray<FTrafficLight> TrafficLights;

    // Public Functions
    UFUNCTION(BlueprintCallable, Category = "Traffic Management")
    void SpawnVehicle(int32 Lane = -1);

    UFUNCTION(BlueprintCallable, Category = "Traffic Management")
    void RemoveVehicle(int32 VehicleIndex);

    UFUNCTION(BlueprintCallable, Category = "Traffic Management")
    void AddTrafficLight(FVector Position, float GreenTime = 30.0f, float RedTime = 20.0f);

    UFUNCTION(BlueprintCallable, Category = "Traffic Management")
    void TriggerTrafficIncident(FVector Location, float Radius, float Duration);

    UFUNCTION(BlueprintCallable, Category = "Consciousness")
    void UpdateConsciousnessInfluence(FVector Location, float Influence, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Traffic Management")
    void SetTrafficDensity(float Density);

private:
    // Internal Functions
    void UpdateVehicleMovement(float DeltaTime);
    void UpdateTrafficLights(float DeltaTime);
    void UpdateInstancedMeshes();
    
    void HandleVehicleBehavior(FTrafficVehicle& Vehicle, int32 VehicleIndex, float DeltaTime);
    void HandleLaneChanging(FTrafficVehicle& Vehicle, float DeltaTime);
    
    FVector GetLanePosition(float DistanceOnSpline, int32 Lane);
    FVector GetSplineDirection(float DistanceOnSpline);
    
    bool CanChangeLanes(const FTrafficVehicle& Vehicle, int32 TargetLane);
    FTrafficVehicle* GetVehicleAhead(const FTrafficVehicle& Vehicle, int32 Lane, float MaxDistance);
    FTrafficLight* GetNextTrafficLight(const FTrafficVehicle& Vehicle);
    
    float CalculateDesiredSpeed(const FTrafficVehicle& Vehicle);
    float CalculateFollowingDistance(const FTrafficVehicle& Vehicle);
    
    // Performance optimization
    float LastSpawnTime;
    int32 UpdateBatchSize;
    int32 CurrentUpdateIndex;
    
    // Traffic incidents
    struct FTrafficIncident
    {
        FVector Location;
        float Radius;
        float RemainingTime;
    };
    TArray<FTrafficIncident> ActiveIncidents;
    
    void UpdateTrafficIncidents(float DeltaTime);
    bool IsAffectedByIncident(const FVector& Position);
};