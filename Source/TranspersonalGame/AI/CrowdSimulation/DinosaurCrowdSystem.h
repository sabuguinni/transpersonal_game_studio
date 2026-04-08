#pragma once

#include "CoreMinimal.h"
#include "MassEntitySubsystem.h"
#include "MassProcessor.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "DinosaurCrowdSystem.generated.h"

// Forward declarations
class UMassEntitySubsystem;
struct FMassEntityHandle;

/**
 * Tipos de comportamento de grupo
 */
UENUM(BlueprintType)
enum class EDinosaurHerdType : uint8
{
    Herbivore       UMETA(DisplayName = "Herbívoro - Manada Defensiva"),
    Carnivore       UMETA(DisplayName = "Carnívoro - Bando de Caça"),
    Scavenger       UMETA(DisplayName = "Necrófago - Grupo Oportunista"),
    SmallPack       UMETA(DisplayName = "Pequeno Bando - Enxame"),
    Solitary        UMETA(DisplayName = "Solitário - Sem Grupo")
};

/**
 * Estados de comportamento de grupo
 */
UENUM(BlueprintType)
enum class EDinosaurHerdState : uint8
{
    Grazing         UMETA(DisplayName = "Pastando/Alimentando"),
    Moving          UMETA(DisplayName = "Em Movimento"),
    Drinking        UMETA(DisplayName = "Bebendo Água"),
    Resting         UMETA(DisplayName = "Descansando"),
    Alert           UMETA(DisplayName = "Em Alerta"),
    Fleeing         UMETA(DisplayName = "Fugindo"),
    Hunting         UMETA(DisplayName = "Caçando"),
    Migrating       UMETA(DisplayName = "Migrando")
};

/**
 * Fragment que define o comportamento de grupo de um dinossauro
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDinosaurHerdFragment : public FMassFragment
{
    GENERATED_BODY()

    // ID único do grupo/manada
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 HerdID = -1;

    // Tipo de comportamento de grupo
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EDinosaurHerdType HerdType = EDinosaurHerdType::Herbivore;

    // Posição do líder da manada
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector LeaderPosition = FVector::ZeroVector;

    // Distância preferida do centro da manada
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PreferredDistanceFromCenter = 500.0f;

    // Força de coesão com a manada (0-1)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CohesionStrength = 0.7f;

    // Força de separação de outros membros (0-1)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SeparationStrength = 0.8f;

    // Força de alinhamento com a direção da manada (0-1)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AlignmentStrength = 0.6f;
};

/**
 * Fragment que define o estado atual do comportamento de grupo
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDinosaurHerdStateFragment : public FMassFragment
{
    GENERATED_BODY()

    // Estado atual do comportamento
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EDinosaurHerdState CurrentState = EDinosaurHerdState::Grazing;

    // Tempo restante no estado atual
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float StateTimeRemaining = 0.0f;

    // Alvo atual (pode ser comida, água, abrigo, etc.)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector TargetLocation = FVector::ZeroVector;

    // Nível de alerta da manada (0-1)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AlertLevel = 0.0f;
};

/**
 * Dados de uma manada para gestão pelo subsystem
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDinosaurHerdData
{
    GENERATED_BODY()

    // Entidades que fazem parte desta manada
    UPROPERTY()
    TArray<FMassEntityHandle> HerdMembers;

    // Tipo da manada
    UPROPERTY()
    EDinosaurHerdType HerdType = EDinosaurHerdType::Herbivore;

    // Centro atual da manada
    UPROPERTY()
    FVector HerdCenter = FVector::ZeroVector;

    // Estado atual da manada
    UPROPERTY()
    EDinosaurHerdState CurrentState = EDinosaurHerdState::Grazing;

    // Destino atual da manada
    UPROPERTY()
    FVector Destination = FVector::ZeroVector;

    // Tempo de criação da manada
    UPROPERTY()
    float CreationTime = 0.0f;
};

/**
 * Processor responsável pelo comportamento de coesão das manadas
 */
UCLASS()
class TRANSPERSONALGAME_API UDinosaurHerdCohesionProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    UDinosaurHerdCohesionProcessor();

protected:
    virtual void ConfigureQueries() override;
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
    FMassEntityQuery EntityQuery;

    // Calcula força de coesão para manter o grupo unido
    FVector CalculateCohesionForce(const FVector& CurrentPosition, const FVector& HerdCenter, float CohesionStrength);

    // Calcula força de separação para evitar sobreposição
    FVector CalculateSeparationForce(const FVector& CurrentPosition, const TArray<FVector>& NearbyPositions, float SeparationStrength);

    // Calcula força de alinhamento para mover na mesma direção
    FVector CalculateAlignmentForce(const FVector& CurrentVelocity, const FVector& HerdAverageVelocity, float AlignmentStrength);
};

/**
 * Processor responsável pela gestão de estados da manada
 */
UCLASS()
class TRANSPERSONALGAME_API UDinosaurHerdStateProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    UDinosaurHerdStateProcessor();

protected:
    virtual void ConfigureQueries() override;
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
    FMassEntityQuery EntityQuery;

    // Determina se a manada deve mudar de estado
    EDinosaurHerdState DetermineNextState(const FDinosaurHerdStateFragment& CurrentState, const FDinosaurHerdFragment& HerdData, float DeltaTime);

    // Atualiza o nível de alerta baseado no ambiente
    float UpdateAlertLevel(const FVector& Position, float CurrentAlert, float DeltaTime);
};

/**
 * Subsystem que gere todas as manadas de dinossauros
 */
UCLASS()
class TRANSPERSONALGAME_API UDinosaurCrowdSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Cria uma nova manada de dinossauros
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Crowd")
    int32 CreateDinosaurHerd(EDinosaurHerdType HerdType, const FVector& SpawnLocation, int32 HerdSize = 10);

    // Remove uma manada existente
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Crowd")
    void RemoveHerd(int32 HerdID);

    // Obtém informações sobre uma manada específica
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Crowd")
    bool GetHerdInfo(int32 HerdID, FVector& HerdCenter, int32& HerdSize, EDinosaurHerdState& HerdState);

    // Define um novo destino para uma manada
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Crowd")
    void SetHerdDestination(int32 HerdID, const FVector& NewDestination);

    // Força uma manada a entrar em estado de alerta/fuga
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Crowd")
    void TriggerHerdAlert(int32 HerdID, const FVector& ThreatLocation);

private:
    // Mapa de manadas ativas
    UPROPERTY()
    TMap<int32, FDinosaurHerdData> ActiveHerds;

    // Próximo ID disponível para manadas
    int32 NextHerdID = 1;

    // Referência ao Mass Entity Subsystem
    UPROPERTY()
    UMassEntitySubsystem* MassEntitySubsystem = nullptr;
};