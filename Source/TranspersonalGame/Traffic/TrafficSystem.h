// TrafficSystem.h
// Sistema de tráfego consciente que responde ao estado espiritual do ambiente

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SplineComponent.h"
#include "../Core/ConsciousnessSystem.h"
#include "TrafficSystem.generated.h"

UENUM(BlueprintType)
enum class ETrafficFlowState : uint8
{
    Smooth,        // Fluxo suave e harmônico
    Congested,     // Congestionado
    Chaotic,       // Caótico e errático
    Transcendent,  // Fluxo transcendente - veículos em perfeita sincronia
    Stagnant       // Estagnado - pouco ou nenhum movimento
};

UENUM(BlueprintType)
enum class EVehicleType : uint8
{
    Car,
    Bus,
    Truck,
    Motorcycle,
    Bicycle,
    Spiritual      // Veículo especial que aparece em estados elevados
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
    FRotator Rotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EVehicleType VehicleType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Speed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ConsciousnessLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 CurrentLaneIndex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DistanceAlongSpline;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsActive;

    FTrafficVehicle()
    {
        Position = FVector::ZeroVector;
        Velocity = FVector::ZeroVector;
        Rotation = FRotator::ZeroRotator;
        VehicleType = EVehicleType::Car;
        Speed = 0.0f;
        MaxSpeed = 600.0f; // 60 km/h em unidades Unreal
        ConsciousnessLevel = 0.5f;
        CurrentLaneIndex = 0;
        DistanceAlongSpline = 0.0f;
        bIsActive = true;
    }
};

USTRUCT(BlueprintType)
struct FTrafficLane
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    class USplineComponent* SplinePath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FTrafficVehicle> Vehicles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LaneWidth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsReversed;

    FTrafficLane()
    {
        SplinePath = nullptr;
        LaneWidth = 400.0f; // 4 metros
        bIsReversed = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ATrafficSystem : public AActor
{
    GENERATED_BODY()

public:
    ATrafficSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Configurações do sistema de tráfego
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic Settings")
    int32 MaxVehiclesPerLane;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic Settings")
    float VehicleSpawnRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic Settings")
    float MinFollowDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic Settings")
    float ConsciousnessInfluenceRange;

    // Pistas de tráfego
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic Lanes")
    TArray<FTrafficLane> TrafficLanes;

    // Estado atual do tráfego
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Traffic State")
    ETrafficFlowState CurrentTrafficState;

    // Referência ao sistema de consciência
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Systems")
    class UConsciousnessComponent* ConsciousnessSystem;

    // Funções públicas
    UFUNCTION(BlueprintCallable, Category = "Traffic Management")
    void AddTrafficLane(USplineComponent* SplinePath, bool bReversed = false);

    UFUNCTION(BlueprintCallable, Category = "Traffic Management")
    void SpawnVehicle(int32 LaneIndex, EVehicleType VehicleType);

    UFUNCTION(BlueprintCallable, Category = "Traffic Management")
    void UpdateTrafficFlow(float PlayerConsciousnessLevel);

    UFUNCTION(BlueprintCallable, Category = "Traffic Management")
    void SetTrafficState(ETrafficFlowState NewState);

    UFUNCTION(BlueprintCallable, Category = "Traffic Management")
    void ClearAllTraffic();

    // Eventos Blueprint
    UFUNCTION(BlueprintImplementableEvent, Category = "Traffic Events")
    void OnTrafficStateChanged(ETrafficFlowState NewState);

    UFUNCTION(BlueprintImplementableEvent, Category = "Traffic Events")
    void OnVehicleSpawned(const FTrafficVehicle& NewVehicle, int32 LaneIndex);

    UFUNCTION(BlueprintImplementableEvent, Category = "Traffic Events")
    void OnSpiritualVehicleAppeared();

private:
    // Funções internas
    void UpdateVehicleMovement(float DeltaTime);
    void ApplyConsciousnessInfluence();
    void HandleVehicleCollisionAvoidance(FTrafficVehicle& Vehicle, const FTrafficLane& Lane);
    void UpdateVehicleOnSpline(FTrafficVehicle& Vehicle, const FTrafficLane& Lane, float DeltaTime);
    bool ShouldSpawnVehicle(int32 LaneIndex);
    EVehicleType GetRandomVehicleType();
    void RemoveInactiveVehicles();

    // Configurações de comportamento
    UPROPERTY(EditAnywhere, Category = "Behavior")
    float AccelerationRate;

    UPROPERTY(EditAnywhere, Category = "Behavior")
    float DecelerationRate;

    UPROPERTY(EditAnywhere, Category = "Behavior")
    float LaneChangeChance;

    // Timer para spawn de veículos
    float LastSpawnTime;
    float SpawnInterval;

    // Timer para otimização
    float LastUpdateTime;
    float UpdateInterval;
};