#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MassEntitySubsystem.h"
#include "MassCommonFragments.h"
#include "JurassicCrowdTypes.h"
#include "Engine/World.h"
#include "AdvancedCrowdManager.generated.h"

/**
 * Configuração de densidade dinâmica baseada na proximidade do jogador
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDynamicDensityConfig
{
    GENERATED_BODY()

    // Densidade máxima em diferentes zonas de distância
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Density")
    float HighDetailRadius = 2000.0f; // Zona de alta densidade (próximo ao jogador)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Density")
    float MediumDetailRadius = 5000.0f; // Zona de densidade média

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Density")
    float LowDetailRadius = 15000.0f; // Zona de baixa densidade

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Density")
    float CullingRadius = 25000.0f; // Além desta distância, agentes são removidos

    // Multiplicadores de densidade por zona
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Density")
    float HighDetailDensityMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Density")
    float MediumDetailDensityMultiplier = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Density")
    float LowDetailDensityMultiplier = 0.2f;

    // Limites absolutos
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Density")
    int32 MaxAgentsHighDetail = 5000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Density")
    int32 MaxAgentsMediumDetail = 15000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Density")
    int32 MaxAgentsLowDetail = 30000;
};

/**
 * Configuração de eventos emergentes
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEmergentEventConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emergent Events")
    FString EventName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emergent Events")
    float EventProbabilityPerHour = 0.1f; // Probabilidade por hora de jogo

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emergent Events")
    float EventDuration = 300.0f; // Duração em segundos

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emergent Events")
    float EventRadius = 5000.0f; // Raio de influência

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emergent Events")
    TArray<EJurassicAgentType> AffectedSpecies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emergent Events")
    EJurassicBehaviorState ForcedBehaviorState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emergent Events")
    bool bRequiresPlayerProximity = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emergent Events")
    float MinPlayerDistance = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emergent Events")
    float MaxPlayerDistance = 10000.0f;
};

/**
 * Configuração de migração sazonal
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FSeasonalMigrationConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration")
    TArray<EJurassicAgentType> MigratingSpecies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration")
    TArray<FVector> MigrationWaypoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration")
    float MigrationSpeed = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration")
    float MigrationRadius = 2000.0f; // Raio em torno dos waypoints

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration")
    float SeasonDuration = 1200.0f; // Duração de uma "estação" em segundos

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration")
    bool bCyclicalMigration = true;
};

/**
 * Sistema de comunicação entre espécies
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FSpeciesCommunicationRule
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Communication")
    EJurassicAgentType SourceSpecies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Communication")
    EJurassicAgentType TargetSpecies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Communication")
    EJurassicBehaviorState TriggerState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Communication")
    EJurassicBehaviorState ResponseState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Communication")
    float CommunicationRange = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Communication")
    float ResponseProbability = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Communication")
    float ResponseDelay = 2.0f; // Delay antes da resposta
};

/**
 * Evento emergente ativo
 */
USTRUCT()
struct TRANSPERSONALGAME_API FActiveEmergentEvent
{
    GENERATED_BODY()

    FString EventName;
    FVector EventLocation;
    float EventRadius;
    float RemainingDuration;
    EJurassicBehaviorState ForcedState;
    TArray<EJurassicAgentType> AffectedSpecies;
    TArray<FMassEntityHandle> AffectedAgents;
};

/**
 * Gerenciador avançado de crowd simulation para criar um ecossistema verdadeiramente vivo
 * Implementa densidade dinâmica, eventos emergentes, migração sazonal e comunicação entre espécies
 */
UCLASS()
class TRANSPERSONALGAME_API UAdvancedCrowdManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual void Tick(float DeltaTime) override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    // === DENSIDADE DINÂMICA ===
    UFUNCTION(BlueprintCallable, Category = "Advanced Crowd")
    void UpdateDynamicDensity(const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Advanced Crowd")
    void SetDynamicDensityConfig(const FDynamicDensityConfig& NewConfig);

    UFUNCTION(BlueprintCallable, Category = "Advanced Crowd")
    int32 GetOptimalAgentCountForLocation(const FVector& Location) const;

    // === EVENTOS EMERGENTES ===
    UFUNCTION(BlueprintCallable, Category = "Advanced Crowd")
    void RegisterEmergentEvent(const FEmergentEventConfig& EventConfig);

    UFUNCTION(BlueprintCallable, Category = "Advanced Crowd")
    bool TriggerEmergentEvent(const FString& EventName, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Advanced Crowd")
    void StopEmergentEvent(const FString& EventName);

    UFUNCTION(BlueprintCallable, Category = "Advanced Crowd")
    TArray<FString> GetActiveEmergentEvents() const;

    // === MIGRAÇÃO SAZONAL ===
    UFUNCTION(BlueprintCallable, Category = "Advanced Crowd")
    void StartSeasonalMigration(const FSeasonalMigrationConfig& MigrationConfig);

    UFUNCTION(BlueprintCallable, Category = "Advanced Crowd")
    void UpdateMigrationProgress(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Advanced Crowd")
    bool IsSpeciesMigrating(EJurassicAgentType Species) const;

    // === COMUNICAÇÃO ENTRE ESPÉCIES ===
    UFUNCTION(BlueprintCallable, Category = "Advanced Crowd")
    void RegisterCommunicationRule(const FSpeciesCommunicationRule& Rule);

    UFUNCTION(BlueprintCallable, Category = "Advanced Crowd")
    void ProcessSpeciesCommunication(FMassEntityHandle SourceEntity, 
                                   EJurassicBehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "Advanced Crowd")
    void PropagateAlertSignal(const FVector& AlertLocation, 
                            EJurassicAgentType SourceSpecies,
                            float AlertRadius, 
                            float AlertIntensity);

    // === MONITORAMENTO E DEBUG ===
    UFUNCTION(BlueprintCallable, Category = "Advanced Crowd")
    void EnableDebugVisualization(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Advanced Crowd")
    FString GetCrowdSimulationStats() const;

    UFUNCTION(BlueprintCallable, Category = "Advanced Crowd")
    TArray<FVector> GetHerdCentersInRadius(const FVector& Center, float Radius) const;

    // === PERFORMANCE OPTIMIZATION ===
    UFUNCTION(BlueprintCallable, Category = "Advanced Crowd")
    void OptimizeAgentLOD(const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Advanced Crowd")
    void CullDistantAgents(const FVector& PlayerLocation);

protected:
    // Configurações
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    FDynamicDensityConfig DensityConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    TArray<FEmergentEventConfig> EmergentEventConfigs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    TArray<FSeasonalMigrationConfig> MigrationConfigs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    TArray<FSpeciesCommunicationRule> CommunicationRules;

    // Estado atual
    UPROPERTY()
    TArray<FActiveEmergentEvent> ActiveEvents;

    UPROPERTY()
    TMap<EJurassicAgentType, int32> CurrentMigrationWaypoint;

    UPROPERTY()
    float CurrentSeasonTime = 0.0f;

    UPROPERTY()
    bool bDebugVisualization = false;

    // Referências
    UPROPERTY()
    TObjectPtr<UMassEntitySubsystem> MassEntitySubsystem;

    // Estatísticas
    UPROPERTY()
    int32 TotalActiveAgents = 0;

    UPROPERTY()
    int32 HighDetailAgents = 0;

    UPROPERTY()
    int32 MediumDetailAgents = 0;

    UPROPERTY()
    int32 LowDetailAgents = 0;

    UPROPERTY()
    FVector LastPlayerLocation = FVector::ZeroVector;

private:
    // Métodos internos
    void InitializeDefaultConfigurations();
    void ProcessEmergentEvents(float DeltaTime);
    void UpdateEventAffectedAgents(FActiveEmergentEvent& Event);
    void CheckEventTriggerConditions(float DeltaTime);
    
    // Densidade dinâmica
    void SpawnAgentsInZone(const FVector& Center, float Radius, float DensityMultiplier);
    void DespawnAgentsInZone(const FVector& Center, float Radius);
    
    // Migração
    FVector GetCurrentMigrationTarget(EJurassicAgentType Species) const;
    void UpdateSpeciesMigration(EJurassicAgentType Species, float DeltaTime);
    
    // Comunicação
    void ProcessCommunicationForAgent(FMassEntityHandle Entity, 
                                    const FSpeciesCommunicationRule& Rule);
    
    // Debug
    void DrawDebugInformation() const;
    
    // Performance
    void UpdateAgentLODLevel(FMassEntityHandle Entity, float DistanceToPlayer);
    
    // Timers
    float EmergentEventCheckTimer = 0.0f;
    float DensityUpdateTimer = 0.0f;
    float MigrationUpdateTimer = 0.0f;
    
    // Constantes
    static constexpr float EMERGENT_EVENT_CHECK_INTERVAL = 30.0f; // Verifica eventos a cada 30s
    static constexpr float DENSITY_UPDATE_INTERVAL = 5.0f; // Atualiza densidade a cada 5s
    static constexpr float MIGRATION_UPDATE_INTERVAL = 10.0f; // Atualiza migração a cada 10s
};