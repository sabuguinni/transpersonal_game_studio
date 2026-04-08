#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerSubsystem.h"
#include "MassSimulationSubsystem.h"
#include "Engine/World.h"
#include "JurassicCrowdManager.generated.h"

UENUM(BlueprintType)
enum class EHerdType : uint8
{
    SmallHerbivores,     // Compsognathus, Dryosaurus - 50-200 indivíduos
    LargeHerbivores,     // Triceratops, Brachiosaurus - 10-50 indivíduos
    FlyingCreatures,     // Pteranodon, Pterodactyl - 20-100 indivíduos
    AquaticLife,         // Peixes primitivos, plesiossauros - 100-500 indivíduos
    Insects,             // Libélulas gigantes, besouros - 1000-5000 indivíduos
    Scavengers           // Pequenos mamíferos, répteis - 20-100 indivíduos
};

USTRUCT(BlueprintType)
struct FHerdConfiguration
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EHerdType HerdType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MinGroupSize = 10;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxGroupSize = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SpawnRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MovementSpeed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CohesionRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SeparationRadius = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AlignmentRadius = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bAvoidPredators = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PredatorAvoidanceRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> PreferredBiomes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ActivityCycleHours = 24.0f; // Ciclo completo de actividade

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bNocturnal = false;
};

/**
 * Gestor central do sistema de simulação de multidões para o mundo Jurássico
 * Coordena manadas, bandos e grupos de criaturas usando Mass Entity Framework
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AJurassicCrowdManager : public AActor
{
    GENERATED_BODY()

public:
    AJurassicCrowdManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Configurações das diferentes manadas
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd Configuration")
    TArray<FHerdConfiguration> HerdConfigurations;

    // Densidade máxima de criaturas por área
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxEntitiesPerChunk = 1000;

    // Raio de activação em torno do jogador
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float PlayerActivationRadius = 5000.0f;

    // Sistema de LOD para performance
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float HighDetailRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MediumDetailRadius = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LowDetailRadius = 5000.0f;

private:
    // Referências aos subsistemas Mass
    UPROPERTY()
    UMassEntitySubsystem* MassEntitySubsystem;

    UPROPERTY()
    UMassSpawnerSubsystem* MassSpawnerSubsystem;

    UPROPERTY()
    UMassSimulationSubsystem* MassSimulationSubsystem;

    // Tracking de manadas activas
    TMap<EHerdType, TArray<FMassEntityHandle>> ActiveHerds;

    // Sistema de chunks para optimização espacial
    struct FWorldChunk
    {
        FVector2D ChunkCoordinate;
        TArray<FMassEntityHandle> Entities;
        bool bIsActive = false;
        float LastUpdateTime = 0.0f;
    };

    TMap<FVector2D, FWorldChunk> WorldChunks;
    float ChunkSize = 2000.0f;

public:
    // Funções públicas para controlo das manadas
    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    void SpawnHerd(EHerdType HerdType, FVector Location, int32 Count = -1);

    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    void DespawnHerd(EHerdType HerdType, FVector Location, float Radius = 1000.0f);

    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    void SetHerdBehaviorState(EHerdType HerdType, const FString& NewState);

    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    int32 GetActiveEntityCount() const;

    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    TArray<FVector> GetNearbyHerdLocations(FVector PlayerLocation, float Radius = 2000.0f) const;

    // Sistema de reacção a eventos
    UFUNCTION(BlueprintCallable, Category = "Event Response")
    void TriggerPredatorAlert(FVector PredatorLocation, float AlertRadius = 2000.0f);

    UFUNCTION(BlueprintCallable, Category = "Event Response")
    void TriggerStampedeEvent(FVector EpicenterLocation, FVector Direction, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Event Response")
    void TriggerWeatherResponse(const FString& WeatherType, float Intensity = 1.0f);

private:
    // Funções internas de gestão
    void UpdateChunkSystem();
    void UpdateHerdBehaviors(float DeltaTime);
    void ProcessLODSystem();
    FVector2D GetChunkCoordinate(FVector WorldLocation) const;
    void ActivateChunk(const FVector2D& ChunkCoord);
    void DeactivateChunk(const FVector2D& ChunkCoord);
    
    // Comportamentos emergentes
    void ProcessMigrationPatterns();
    void ProcessFeedingBehaviors();
    void ProcessSocialInteractions();
    void ProcessEnvironmentalResponses();
};