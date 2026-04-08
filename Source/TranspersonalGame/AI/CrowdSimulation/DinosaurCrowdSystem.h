#pragma once

#include "CoreMinimal.h"
#include "MassEntitySubsystem.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "MassNavigationFragments.h"
#include "DinosaurCrowdSystem.generated.h"

// Forward Declarations
class UMassEntitySubsystem;
struct FMassEntityHandle;

/**
 * Fragment que define o tipo de dinossauro para simulação de multidão
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDinosaurTypeFragment : public FMassFragment
{
    GENERATED_BODY()

public:
    // Tipo de dinossauro (Herbívoro, Carnívoro, Omnívoro)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    uint8 DinosaurType = 0; // 0=Herbívoro, 1=Carnívoro, 2=Omnívoro
    
    // Tamanho relativo (0=Pequeno, 1=Médio, 2=Grande, 3=Gigante)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    uint8 SizeCategory = 0;
    
    // Nível de agressividade (0-255)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    uint8 AggressionLevel = 50;
    
    // ID único para reconhecimento individual
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    uint32 IndividualID = 0;
};

/**
 * Fragment que define comportamento de grupo/manada
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FHerdBehaviorFragment : public FMassFragment
{
    GENERATED_BODY()

public:
    // ID do grupo/manada (0 = solitário)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    uint32 HerdID = 0;
    
    // Posição hierárquica no grupo (0=Líder, 1=Seguidor, 2=Jovem)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    uint8 HerdRank = 1;
    
    // Distância preferida do líder da manada
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PreferredDistanceFromLeader = 500.0f;
    
    // Raio de coesão do grupo
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CohesionRadius = 1000.0f;
};

/**
 * Fragment que define rotinas diárias dos dinossauros
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDailyRoutineFragment : public FMassFragment
{
    GENERATED_BODY()

public:
    // Estado atual da rotina (0=Pastando, 1=Bebendo, 2=Descansando, 3=Migrando, 4=Caçando)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    uint8 CurrentActivity = 0;
    
    // Tempo restante na atividade atual (em segundos)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TimeInCurrentActivity = 0.0f;
    
    // Duração total da atividade atual
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ActivityDuration = 300.0f;
    
    // Localização preferida para a atividade atual
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector PreferredActivityLocation = FVector::ZeroVector;
};

/**
 * Fragment que define características físicas únicas
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPhysicalVariationFragment : public FMassFragment
{
    GENERATED_BODY()

public:
    // Escala geral do dinossauro (0.8 - 1.2)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float OverallScale = 1.0f;
    
    // Variação de cor (índice para palette)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    uint8 ColorVariation = 0;
    
    // Características distintivas (cicatrizes, marcas, etc.)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    uint32 DistinctiveFeatures = 0;
    
    // Seed para geração procedural de características
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    uint32 VariationSeed = 0;
};

/**
 * Sistema principal de simulação de multidões de dinossauros
 * Usa Mass Entity Framework para simular até 50.000 dinossauros simultâneos
 */
UCLASS()
class TRANSPERSONALGAME_API UDinosaurCrowdSystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UDinosaurCrowdSystem();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Spawnar uma manada de dinossauros numa localização
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Crowd")
    void SpawnDinosaurHerd(FVector Location, int32 HerdSize, uint8 DinosaurType, uint32 HerdID);
    
    // Spawnar dinossauro solitário
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Crowd")
    FMassEntityHandle SpawnSolitaryDinosaur(FVector Location, uint8 DinosaurType);
    
    // Obter informações de um dinossauro específico
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Crowd")
    bool GetDinosaurInfo(FMassEntityHandle Entity, FDinosaurTypeFragment& OutDinosaurInfo);
    
    // Definir novo objetivo para uma manada
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Crowd")
    void SetHerdDestination(uint32 HerdID, FVector Destination);

protected:
    // Referência ao subsistema Mass Entity
    UPROPERTY()
    TObjectPtr<UMassEntitySubsystem> MassEntitySubsystem;
    
    // Configurações de spawn por tipo de dinossauro
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Settings")
    TMap<uint8, float> DinosaurSpawnWeights;
    
    // Tamanhos típicos de manada por tipo
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd Settings")
    TMap<uint8, int32> TypicalHerdSizes;

private:
    // Contador para IDs únicos
    uint32 NextIndividualID = 1;
    uint32 NextHerdID = 1;
    
    // Gerar características físicas únicas
    FPhysicalVariationFragment GeneratePhysicalVariation(uint32 Seed);
    
    // Determinar comportamento de grupo baseado no tipo
    FHerdBehaviorFragment GenerateHerdBehavior(uint8 DinosaurType, uint32 HerdID, bool IsLeader);
};