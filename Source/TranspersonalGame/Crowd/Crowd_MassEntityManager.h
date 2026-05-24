#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "MassEntityTypes.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerTypes.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "MassRepresentationFragments.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "TranspersonalGame.h"
#include "Crowd_MassEntityManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_EntitySpawnData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    FVector SpawnLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    FRotator SpawnRotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float MovementSpeed = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    FVector TargetLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    int32 EntityType = 0; // 0=Human, 1=Dinosaur, 2=Animal

    FCrowd_EntitySpawnData()
    {
        SpawnLocation = FVector::ZeroVector;
        SpawnRotation = FRotator::ZeroRotator;
        MovementSpeed = 100.0f;
        TargetLocation = FVector::ZeroVector;
        EntityType = 0;
    }
};

UENUM(BlueprintType)
enum class ECrowd_BiomeType : uint8
{
    Savana     UMETA(DisplayName = "Savana"),
    Forest     UMETA(DisplayName = "Forest"),
    Desert     UMETA(DisplayName = "Desert"),
    Swamp      UMETA(DisplayName = "Swamp"),
    Mountain   UMETA(DisplayName = "Mountain")
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACrowd_MassEntityManager : public AActor
{
    GENERATED_BODY()

public:
    ACrowd_MassEntityManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Mass Entity System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Entity")
    int32 MaxEntities = 5000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Entity")
    float SpawnRadius = 10000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Entity")
    float UpdateInterval = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Entity")
    bool bAutoSpawn = true;

    // Biome Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    ECrowd_BiomeType CurrentBiome = ECrowd_BiomeType::Savana;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    TMap<ECrowd_BiomeType, FVector> BiomeLocations;

    // Entity Types
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entities")
    TArray<FCrowd_EntitySpawnData> EntityTemplates;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entities")
    UStaticMesh* HumanMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entities")
    UStaticMesh* DinosaurMesh;

    // Mass Entity Functions
    UFUNCTION(BlueprintCallable, Category = "Mass Entity")
    void SpawnMassEntities(int32 Count, ECrowd_BiomeType Biome);

    UFUNCTION(BlueprintCallable, Category = "Mass Entity")
    void ClearAllEntities();

    UFUNCTION(BlueprintCallable, Category = "Mass Entity")
    void UpdateEntityMovement();

    UFUNCTION(BlueprintCallable, Category = "Mass Entity")
    int32 GetActiveEntityCount() const;

    UFUNCTION(BlueprintCallable, Category = "Mass Entity")
    void SetBiomeConfiguration(ECrowd_BiomeType Biome, FVector Location);

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Mass Entity")
    void TestSpawnEntities();

private:
    // Mass Entity System
    UMassEntitySubsystem* MassEntitySubsystem;
    FMassArchetypeHandle EntityArchetype;
    TArray<FMassEntityHandle> SpawnedEntities;

    float LastUpdateTime = 0.0f;
    bool bSystemInitialized = false;

    // Internal Functions
    void InitializeMassSystem();
    void CreateEntityArchetype();
    FVector GetBiomeSpawnLocation(ECrowd_BiomeType Biome) const;
    void SetupBiomeLocations();
};