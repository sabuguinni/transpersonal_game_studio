#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MassEntityTypes.h"
#include "MassProcessor.h"
#include "Crowd_SharedTypes.h"
#include "Engine/World.h"
#include "Crowd_MassFlockingSystem.generated.h"

class UMassEntitySubsystem;
class UMassSimulationSubsystem;

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_FlockingParameters
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking")
    float SeparationRadius = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking")
    float AlignmentRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking")
    float CohesionRadius = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking")
    float SeparationWeight = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking")
    float AlignmentWeight = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking")
    float CohesionWeight = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking")
    float MaxSpeed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking")
    float MaxForce = 100.0f;

    FCrowd_FlockingParameters()
    {
        SeparationRadius = 200.0f;
        AlignmentRadius = 500.0f;
        CohesionRadius = 800.0f;
        SeparationWeight = 2.0f;
        AlignmentWeight = 1.0f;
        CohesionWeight = 1.0f;
        MaxSpeed = 300.0f;
        MaxForce = 100.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_FlockingAgent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    FVector Position = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    FVector Velocity = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    FVector Acceleration = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    float MaxSpeed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    float MaxForce = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    int32 FlockID = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    ECrowd_BiomeType BiomeType = ECrowd_BiomeType::Savana;

    FCrowd_FlockingAgent()
    {
        Position = FVector::ZeroVector;
        Velocity = FVector::ZeroVector;
        Acceleration = FVector::ZeroVector;
        MaxSpeed = 300.0f;
        MaxForce = 100.0f;
        FlockID = 0;
        BiomeType = ECrowd_BiomeType::Savana;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCrowd_MassFlockingSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UCrowd_MassFlockingSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Configuração de flocking
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking Configuration")
    FCrowd_FlockingParameters FlockingParameters;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking Configuration")
    int32 MaxFlockingAgents = 10000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking Configuration")
    float FlockingUpdateInterval = 0.05f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking Configuration")
    float NeighborSearchRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking Configuration")
    bool bEnableFlocking = true;

    // Configuração por bioma
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Configuration")
    TMap<ECrowd_BiomeType, FCrowd_FlockingParameters> BiomeFlockingParameters;

    // Sistema de agentes
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime")
    TArray<FCrowd_FlockingAgent> FlockingAgents;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime")
    int32 ActiveFlockingAgents = 0;

    // Subsistemas Mass Entity
    UPROPERTY()
    UMassEntitySubsystem* MassEntitySubsystem = nullptr;

    UPROPERTY()
    UMassSimulationSubsystem* MassSimulationSubsystem = nullptr;

    // Métodos principais
    UFUNCTION(BlueprintCallable, Category = "Flocking System")
    void InitializeFlockingSystem();

    UFUNCTION(BlueprintCallable, Category = "Flocking System")
    void SpawnFlockingAgents(int32 NumAgents, ECrowd_BiomeType BiomeType, const FVector& SpawnCenter, float SpawnRadius);

    UFUNCTION(BlueprintCallable, Category = "Flocking System")
    void UpdateFlockingBehavior(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Flocking System")
    void ClearAllFlockingAgents();

    // Comportamentos de flocking
    UFUNCTION(BlueprintCallable, Category = "Flocking Behavior")
    FVector CalculateSeparation(const FCrowd_FlockingAgent& Agent, const TArray<FCrowd_FlockingAgent>& Neighbors) const;

    UFUNCTION(BlueprintCallable, Category = "Flocking Behavior")
    FVector CalculateAlignment(const FCrowd_FlockingAgent& Agent, const TArray<FCrowd_FlockingAgent>& Neighbors) const;

    UFUNCTION(BlueprintCallable, Category = "Flocking Behavior")
    FVector CalculateCohesion(const FCrowd_FlockingAgent& Agent, const TArray<FCrowd_FlockingAgent>& Neighbors) const;

    UFUNCTION(BlueprintCallable, Category = "Flocking Behavior")
    TArray<FCrowd_FlockingAgent> FindNeighbors(const FCrowd_FlockingAgent& Agent, float SearchRadius) const;

    // Configuração e utilidades
    UFUNCTION(BlueprintCallable, Category = "Configuration")
    void SetFlockingParameters(const FCrowd_FlockingParameters& NewParameters);

    UFUNCTION(BlueprintCallable, Category = "Configuration")
    void SetBiomeFlockingParameters(ECrowd_BiomeType BiomeType, const FCrowd_FlockingParameters& Parameters);

    UFUNCTION(BlueprintCallable, Category = "Configuration")
    FCrowd_FlockingParameters GetBiomeFlockingParameters(ECrowd_BiomeType BiomeType) const;

    // Informações do sistema
    UFUNCTION(BlueprintCallable, Category = "Information")
    int32 GetActiveFlockingAgents() const;

    UFUNCTION(BlueprintCallable, Category = "Information")
    float GetAverageFlockSpeed() const;

    UFUNCTION(BlueprintCallable, Category = "Information")
    FVector GetFlockCenter(int32 FlockID) const;

private:
    // Variáveis internas
    float FlockingUpdateTimer = 0.0f;
    TMap<int32, TArray<int32>> FlockGroups;
    int32 NextFlockID = 1;

    // Métodos internos
    void InitializeBiomeFlockingParameters();
    void ProcessFlockingAgent(FCrowd_FlockingAgent& Agent, float DeltaTime);
    FVector ApplyFlockingForces(const FCrowd_FlockingAgent& Agent, const TArray<FCrowd_FlockingAgent>& Neighbors) const;
    FVector LimitForce(const FVector& Force, float MaxForce) const;
    void UpdateAgentPosition(FCrowd_FlockingAgent& Agent, float DeltaTime);
    void AssignAgentToFlock(int32 AgentIndex);
    void UpdateFlockGroups();
    FVector GetBiomeCenter(ECrowd_BiomeType BiomeType) const;
    bool IsValidBiomePosition(const FVector& Position, ECrowd_BiomeType BiomeType) const;
};