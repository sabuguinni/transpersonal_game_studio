#pragma once

#include "CoreMinimal.h"
#include "MassEntitySubsystem.h"
#include "MassProcessor.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "DinosaurHerdSystem.generated.h"

// Fragment para dados específicos de manada
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDinosaurHerdFragment : public FMassFragment
{
    GENERATED_BODY()

    // ID único da manada
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 HerdID = -1;

    // Tipo de dinossauro (herbívoro, carnívoro, etc.)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EDinosaurType DinosaurType = EDinosaurType::Herbivore;

    // Posição do líder da manada
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector LeaderPosition = FVector::ZeroVector;

    // Raio de coesão da manada
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CohesionRadius = 500.0f;

    // Força de separação entre indivíduos
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SeparationForce = 2.0f;

    // Velocidade preferida da manada
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PreferredSpeed = 300.0f;

    // Estado atual da manada (pastando, migrando, fugindo)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EHerdState CurrentState = EHerdState::Grazing;

    // Tempo restante no estado atual
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float StateTimer = 0.0f;
};

// Enums para tipos de dinossauro e estados de manada
UENUM(BlueprintType)
enum class EDinosaurType : uint8
{
    Herbivore,
    Carnivore,
    Omnivore,
    Scavenger
};

UENUM(BlueprintType)
enum class EHerdState : uint8
{
    Grazing,      // Pastando
    Migrating,    // Migrando
    Fleeing,      // Fugindo
    Drinking,     // Bebendo água
    Resting       // Descansando
};

// Fragment para comportamento de fuga
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FFleeingBehaviorFragment : public FMassFragment
{
    GENERATED_BODY()

    // Posição da ameaça
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector ThreatPosition = FVector::ZeroVector;

    // Intensidade do medo (0-1)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FearLevel = 0.0f;

    // Tempo desde que a ameaça foi detectada
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TimeSinceThreatDetected = 0.0f;

    // Distância mínima segura da ameaça
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SafeDistance = 1000.0f;
};

// Processor principal para simulação de manadas
UCLASS()
class TRANSPERSONALGAME_API UDinosaurHerdProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    UDinosaurHerdProcessor();

protected:
    virtual void ConfigureQueries() override;
    virtual void Execute(UMassEntitySubsystem& EntitySubsystem, FMassExecutionContext& Context) override;

private:
    // Query para entidades de manada
    FMassEntityQuery HerdEntityQuery;

    // Funções de comportamento
    void ProcessGrazingBehavior(FMassExecutionContext& Context, FDinosaurHerdFragment& HerdData, FTransformFragment& Transform);
    void ProcessMigrationBehavior(FMassExecutionContext& Context, FDinosaurHerdFragment& HerdData, FTransformFragment& Transform);
    void ProcessFleeingBehavior(FMassExecutionContext& Context, FDinosaurHerdFragment& HerdData, FFleeingBehaviorFragment& FleeData, FTransformFragment& Transform);
    
    // Funções de cálculo de forças
    FVector CalculateCohesionForce(const FVector& EntityPosition, const FVector& LeaderPosition, float CohesionRadius);
    FVector CalculateSeparationForce(const FVector& EntityPosition, const TArray<FVector>& NearbyPositions, float SeparationRadius);
    FVector CalculateAlignmentForce(const FVector& EntityVelocity, const FVector& AverageVelocity);
    
    // Detecção de ameaças
    bool DetectThreat(const FVector& Position, float DetectionRadius, FVector& ThreatPosition);
};

// Processor para migração sazonal
UCLASS()
class TRANSPERSONALGAME_API USeasonalMigrationProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    USeasonalMigrationProcessor();

protected:
    virtual void ConfigureQueries() override;
    virtual void Execute(UMassEntitySubsystem& EntitySubsystem, FMassExecutionContext& Context) override;

private:
    FMassEntityQuery MigrationQuery;
    
    // Pontos de migração sazonais
    UPROPERTY(EditAnywhere)
    TArray<FVector> SpringMigrationPoints;
    
    UPROPERTY(EditAnywhere)
    TArray<FVector> SummerMigrationPoints;
    
    UPROPERTY(EditAnywhere)
    TArray<FVector> AutumnMigrationPoints;
    
    UPROPERTY(EditAnywhere)
    TArray<FVector> WinterMigrationPoints;
    
    // Função para determinar pontos de migração baseados na estação
    TArray<FVector> GetMigrationPointsForSeason(ESeason CurrentSeason);
};

// Enum para estações do ano
UENUM(BlueprintType)
enum class ESeason : uint8
{
    Spring,
    Summer,
    Autumn,
    Winter
};