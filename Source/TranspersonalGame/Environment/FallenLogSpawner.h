#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FallenLogSpawner.generated.h"

/**
 * EEnvArt_LogState — condition of a fallen log prop in the world.
 * Used by the foliage/environment system to vary visual appearance.
 */
UENUM(BlueprintType)
enum class EEnvArt_LogState : uint8
{
	Fresh       UMETA(DisplayName = "Fresh"),
	Mossy       UMETA(DisplayName = "Mossy"),
	Rotting     UMETA(DisplayName = "Rotting"),
	Hollow      UMETA(DisplayName = "Hollow"),
	Submerged   UMETA(DisplayName = "Submerged")
};

/**
 * FEnvArt_LogSpawnConfig — configuration for a single fallen log placement.
 */
USTRUCT(BlueprintType)
struct FEnvArt_LogSpawnConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment|Log")
	FVector Location = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment|Log")
	FRotator Rotation = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment|Log")
	FVector Scale = FVector(4.0f, 0.6f, 0.6f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment|Log")
	EEnvArt_LogState LogState = EEnvArt_LogState::Mossy;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment|Log")
	float NarrativeRadius = 200.0f;
};

/**
 * AEnvArt_FallenLogSpawner — places fallen log props in the world.
 * Designed for Cretaceous jungle biome narrative storytelling:
 * logs serve as environmental storytelling elements (shelter, obstacles, habitat).
 * Agent #06 — Environment Artist
 */
UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Fallen Log Spawner"))
class TRANSPERSONALGAME_API AEnvArt_FallenLogSpawner : public AActor
{
	GENERATED_BODY()

public:
	AEnvArt_FallenLogSpawner();

	/** Mesh to use for fallen log props */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment|Log")
	TObjectPtr<UStaticMesh> LogMesh;

	/** Array of log placement configurations */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment|Log")
	TArray<FEnvArt_LogSpawnConfig> LogConfigs;

	/** Whether to spawn logs on BeginPlay */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment|Log")
	bool bSpawnOnBeginPlay = true;

	/** Density multiplier — scales number of logs spawned from LogConfigs */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment|Log", meta = (ClampMin = "0.1", ClampMax = "3.0"))
	float DensityMultiplier = 1.0f;

	/** Spawns all logs defined in LogConfigs */
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Environment|Log")
	void SpawnLogs();

	/** Clears all spawned log actors from this spawner */
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Environment|Log")
	void ClearSpawnedLogs();

	/** Returns number of currently spawned logs */
	UFUNCTION(BlueprintPure, Category = "Environment|Log")
	int32 GetSpawnedLogCount() const;

	/** Adds a log config at the given world location */
	UFUNCTION(BlueprintCallable, Category = "Environment|Log")
	void AddLogAtLocation(FVector InLocation, FRotator InRotation, EEnvArt_LogState InState);

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY()
	TArray<TObjectPtr<AActor>> SpawnedLogActors;
};
