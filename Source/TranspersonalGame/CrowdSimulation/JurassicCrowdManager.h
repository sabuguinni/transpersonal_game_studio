#pragma once

#include "CoreMinimal.h"
#include "MassEntitySubsystem.h"
#include "MassProcessor.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "Engine/World.h"
#include "JurassicCrowdManager.generated.h"

// Forward declarations
class UMassEntitySubsystem;
struct FMassEntityHandle;

/**
 * Fragment que define características únicas de cada dinossauro
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FJurassicDinosaurFragment : public FMassFragment
{
    GENERATED_BODY()

    // Identificação única do dinossauro
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGuid DinosaurID;

    // Espécie do dinossauro
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName Species;

    // Variações genéticas visuais
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float HornSize = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float EyeSize = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor SkinTone = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BodyScale = 1.0f;

    // Estado comportamental
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsHerbivore = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsDomesticated = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AggressionLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FearLevel = 0.3f;

    // Memória social
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FGuid> KnownDinosaurs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGuid PlayerMemory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PlayerTrustLevel = 0.0f;
};

/**
 * Fragment para comportamento de manada
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FJurassicHerdFragment : public FMassFragment
{
    GENERATED_BODY()

    // ID da manada
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGuid HerdID;

    // Papel na manada
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsHerdLeader = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsScout = false;

    // Coesão da manada
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CohesionRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SeparationRadius = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AlignmentRadius = 500.0f;

    // Estado da manada
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bHerdInDanger = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector DangerSource = FVector::ZeroVector;
};

/**
 * Fragment para rotinas diárias dos dinossauros
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FJurassicDailyRoutineFragment : public FMassFragment
{
    GENERATED_BODY()

    // Horário atual da rotina
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CurrentTimeOfDay = 0.0f;

    // Estados da rotina
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsFeeding = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsResting = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsHunting = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsMigrating = false;

    // Localizações preferidas
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector FeedingLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector RestingLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector WaterSource = FVector::ZeroVector;

    // Necessidades básicas
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float HungerLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ThirstLevel = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float EnergyLevel = 0.8f;
};

/**
 * Manager principal para simulação de crowds de dinossauros
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UJurassicCrowdManager : public UObject
{
    GENERATED_BODY()

public:
    UJurassicCrowdManager();

    // Inicialização do sistema
    UFUNCTION(BlueprintCallable, Category = "Jurassic Crowd")
    void InitializeCrowdSystem(UWorld* World);

    // Spawning de manadas
    UFUNCTION(BlueprintCallable, Category = "Jurassic Crowd")
    void SpawnHerd(FName Species, int32 HerdSize, FVector SpawnLocation, float SpawnRadius = 1000.0f);

    // Spawning de predadores solitários
    UFUNCTION(BlueprintCallable, Category = "Jurassic Crowd")
    void SpawnPredator(FName Species, FVector SpawnLocation, FVector TerritoryCenter, float TerritoryRadius = 5000.0f);

    // Sistema de domesticação
    UFUNCTION(BlueprintCallable, Category = "Jurassic Crowd")
    bool AttemptDomestication(FGuid DinosaurID, float PlayerTrustGain);

    // Queries de informação
    UFUNCTION(BlueprintCallable, Category = "Jurassic Crowd")
    TArray<FGuid> GetNearbyDinosaurs(FVector Location, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Jurassic Crowd")
    FJurassicDinosaurFragment GetDinosaurInfo(FGuid DinosaurID);

    // Sistema de memória e reconhecimento
    UFUNCTION(BlueprintCallable, Category = "Jurassic Crowd")
    void UpdatePlayerMemory(FGuid DinosaurID, FVector PlayerLocation, bool bPositiveInteraction);

protected:
    // Referência ao subsistema Mass Entity
    UPROPERTY()
    UMassEntitySubsystem* MassEntitySubsystem;

    // Configurações de spawning
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
    int32 MaxDinosaursPerSpecies = 1000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
    float MinDistanceBetweenHerds = 2000.0f;

    // Configurações de performance
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxSimultaneousAgents = 50000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LODDistance1 = 1000.0f; // Simulação completa

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LODDistance2 = 5000.0f; // Simulação simplificada

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LODDistance3 = 15000.0f; // Apenas posição

    // Tracking de manadas ativas
    UPROPERTY()
    TMap<FGuid, TArray<FMassEntityHandle>> ActiveHerds;

    // Tracking de predadores ativos
    UPROPERTY()
    TMap<FGuid, FMassEntityHandle> ActivePredators;

private:
    // Geração procedural de características únicas
    FJurassicDinosaurFragment GenerateUniqueTraits(FName Species);

    // Criação de manadas com hierarquia social
    void CreateHerdHierarchy(TArray<FMassEntityHandle>& HerdMembers, FGuid HerdID);

    // Sistema de variação genética
    void ApplyGeneticVariation(FJurassicDinosaurFragment& DinosaurTraits, FName Species);
};