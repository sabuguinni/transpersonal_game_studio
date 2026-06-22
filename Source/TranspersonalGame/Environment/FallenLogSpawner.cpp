// FallenLogSpawner.cpp
// Agent #06 — Environment Artist | PROD_CYCLE_AUTO_20260622_005
// Cretaceous jungle fallen log placement system for narrative environment storytelling.

#include "FallenLogSpawner.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

AEnvArt_FallenLogSpawner::AEnvArt_FallenLogSpawner()
{
	PrimaryActorTick.bCanEverTick = false;

	// Default log configs for Cretaceous jungle NW biome
	// Positions match MinPlayableMap jungle quadrant (-6000 to -3000, -7000 to -4000)
	FEnvArt_LogSpawnConfig Config1;
	Config1.Location = FVector(-6000.0f, -6500.0f, 150.0f);
	Config1.Rotation = FRotator(0.0f, 0.0f, 0.0f);
	Config1.Scale = FVector(4.0f, 0.6f, 0.6f);
	Config1.LogState = EEnvArt_LogState::Mossy;
	Config1.NarrativeRadius = 200.0f;
	LogConfigs.Add(Config1);

	FEnvArt_LogSpawnConfig Config2;
	Config2.Location = FVector(-4000.0f, -5000.0f, 120.0f);
	Config2.Rotation = FRotator(0.0f, 45.0f, 0.0f);
	Config2.Scale = FVector(3.5f, 0.5f, 0.5f);
	Config2.LogState = EEnvArt_LogState::Rotting;
	Config2.NarrativeRadius = 150.0f;
	LogConfigs.Add(Config2);

	FEnvArt_LogSpawnConfig Config3;
	Config3.Location = FVector(-3000.0f, -7000.0f, 130.0f);
	Config3.Rotation = FRotator(0.0f, 90.0f, 0.0f);
	Config3.Scale = FVector(5.0f, 0.7f, 0.7f);
	Config3.LogState = EEnvArt_LogState::Hollow;
	Config3.NarrativeRadius = 300.0f;
	LogConfigs.Add(Config3);
}

void AEnvArt_FallenLogSpawner::BeginPlay()
{
	Super::BeginPlay();

	if (bSpawnOnBeginPlay)
	{
		SpawnLogs();
	}
}

void AEnvArt_FallenLogSpawner::SpawnLogs()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const int32 CountToSpawn = FMath::RoundToInt(static_cast<float>(LogConfigs.Num()) * DensityMultiplier);
	const int32 ActualCount = FMath::Min(CountToSpawn, LogConfigs.Num());

	for (int32 i = 0; i < ActualCount; ++i)
	{
		const FEnvArt_LogSpawnConfig& Config = LogConfigs[i];

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		AStaticMeshActor* LogActor = World->SpawnActor<AStaticMeshActor>(
			AStaticMeshActor::StaticClass(),
			Config.Location,
			Config.Rotation,
			SpawnParams
		);

		if (LogActor)
		{
			LogActor->SetActorScale3D(Config.Scale);

			if (LogMesh)
			{
				UStaticMeshComponent* MeshComp = LogActor->GetStaticMeshComponent();
				if (MeshComp)
				{
					MeshComp->SetStaticMesh(LogMesh);
				}
			}

			SpawnedLogActors.Add(LogActor);
		}
	}
}

void AEnvArt_FallenLogSpawner::ClearSpawnedLogs()
{
	for (TObjectPtr<AActor>& LogActor : SpawnedLogActors)
	{
		if (IsValid(LogActor))
		{
			LogActor->Destroy();
		}
	}
	SpawnedLogActors.Empty();
}

int32 AEnvArt_FallenLogSpawner::GetSpawnedLogCount() const
{
	int32 ValidCount = 0;
	for (const TObjectPtr<AActor>& LogActor : SpawnedLogActors)
	{
		if (IsValid(LogActor))
		{
			++ValidCount;
		}
	}
	return ValidCount;
}

void AEnvArt_FallenLogSpawner::AddLogAtLocation(FVector InLocation, FRotator InRotation, EEnvArt_LogState InState)
{
	FEnvArt_LogSpawnConfig NewConfig;
	NewConfig.Location = InLocation;
	NewConfig.Rotation = InRotation;
	NewConfig.LogState = InState;
	NewConfig.Scale = FVector(4.0f, 0.6f, 0.6f);
	NewConfig.NarrativeRadius = 200.0f;
	LogConfigs.Add(NewConfig);
}
