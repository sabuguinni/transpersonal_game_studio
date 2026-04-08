#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MassEntitySubsystem.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "JurassicMassEntitySubsystem.generated.h"

class UMassSpawnerSubsystem;

UENUM(BlueprintType)
enum class EDinosaurSpecies : uint8
{
    // Herbívoros de Manada (Alta Densidade)
    Triceratops,
    Parasaurolophus,
    Stegosaurus,
    Ankylosaurus,
    
    // Herbívoros Solitários (Média Densidade)
    Brachiosaurus,
    Diplodocus,
    
    // Carnívoros de Matilha (Baixa Densidade)
    Velociraptor,
    Deinonychus,
    
    // Carnívoros Solitários (Muito Baixa Densidade)
    TyrannosaurusRex,
    Allosaurus,
    
    // Voadores (Densidade Variável)
    Pteranodon,
    
    // Aquáticos (Zonas Específicas)
    Plesiosaur
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDinosaurMassConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EDinosaurSpecies Species;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxPopulation = 1000;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SpawnRadius = 5000.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float HerdSize = 10.0f; // Média de indivíduos por grupo
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TerritorySize = 2000.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsHerdAnimal = true;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsPredator = false;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DangerResponseRadius = 1500.0f;
};

USTRUCT()
struct TRANSPERSONALGAME_API FDinosaurFragment : public FMassFragment
{
    GENERATED_BODY()

    EDinosaurSpecies Species = EDinosaurSpecies::Triceratops;
    float Health = 100.0f;
    float Hunger = 0.0f;
    float Fear = 0.0f;
    float Aggression = 0.0f;
    
    // Identificação única para reconhecimento individual
    int32 UniqueID = 0;
    FVector PhysicalVariation = FVector::ZeroVector; // Variações de tamanho/forma
    FLinearColor ColorVariation = FLinearColor::White;
    
    // Estado comportamental
    bool bIsInHerd = false;
    int32 HerdID = -1;
    bool bIsAlpha = false;
    bool bIsScared = false;
    bool bIsHunting = false;
    bool bIsFeeding = false;
    
    // Timers
    float LastFeedTime = 0.0f;
    float LastDrinkTime = 0.0f;
    float LastRestTime = 0.0f;
};

USTRUCT()
struct TRANSPERSONALGAME_API FHerdFragment : public FMassFragment
{
    GENERATED_BODY()

    int32 HerdID = -1;
    TArray<FMassEntityHandle> HerdMembers;
    FMassEntityHandle AlphaEntity;
    FVector HerdCenter = FVector::ZeroVector;
    FVector MovementDirection = FVector::ForwardVector;
    float HerdRadius = 500.0f;
    bool bIsFleeingDanger = false;
    FVector DangerLocation = FVector::ZeroVector;
    float FleeStartTime = 0.0f;
};

USTRUCT()
struct TRANSPERSONALGAME_API FEcosystemStateFragment : public FMassFragment
{
    GENERATED_BODY()

    // Estado geral do ecossistema na região
    float PredatorPressure = 0.0f;
    float FoodAvailability = 1.0f;
    float WaterAvailability = 1.0f;
    float WeatherStress = 0.0f;
    
    // Eventos em cascata
    bool bMajorPredatorPresent = false;
    bool bStampedeInProgress = false;
    FVector StampedeOrigin = FVector::ZeroVector;
    float StampedeRadius = 0.0f;
};

/**
 * Subsistema principal para gerenciar simulação de massa de dinossauros
 * Baseado em Mass AI do UE5 para suportar até 50.000 agentes simultâneos
 */
UCLASS()
class TRANSPERSONALGAME_API UJurassicMassEntitySubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual void Tick(float DeltaTime) override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    // Configuração de espécies
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Simulation")
    TArray<FDinosaurMassConfig> SpeciesConfigs;
    
    // Controle de densidade populacional
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Population Control")
    int32 MaxTotalPopulation = 50000;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Population Control")
    float PopulationDensityPerKm2 = 100.0f;
    
    // Sistema de LOD para performance
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float HighDetailRadius = 2000.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MediumDetailRadius = 5000.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LowDetailRadius = 10000.0f;

    // Funções públicas
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Simulation")
    void SpawnDinosaurHerd(EDinosaurSpecies Species, const FVector& Location, int32 HerdSize);
    
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Simulation")
    void TriggerDangerResponse(const FVector& DangerLocation, float DangerRadius, bool bIsPredatorThreat = true);
    
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Simulation")
    void StartStampede(const FVector& Origin, float Radius);
    
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Simulation")
    TArray<FMassEntityHandle> GetNearbyDinosaurs(const FVector& Location, float Radius, EDinosaurSpecies SpeciesFilter = EDinosaurSpecies::Triceratops);

private:
    UPROPERTY()
    UMassEntitySubsystem* MassEntitySubsystem;
    
    UPROPERTY()
    UMassSpawnerSubsystem* SpawnerSubsystem;
    
    // Gerenciamento de manadas
    TMap<int32, FHerdFragment> ActiveHerds;
    int32 NextHerdID = 0;
    
    // Estado do ecossistema
    FEcosystemStateFragment EcosystemState;
    
    // Funções internas
    void InitializeSpeciesConfigs();
    void UpdateEcosystemState(float DeltaTime);
    void ProcessHerdBehavior(float DeltaTime);
    void ProcessPredatorBehavior(float DeltaTime);
    void ProcessDangerResponses(float DeltaTime);
    
    int32 CreateNewHerd(EDinosaurSpecies Species, const FVector& Center);
    void AddEntityToHerd(FMassEntityHandle Entity, int32 HerdID);
    void RemoveEntityFromHerd(FMassEntityHandle Entity, int32 HerdID);
    
    FDinosaurMassConfig GetConfigForSpecies(EDinosaurSpecies Species) const;
};