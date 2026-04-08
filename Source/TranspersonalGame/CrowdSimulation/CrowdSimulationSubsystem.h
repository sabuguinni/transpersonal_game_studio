#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerSubsystem.h"
#include "Engine/World.h"
#include "CrowdSimulationSubsystem.generated.h"

UENUM(BlueprintType)
enum class EHerdType : uint8
{
    SmallHerbivore,     // Compsognathus, Dryosaurus - 50-200 indivíduos
    MediumHerbivore,    // Triceratops, Stegosaurus - 10-30 indivíduos  
    LargeHerbivore,     // Brontosaurus, Diplodocus - 3-8 indivíduos
    PackCarnivore,      // Velociraptors, Deinonychus - 3-12 indivíduos
    SolitaryCarnivore,  // T-Rex, Allosaurus - 1-2 indivíduos
    FlyingCreatures,    // Pteranodons - 20-100 indivíduos
    AquaticLife         // Plesiosaurs - 5-15 indivíduos
};

USTRUCT(BlueprintType)
struct FHerdConfiguration
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EHerdType HerdType = EHerdType::SmallHerbivore;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MinGroupSize = 10;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxGroupSize = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MovementSpeed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CohesionRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SeparationRadius = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AlignmentRadius = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FleeRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float WanderRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsNocturnal = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ActivityLevel = 1.0f; // 0.0 = dormant, 1.0 = fully active
};

/**
 * Sistema de simulação de multidões para ecossistemas pré-históricos
 * Usa Mass Entity Framework para simular até 50.000 agentes simultâneos
 * Foca em comportamento emergente de manadas e predador-presa
 */
UCLASS()
class TRANSPERSONALGAME_API UCrowdSimulationSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Spawning de manadas
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SpawnHerd(EHerdType HerdType, FVector Location, int32 GroupSize = -1);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SpawnEcosystemInRegion(FVector Center, float Radius);

    // Controle de densidade populacional
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SetPopulationDensity(float DensityMultiplier);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    int32 GetActiveEntityCount() const;

    // Eventos de emergência (fuga em massa)
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void TriggerMassFleeEvent(FVector ThreatLocation, float Radius, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void TriggerPredatorHuntEvent(FVector PreyLocation, EHerdType PredatorType);

    // Configuração de comportamento
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SetHerdConfiguration(EHerdType HerdType, const FHerdConfiguration& Config);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    FHerdConfiguration GetHerdConfiguration(EHerdType HerdType) const;

    // Ciclo dia/noite
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SetTimeOfDay(float TimeNormalized); // 0.0 = midnight, 0.5 = noon

    // Debug e visualização
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SetDebugVisualization(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    TArray<FVector> GetHerdCenters(EHerdType HerdType) const;

protected:
    UPROPERTY()
    TObjectPtr<UMassEntitySubsystem> MassEntitySubsystem;

    UPROPERTY()
    TObjectPtr<UMassSpawnerSubsystem> MassSpawnerSubsystem;

    // Configurações por tipo de manada
    UPROPERTY(EditAnywhere, Category = "Configuration")
    TMap<EHerdType, FHerdConfiguration> HerdConfigurations;

    // Estado do sistema
    UPROPERTY()
    float CurrentTimeOfDay = 0.5f; // Noon by default

    UPROPERTY()
    float PopulationDensityMultiplier = 1.0f;

    UPROPERTY()
    bool bDebugVisualizationEnabled = false;

    // Tracking de manadas ativas
    UPROPERTY()
    TMap<EHerdType, TArray<FVector>> ActiveHerdCenters;

private:
    void InitializeDefaultConfigurations();
    void UpdateHerdBehaviors();
    void ProcessTimeOfDayChanges();
    
    // Timers
    FTimerHandle HerdUpdateTimer;
    FTimerHandle EcosystemUpdateTimer;
};