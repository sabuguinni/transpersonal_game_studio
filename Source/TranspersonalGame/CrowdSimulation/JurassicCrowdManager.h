#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerSubsystem.h"
#include "Engine/World.h"
#include "JurassicCrowdManager.generated.h"

UENUM(BlueprintType)
enum class EDinosaurHerdType : uint8
{
    SmallHerbivore,     // 5-15 indivíduos (Compsognathus, Dryosaurus)
    MediumHerbivore,    // 10-30 indivíduos (Parasaurolophus, Triceratops)
    LargeHerbivore,     // 5-20 indivíduos (Brontosaurus, Diplodocus)
    SmallCarnivore,     // 2-5 indivíduos (Velociraptor, Deinonychus)
    LargeCarnivore,     // 1-3 indivíduos (T-Rex, Allosaurus)
    FlyingHerd,         // 10-100 indivíduos (Pteranodon, Quetzalcoatlus)
    AquaticGroup        // 5-30 indivíduos (Plesiosaurs, Mosasaurus)
};

USTRUCT(BlueprintType)
struct FHerdBehaviorProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EDinosaurHerdType HerdType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MinHerdSize = 5;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxHerdSize = 30;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CohesionRadius = 1000.0f;  // Raio de coesão do grupo

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SeparationRadius = 200.0f; // Distância mínima entre indivíduos

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AlertRadius = 2000.0f;     // Raio de detecção de predadores

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MovementSpeed = 300.0f;    // Velocidade base de movimento

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PanicSpeedMultiplier = 3.0f; // Multiplicador de velocidade em pânico

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsNocturnal = false;       // Ativo durante a noite

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<EDinosaurHerdType> PreyTypes; // Tipos que este grupo caça

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<EDinosaurHerdType> PredatorTypes; // Tipos que caçam este grupo
};

USTRUCT(BlueprintType)
struct FEcosystemZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector ZoneCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ZoneRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FHerdBehaviorProfile> AllowedHerdTypes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxTotalPopulation = 1000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float WaterAccessDistance = 2000.0f; // Distância máxima da água

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float VegetationDensity = 1.0f;      // 0.0 = deserto, 1.0 = floresta densa
};

/**
 * Gerenciador principal do sistema de simulação de multidões para o mundo Jurássico
 * Utiliza Mass AI do UE5 para simular até 50.000 criaturas simultâneas
 */
UCLASS()
class TRANSPERSONALGAME_API AJurassicCrowdManager : public AActor
{
    GENERATED_BODY()
    
public:    
    AJurassicCrowdManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Configuração dos perfis de comportamento
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd Behavior")
    TArray<FHerdBehaviorProfile> HerdProfiles;

    // Zonas do ecossistema
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecosystem")
    TArray<FEcosystemZone> EcosystemZones;

    // Configurações de performance
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxSimultaneousEntities = 50000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LODDistance1 = 2000.0f;  // Distância para LOD máximo

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LODDistance2 = 5000.0f;  // Distância para LOD médio

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LODDistance3 = 10000.0f; // Distância para LOD mínimo

    // Ciclo dia/noite
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Cycle")
    bool bEnableDayNightBehavior = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Cycle")
    float DawnHour = 6.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Cycle")
    float DuskHour = 18.0f;

private:
    // Subsistemas Mass
    UPROPERTY()
    UMassEntitySubsystem* MassEntitySubsystem;

    UPROPERTY()
    UMassSpawnerSubsystem* MassSpawnerSubsystem;

    // Estado interno
    float CurrentTimeOfDay = 12.0f; // Meio-dia por defeito
    bool bIsNightTime = false;

    // Métodos internos
    void InitializeMassSystem();
    void SpawnInitialHerds();
    void UpdateHerdBehaviors(float DeltaTime);
    void UpdateDayNightCycle(float DeltaTime);
    void ProcessEcosystemInteractions();
    
    // Gestão de LOD
    void UpdateLODSystem();
    int32 CalculateLODLevel(float DistanceToPlayer);

public:
    // Interface pública
    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    void SpawnHerdAtLocation(EDinosaurHerdType HerdType, FVector Location, int32 HerdSize = -1);

    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    void TriggerPanicInRadius(FVector Location, float Radius, EDinosaurHerdType ThreatType);

    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    void SetTimeOfDay(float Hour);

    UFUNCTION(BlueprintPure, Category = "Crowd Management")
    int32 GetActiveEntityCount() const;

    UFUNCTION(BlueprintPure, Category = "Crowd Management")
    bool IsNightTime() const { return bIsNightTime; }
};