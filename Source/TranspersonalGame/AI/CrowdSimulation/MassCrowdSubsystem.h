#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MassEntityTypes.h"
#include "MassProcessingTypes.h"
#include "MassSpawnerTypes.h"
#include "MassCrowdSubsystem.generated.h"

class UMassEntitySubsystem;
class UMassSimulationSubsystem;

USTRUCT(BlueprintType)
struct FDinosaurHerdConfig
{
    GENERATED_BODY()

    // Espécie do dinossauro
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName SpeciesName = "Triceratops";

    // Tamanho da manada
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "5", ClampMax = "200"))
    int32 HerdSize = 25;

    // Raio de coesão da manada
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "500.0", ClampMax = "5000.0"))
    float CohesionRadius = 1500.0f;

    // Velocidade de movimento
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "100.0", ClampMax = "1000.0"))
    float MovementSpeed = 300.0f;

    // Área de pastagem preferida
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector GrazingCenter = FVector::ZeroVector;

    // Raio da área de pastagem
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "1000.0", ClampMax = "10000.0"))
    float GrazingRadius = 3000.0f;

    // Horário de atividade (0-24)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", ClampMax = "24"))
    int32 ActiveHourStart = 6;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", ClampMax = "24"))
    int32 ActiveHourEnd = 18;
};

USTRUCT(BlueprintType)
struct FPredatorTerritoryConfig
{
    GENERATED_BODY()

    // Espécie do predador
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName SpeciesName = "TRex";

    // Centro do território
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector TerritoryCenter = FVector::ZeroVector;

    // Raio do território
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "2000.0", ClampMax = "15000.0"))
    float TerritoryRadius = 8000.0f;

    // Velocidade de patrulhamento
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "200.0", ClampMax = "800.0"))
    float PatrolSpeed = 400.0f;

    // Distância de detecção de presas
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "1000.0", ClampMax = "5000.0"))
    float HuntingRange = 2500.0f;

    // Horário de caça preferido
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", ClampMax = "24"))
    int32 HuntingHourStart = 5;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", ClampMax = "24"))
    int32 HuntingHourEnd = 10;
};

USTRUCT(BlueprintType)
struct FMigrationRouteConfig
{
    GENERATED_BODY()

    // Nome da rota
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString RouteName = "SeasonalMigration";

    // Pontos da rota de migração
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FVector> WayPoints;

    // Espécies que usam esta rota
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FName> MigratingSpecies;

    // Duração da migração em dias de jogo
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "1", ClampMax = "30"))
    int32 MigrationDurationDays = 7;

    // Época do ano para migração (0-365)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", ClampMax = "365"))
    int32 MigrationStartDay = 90; // Primavera

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", ClampMax = "365"))
    int32 ReturnMigrationDay = 270; // Outono
};

/**
 * Subsistema responsável pela gestão de simulação de multidões de dinossauros
 * Utiliza Mass AI para simular até 50.000 agentes simultâneos
 */
UCLASS()
class TRANSPERSONALGAME_API UMassCrowdSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    // Configuração de manadas
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void RegisterHerdConfig(const FDinosaurHerdConfig& HerdConfig);

    // Configuração de territórios de predadores
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void RegisterPredatorTerritory(const FPredatorTerritoryConfig& TerritoryConfig);

    // Configuração de rotas de migração
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void RegisterMigrationRoute(const FMigrationRouteConfig& RouteConfig);

    // Spawning de entidades
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SpawnHerd(const FName& SpeciesName, const FVector& Location, int32 Count);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SpawnPredator(const FName& SpeciesName, const FVector& Location);

    // Controle de simulação
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void StartMigration(const FString& RouteName);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void PauseSimulation();

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void ResumeSimulation();

    // Debug e estatísticas
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    int32 GetActiveEntityCount() const;

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    float GetSimulationPerformance() const;

    // Getters para configurações
    const TArray<FDinosaurHerdConfig>& GetHerdConfigs() const { return HerdConfigs; }
    const TArray<FPredatorTerritoryConfig>& GetPredatorConfigs() const { return PredatorConfigs; }
    const TArray<FMigrationRouteConfig>& GetMigrationRoutes() const { return MigrationRoutes; }

protected:
    // Configurações registradas
    UPROPERTY()
    TArray<FDinosaurHerdConfig> HerdConfigs;

    UPROPERTY()
    TArray<FPredatorTerritoryConfig> PredatorConfigs;

    UPROPERTY()
    TArray<FMigrationRouteConfig> MigrationRoutes;

    // Referências aos subsistemas Mass
    UPROPERTY()
    TObjectPtr<UMassEntitySubsystem> MassEntitySubsystem;

    UPROPERTY()
    TObjectPtr<UMassSimulationSubsystem> MassSimulationSubsystem;

    // Entidades ativas
    TArray<FMassEntityHandle> ActiveEntities;

    // Estado da simulação
    bool bSimulationPaused = false;
    float LastPerformanceCheck = 0.0f;
    int32 FrameCounter = 0;

private:
    // Métodos internos
    void InitializeMassFramework();
    void SetupDefaultConfigurations();
    void UpdatePerformanceMetrics();
};