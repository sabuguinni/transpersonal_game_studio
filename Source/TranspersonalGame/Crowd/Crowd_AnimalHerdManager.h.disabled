#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "../SharedTypes.h"
#include "Crowd_AnimalHerdManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_HerdAnimal
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd Animal")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd Animal")
    FRotator Rotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd Animal")
    FVector Velocity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd Animal")
    float MovementSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd Animal")
    float MaxSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd Animal")
    ECrowd_AnimalSpecies Species;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd Animal")
    ECrowd_AnimalBehavior CurrentBehavior;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd Animal")
    float FearLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd Animal")
    float HungerLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd Animal")
    FVector FleeTarget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd Animal")
    bool bIsAlpha;

    FCrowd_HerdAnimal()
    {
        Location = FVector::ZeroVector;
        Rotation = FRotator::ZeroRotator;
        Velocity = FVector::ZeroVector;
        MovementSpeed = 200.0f;
        MaxSpeed = 400.0f;
        Species = ECrowd_AnimalSpecies::Herbivore;
        CurrentBehavior = ECrowd_AnimalBehavior::Grazing;
        FearLevel = 0.0f;
        HungerLevel = 0.5f;
        FleeTarget = FVector::ZeroVector;
        bIsAlpha = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_AnimalHerd
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animal Herd")
    TArray<FCrowd_HerdAnimal> Animals;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animal Herd")
    FVector HerdCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animal Herd")
    float HerdRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animal Herd")
    ECrowd_AnimalSpecies HerdSpecies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animal Herd")
    ECrowd_AnimalBehavior HerdBehavior;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animal Herd")
    FVector MigrationTarget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animal Herd")
    float CohesionStrength;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animal Herd")
    float SeparationDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animal Herd")
    float AlignmentStrength;

    FCrowd_AnimalHerd()
    {
        HerdCenter = FVector::ZeroVector;
        HerdRadius = 800.0f;
        HerdSpecies = ECrowd_AnimalSpecies::Herbivore;
        HerdBehavior = ECrowd_AnimalBehavior::Grazing;
        MigrationTarget = FVector::ZeroVector;
        CohesionStrength = 1.0f;
        SeparationDistance = 150.0f;
        AlignmentStrength = 0.8f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACrowd_AnimalHerdManager : public AActor
{
    GENERATED_BODY()

public:
    ACrowd_AnimalHerdManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animal Herds")
    int32 MaxAnimalsPerHerd;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animal Herds")
    int32 MaxHerdCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animal Herds")
    float SpawnRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animal Herds")
    TArray<FCrowd_AnimalHerd> AnimalHerds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animal Herds")
    TArray<UStaticMeshComponent*> AnimalMeshComponents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animal Herds")
    UStaticMesh* HerbivoreMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animal Herds")
    UStaticMesh* CarnivoreMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animal Herds")
    UMaterialInterface* HerbivoreMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animal Herds")
    UMaterialInterface* CarnivoreMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animal Herds")
    float UpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animal Herds")
    float BehaviorChangeChance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animal Herds")
    float PredatorDetectionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animal Herds")
    float FlockingUpdateRate;

    UFUNCTION(BlueprintCallable, Category = "Animal Herds")
    void SpawnAnimalHerd(FVector CenterLocation, ECrowd_AnimalSpecies Species, int32 HerdSize);

    UFUNCTION(BlueprintCallable, Category = "Animal Herds")
    void UpdateHerdBehavior();

    UFUNCTION(BlueprintCallable, Category = "Animal Herds")
    void TriggerPredatorResponse(FVector PredatorLocation, float ThreatRadius);

    UFUNCTION(BlueprintCallable, Category = "Animal Herds")
    void SetHerdMigrationTarget(int32 HerdIndex, FVector TargetLocation);

    UFUNCTION(BlueprintCallable, Category = "Animal Herds")
    void UpdateAnimalFlocking(int32 HerdIndex, float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Animal Herds")
    FVector CalculateFlockingForces(const FCrowd_HerdAnimal& Animal, const FCrowd_AnimalHerd& Herd);

    UFUNCTION(BlueprintCallable, Category = "Animal Herds")
    void HandleEnvironmentalStimuli(FVector StimulusLocation, float StimulusStrength);

    UFUNCTION(BlueprintCallable, Category = "Animal Herds")
    FVector GetRandomGrazingLocation(FVector CurrentLocation, float SearchRadius);

    UFUNCTION(BlueprintCallable, Category = "Animal Herds")
    void UpdateAnimalStates(FCrowd_AnimalHerd& Herd, float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Animal Herds")
    void ApplyHerdBehavior(FCrowd_AnimalHerd& Herd, float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Animal Herds")
    bool IsLocationSafe(FVector Location, float SafetyRadius);

private:
    FTimerHandle BehaviorUpdateTimer;
    FTimerHandle FlockingUpdateTimer;
    
    void InitializeAnimalMeshes();
    void UpdateAnimalMovement(FCrowd_HerdAnimal& Animal, float DeltaTime);
    void UpdateAnimalVisuals();
    FVector CalculateCohesionForce(const FCrowd_HerdAnimal& Animal, const FCrowd_AnimalHerd& Herd);
    FVector CalculateSeparationForce(const FCrowd_HerdAnimal& Animal, const FCrowd_AnimalHerd& Herd);
    FVector CalculateAlignmentForce(const FCrowd_HerdAnimal& Animal, const FCrowd_AnimalHerd& Herd);
    void UpdateHerdCenter(FCrowd_AnimalHerd& Herd);
    void SpawnAnimalMeshComponent(const FCrowd_HerdAnimal& Animal, int32 HerdIndex, int32 AnimalIndex);
};