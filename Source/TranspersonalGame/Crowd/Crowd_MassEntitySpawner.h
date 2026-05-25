#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MassEntityTypes.h"
#include "MassSpawnerTypes.h"
#include "MassEntitySubsystem.h"
#include "Engine/World.h"
#include "Crowd_MassEntitySpawner.generated.h"

UENUM(BlueprintType)
enum class ECrowd_BiomeType : uint8
{
    Savana UMETA(DisplayName = "Savana"),
    Forest UMETA(DisplayName = "Forest"),
    Desert UMETA(DisplayName = "Desert"),
    Swamp UMETA(DisplayName = "Swamp"),
    Mountain UMETA(DisplayName = "Mountain")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_BiomeSpawnConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Config")
    ECrowd_BiomeType BiomeType = ECrowd_BiomeType::Savana;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Config")
    FVector BiomeCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Config")
    float SpawnRadius = 10000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Config")
    int32 MaxEntities = 5000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Config")
    float EntityDensity = 0.5f;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_EntitySpawnData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entity Data")
    FVector SpawnLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entity Data")
    FRotator SpawnRotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entity Data")
    ECrowd_BiomeType AssignedBiome = ECrowd_BiomeType::Savana;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entity Data")
    float MovementSpeed = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entity Data")
    bool bIsActive = true;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACrowd_MassEntitySpawner : public AActor
{
    GENERATED_BODY()

public:
    ACrowd_MassEntitySpawner();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Spawning")
    TArray<FCrowd_BiomeSpawnConfig> BiomeConfigs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Spawning")
    int32 TotalEntityLimit = 50000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Spawning")
    float SpawnInterval = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Spawning")
    bool bAutoSpawn = true;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime Data")
    TArray<FCrowd_EntitySpawnData> SpawnedEntities;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime Data")
    int32 CurrentEntityCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime Data")
    FMassEntityHandle LastSpawnedEntity;

private:
    FTimerHandle SpawnTimerHandle;
    class UMassEntitySubsystem* MassEntitySubsystem;

public:
    UFUNCTION(BlueprintCallable, Category = "Mass Spawning")
    void InitializeBiomes();

    UFUNCTION(BlueprintCallable, Category = "Mass Spawning")
    void SpawnEntitiesInBiome(ECrowd_BiomeType BiomeType, int32 Count);

    UFUNCTION(BlueprintCallable, Category = "Mass Spawning")
    void SpawnSingleEntity(const FVector& Location, ECrowd_BiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Mass Spawning")
    void ClearAllEntities();

    UFUNCTION(BlueprintCallable, Category = "Mass Spawning")
    void SetSpawnRate(float NewInterval);

    UFUNCTION(BlueprintPure, Category = "Mass Spawning")
    int32 GetEntityCountInBiome(ECrowd_BiomeType BiomeType) const;

    UFUNCTION(BlueprintPure, Category = "Mass Spawning")
    FVector GetBiomeCenter(ECrowd_BiomeType BiomeType) const;

protected:
    UFUNCTION()
    void OnSpawnTimer();

    FVector GenerateRandomLocationInBiome(const FCrowd_BiomeSpawnConfig& BiomeConfig) const;
    bool CanSpawnMoreEntities() const;
    void UpdateEntityLOD();
};