#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "MassEntityTypes.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "DinosaurMassProcessor.generated.h"

// Fragmentos específicos para dinossauros
USTRUCT()
struct TRANSPERSONALGAME_API FDinosaurSpeciesFragment : public FMassFragment
{
    GENERATED_BODY()

    // Tipo de dinossauro (Herbívoro, Carnívoro, etc.)
    UPROPERTY(EditAnywhere)
    uint8 SpeciesType = 0;
    
    // Tamanho relativo (0-255, onde 255 é T-Rex)
    UPROPERTY(EditAnywhere)
    uint8 Size = 128;
    
    // Nível de agressividade (0-255)
    UPROPERTY(EditAnywhere)
    uint8 AggressionLevel = 50;
    
    // Raio de detecção de outros dinossauros
    UPROPERTY(EditAnywhere)
    float DetectionRadius = 1000.0f;
};

USTRUCT()
struct TRANSPERSONALGAME_API FDinosaurBehaviorFragment : public FMassFragment
{
    GENERATED_BODY()

    // Estado comportamental atual
    enum class EBehaviorState : uint8
    {
        Wandering,      // Vagueando
        Feeding,        // Alimentando-se
        Drinking,       // Bebendo água
        Resting,        // Descansando
        Hunting,        // Caçando (carnívoros)
        Fleeing,        // Fugindo
        Socializing,    // Interação social
        Territorial     // Comportamento territorial
    };
    
    UPROPERTY(EditAnywhere)
    EBehaviorState CurrentState = EBehaviorState::Wandering;
    
    // Tempo no estado atual
    UPROPERTY(EditAnywhere)
    float StateTimer = 0.0f;
    
    // Duração mínima/máxima para cada estado
    UPROPERTY(EditAnywhere)
    float MinStateDuration = 30.0f;
    
    UPROPERTY(EditAnywhere)
    float MaxStateDuration = 120.0f;
    
    // Target atual (pode ser comida, água, outro dinossauro)
    UPROPERTY(EditAnywhere)
    FVector TargetLocation = FVector::ZeroVector;
    
    // ID da entidade alvo (se aplicável)
    UPROPERTY(EditAnywhere)
    FMassEntityHandle TargetEntity;
};

USTRUCT()
struct TRANSPERSONALGAME_API FDinosaurNeedsFragment : public FMassFragment
{
    GENERATED_BODY()

    // Necessidades básicas (0-255, onde 0 = crítico, 255 = satisfeito)
    UPROPERTY(EditAnywhere)
    uint8 Hunger = 255;
    
    UPROPERTY(EditAnywhere)
    uint8 Thirst = 255;
    
    UPROPERTY(EditAnywhere)
    uint8 Energy = 255;
    
    // Rates de decaimento por segundo
    UPROPERTY(EditAnywhere)
    float HungerDecayRate = 0.1f;
    
    UPROPERTY(EditAnywhere)
    float ThirstDecayRate = 0.15f;
    
    UPROPERTY(EditAnywhere)
    float EnergyDecayRate = 0.05f;
};

USTRUCT()
struct TRANSPERSONALGAME_API FDinosaurVariationFragment : public FMassFragment
{
    GENERATED_BODY()

    // Variações físicas únicas
    UPROPERTY(EditAnywhere)
    float ScaleVariation = 1.0f; // 0.8 - 1.2
    
    UPROPERTY(EditAnywhere)
    FLinearColor ColorTint = FLinearColor::White;
    
    // Padrões únicos de comportamento
    UPROPERTY(EditAnywhere)
    float MovementSpeedMultiplier = 1.0f;
    
    UPROPERTY(EditAnywhere)
    float AggressionModifier = 1.0f;
    
    // ID único para identificação
    UPROPERTY(EditAnywhere)
    uint32 UniqueID = 0;
};

/**
 * Processador principal para simulação de dinossauros usando Mass AI
 * Gerencia comportamento, necessidades e interações entre dinossauros
 */
UCLASS()
class TRANSPERSONALGAME_API UDinosaurMassProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    UDinosaurMassProcessor();

protected:
    virtual void ConfigureQueries() override;
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
    // Query para dinossauros ativos
    FMassEntityQuery ActiveDinosaurQuery;
    
    // Query para detecção de proximidade
    FMassEntityQuery ProximityQuery;
    
    // Processamento de comportamento
    void ProcessBehavior(FMassExecutionContext& Context);
    void ProcessNeeds(FMassExecutionContext& Context);
    void ProcessInteractions(FMassExecutionContext& Context);
    
    // Funções auxiliares
    void UpdateBehaviorState(FDinosaurBehaviorFragment& Behavior, 
                           const FDinosaurNeedsFragment& Needs,
                           const FDinosaurSpeciesFragment& Species,
                           float DeltaTime);
    
    FVector FindNearestResource(const FVector& Location, uint8 ResourceType);
    void HandleSpeciesInteraction(FMassEntityHandle Entity1, FMassEntityHandle Entity2,
                                const FDinosaurSpeciesFragment& Species1,
                                const FDinosaurSpeciesFragment& Species2);
};