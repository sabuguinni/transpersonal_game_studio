#pragma once

#include "CoreMinimal.h"
#include "MassEntitySubsystem.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "MassNavigationFragments.h"
#include "MassActorSubsystem.h"
#include "DinosaurCrowdSystem.generated.h"

// Forward declarations
class UMassEntitySubsystem;
struct FMassEntityHandle;

/**
 * Fragment que define o tipo de dinossauro e suas características básicas
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDinosaurTypeFragment : public FMassFragment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    uint8 SpeciesID = 0;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Size = 1.0f; // Multiplicador de tamanho (0.5 = metade, 2.0 = dobro)
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsHerbivore = true;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsPackAnimal = false; // Se forma grupos
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AggressionLevel = 0.1f; // 0.0 = pacífico, 1.0 = muito agressivo
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxSpeed = 500.0f; // Unidades UE por segundo
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ViewDistance = 2000.0f; // Distância de visão
};

/**
 * Fragment que controla o comportamento atual do dinossauro
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDinosaurBehaviorFragment : public FMassFragment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    uint8 CurrentBehavior = 0; // 0=Idle, 1=Grazing, 2=Moving, 3=Fleeing, 4=Hunting, 5=Sleeping
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BehaviorTimer = 0.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BehaviorDuration = 10.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector TargetLocation = FVector::ZeroVector;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    uint32 GroupID = 0; // ID do grupo ao qual pertence (0 = solitário)
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsGroupLeader = false;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float StressLevel = 0.0f; // 0.0 = calmo, 1.0 = pânico total
};

/**
 * Fragment que define necessidades básicas do dinossauro
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDinosaurNeedsFragment : public FMassFragment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Hunger = 0.5f; // 0.0 = saciado, 1.0 = faminto
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Thirst = 0.5f; // 0.0 = hidratado, 1.0 = sedento
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Energy = 1.0f; // 0.0 = exausto, 1.0 = energético
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LastFeedTime = 0.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LastDrinkTime = 0.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LastRestTime = 0.0f;
};

/**
 * Fragment para detecção de ameaças e outros dinossauros
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDinosaurPerceptionFragment : public FMassFragment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FMassEntityHandle> NearbyDinosaurs;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FMassEntityHandle NearestThreat;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FMassEntityHandle NearestPrey;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector PlayerLocation = FVector::ZeroVector;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PlayerDistance = 999999.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bCanSeePlayer = false;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LastPlayerSightTime = 0.0f;
};

/**
 * Sistema principal de gestão de multidões de dinossauros
 */
UCLASS()
class TRANSPERSONALGAME_API UDinosaurCrowdSystem : public UMassEntitySubsystem
{
    GENERATED_BODY()

public:
    UDinosaurCrowdSystem();

    // Spawning de dinossauros
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Crowd")
    void SpawnDinosaurHerd(uint8 SpeciesID, int32 Count, FVector Location, float SpreadRadius = 1000.0f);
    
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Crowd")
    void SpawnSolitaryDinosaur(uint8 SpeciesID, FVector Location);
    
    // Gestão de grupos
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Crowd")
    uint32 CreateDinosaurGroup(const TArray<FMassEntityHandle>& Members);
    
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Crowd")
    void DisbandGroup(uint32 GroupID);
    
    // Configuração de espécies
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Crowd")
    void RegisterDinosaurSpecies(uint8 SpeciesID, const FDinosaurTypeFragment& SpeciesData);
    
    // Eventos de mundo
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Crowd")
    void TriggerPanicEvent(FVector Location, float Radius, float Intensity = 1.0f);
    
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Crowd")
    void SetTimeOfDay(float TimeHours); // 0-24 horas
    
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Crowd")
    void SetWeatherCondition(uint8 WeatherType); // 0=Clear, 1=Rain, 2=Storm

protected:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

private:
    // Dados de espécies registadas
    UPROPERTY()
    TMap<uint8, FDinosaurTypeFragment> RegisteredSpecies;
    
    // Grupos activos
    UPROPERTY()
    TMap<uint32, TArray<FMassEntityHandle>> ActiveGroups;
    
    // Contador para IDs únicos de grupos
    uint32 NextGroupID = 1;
    
    // Configurações ambientais
    float CurrentTimeOfDay = 12.0f; // Meio-dia por defeito
    uint8 CurrentWeather = 0; // Tempo limpo por defeito
    
    // Áreas de recursos (água, comida)
    TArray<FVector> WaterSources;
    TArray<FVector> FoodSources;
    
    // Métodos internos
    void InitializeSpeciesDatabase();
    void SetupResourceAreas();
    FVector FindNearestWaterSource(FVector Location) const;
    FVector FindNearestFoodSource(FVector Location) const;
};

/**
 * Enums para comportamentos e estados
 */
UENUM(BlueprintType)
enum class EDinosaurBehavior : uint8
{
    Idle = 0,
    Grazing = 1,
    Moving = 2,
    Fleeing = 3,
    Hunting = 4,
    Sleeping = 5,
    Drinking = 6,
    Socializing = 7,
    Territorial = 8
};

UENUM(BlueprintType)
enum class EDinosaurSpecies : uint8
{
    Triceratops = 0,
    Parasaurolophus = 1,
    Stegosaurus = 2,
    Brachiosaurus = 3,
    Velociraptor = 4,
    TyrannosaurusRex = 5,
    Allosaurus = 6,
    Compsognathus = 7,
    Ankylosaurus = 8,
    Diplodocus = 9
};

UENUM(BlueprintType)
enum class EWeatherCondition : uint8
{
    Clear = 0,
    LightRain = 1,
    HeavyRain = 2,
    Storm = 3,
    Fog = 4
};