#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "MassProcessor.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "DinosaurHerdSystem.generated.h"

// Fragment para comportamento de manada
USTRUCT()
struct TRANSPERSONALGAME_API FDinosaurHerdFragment : public FMassFragment
{
    GENERATED_BODY()

    // ID da manada a que pertence
    UPROPERTY()
    int32 HerdID = -1;
    
    // Tipo de dinossauro (Herbívoro, Carnívoro, etc.)
    UPROPERTY()
    EDinosaurType DinosaurType = EDinosaurType::Herbivore;
    
    // Posição dentro da hierarquia da manada (0 = líder alfa)
    UPROPERTY()
    int32 HerdRank = 0;
    
    // Raio de detecção de outros membros da manada
    UPROPERTY()
    float HerdDetectionRadius = 1000.0f;
    
    // Força de coesão (tendência para ficar junto)
    UPROPERTY()
    float CohesionStrength = 1.0f;
    
    // Força de separação (evitar colisões)
    UPROPERTY()
    float SeparationStrength = 2.0f;
    
    // Força de alinhamento (seguir direção da manada)
    UPROPERTY()
    float AlignmentStrength = 1.5f;
    
    // Estado atual do membro da manada
    UPROPERTY()
    EHerdState CurrentState = EHerdState::Grazing;
    
    // Timer para mudanças de estado
    UPROPERTY()
    float StateTimer = 0.0f;
    
    // Nível de stress/medo (afeta comportamento)
    UPROPERTY()
    float StressLevel = 0.0f;
};

// Estados possíveis de um membro da manada
UENUM()
enum class EHerdState : uint8
{
    Grazing,        // A pastar calmamente
    Moving,         // Em movimento normal
    Alert,          // Detectou possível ameaça
    Fleeing,        // A fugir de predador
    Drinking,       // A beber água
    Resting,        // A descansar
    Stampeding      // Em debandada
};

// Tipos de dinossauros para comportamento
UENUM()
enum class EDinosaurType : uint8
{
    Herbivore,      // Herbívoro (forma manadas)
    Carnivore,      // Carnívoro (caça sozinho ou em pequenos grupos)
    Omnivore,       // Omnívoro (comportamento misto)
    Flying,         // Voador (bandos aéreos)
    Aquatic         // Aquático (cardumes)
};

// Fragment para dados globais da manada
USTRUCT()
struct TRANSPERSONALGAME_API FHerdDataFragment : public FMassSharedFragment
{
    GENERATED_BODY()
    
    // Centro atual da manada
    UPROPERTY()
    FVector HerdCenter = FVector::ZeroVector;
    
    // Direção média de movimento
    UPROPERTY()
    FVector AverageDirection = FVector::ForwardVector;
    
    // Velocidade média da manada
    UPROPERTY()
    float AverageSpeed = 100.0f;
    
    // Número de membros na manada
    UPROPERTY()
    int32 MemberCount = 0;
    
    // Nível de alerta da manada (0-1)
    UPROPERTY()
    float AlertLevel = 0.0f;
    
    // Última posição de ameaça detectada
    UPROPERTY()
    FVector LastThreatPosition = FVector::ZeroVector;
    
    // Tempo desde última ameaça
    UPROPERTY()
    float TimeSinceLastThreat = 0.0f;
    
    // Destino atual da manada (água, comida, etc.)
    UPROPERTY()
    FVector CurrentDestination = FVector::ZeroVector;
    
    // Tipo de destino atual
    UPROPERTY()
    EHerdDestinationType DestinationType = EHerdDestinationType::Grazing;
};

// Tipos de destino para manadas
UENUM()
enum class EHerdDestinationType : uint8
{
    Grazing,        // Área de pastagem
    Water,          // Fonte de água
    Shelter,        // Abrigo/sombra
    Migration,      // Rota migratória
    Escape          // Fuga de predador
};

/**
 * Processor principal para comportamento de manadas de dinossauros
 * Implementa algoritmos de boids (Reynolds) adaptados para dinossauros
 */
UCLASS()
class TRANSPERSONALGAME_API UDinosaurHerdProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    UDinosaurHerdProcessor();

protected:
    virtual void ConfigureQueries() override;
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
    // Query para membros de manada
    FMassEntityQuery HerdMemberQuery;
    
    // Query para dados de manada
    FMassEntityQuery HerdDataQuery;
    
    // Calcula força de coesão (aproximar do centro da manada)
    FVector CalculateCohesionForce(const FVector& Position, const FVector& HerdCenter, float Strength) const;
    
    // Calcula força de separação (evitar colisões)
    FVector CalculateSeparationForce(const FVector& Position, const TArray<FVector>& NearbyPositions, float Strength) const;
    
    // Calcula força de alinhamento (seguir direção da manada)
    FVector CalculateAlignmentForce(const FVector& CurrentVelocity, const FVector& HerdDirection, float Strength) const;
    
    // Calcula força de fuga de predadores
    FVector CalculateFleeForce(const FVector& Position, const FVector& ThreatPosition, float FleeRadius) const;
    
    // Atualiza estado do membro da manada baseado em condições
    void UpdateHerdMemberState(FDinosaurHerdFragment& HerdFragment, const FVector& Position, float DeltaTime) const;
    
    // Atualiza dados globais da manada
    void UpdateHerdData(FHerdDataFragment& HerdData, const TArray<FVector>& MemberPositions, const TArray<FVector>& MemberVelocities) const;

    // Parâmetros configuráveis
    UPROPERTY(EditAnywhere, Category = "Herd Behavior")
    float MaxForce = 500.0f;
    
    UPROPERTY(EditAnywhere, Category = "Herd Behavior")
    float MaxSpeed = 800.0f;
    
    UPROPERTY(EditAnywhere, Category = "Herd Behavior")
    float NeighborRadius = 500.0f;
    
    UPROPERTY(EditAnywhere, Category = "Herd Behavior")
    float FleeRadius = 2000.0f;
    
    UPROPERTY(EditAnywhere, Category = "Herd Behavior")
    float AlertDecayRate = 0.1f;
    
    UPROPERTY(EditAnywhere, Category = "Herd Behavior")
    float StressDecayRate = 0.05f;
};

/**
 * Processor para detectar e reagir a predadores
 */
UCLASS()
class TRANSPERSONALGAME_API UPredatorDetectionProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    UPredatorDetectionProcessor();

protected:
    virtual void ConfigureQueries() override;
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
    FMassEntityQuery PreyQuery;
    FMassEntityQuery PredatorQuery;
    
    // Detecta predadores próximos e atualiza nível de alerta
    void DetectNearbyPredators(FDinosaurHerdFragment& HerdFragment, const FVector& Position, const TArray<FVector>& PredatorPositions) const;
    
    // Propaga alerta através da manada
    void PropagateAlert(FMassEntityManager& EntityManager, int32 HerdID, const FVector& ThreatPosition, float AlertLevel) const;

    UPROPERTY(EditAnywhere, Category = "Predator Detection")
    float DetectionRadius = 1500.0f;
    
    UPROPERTY(EditAnywhere, Category = "Predator Detection")
    float AlertPropagationRadius = 2000.0f;
    
    UPROPERTY(EditAnywhere, Category = "Predator Detection")
    float MaxAlertLevel = 1.0f;
};