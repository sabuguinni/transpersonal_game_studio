#include "EnvArt_CretaceousFallenLogSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/StaticMesh.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

AEnvArt_CretaceousFallenLogSystem::AEnvArt_CretaceousFallenLogSystem()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 1.0f; // Update LOD every second

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Create log mesh component
    LogMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LogMeshComponent"));
    LogMeshComponent->SetupAttachment(RootComponent);

    // Initialize default properties
    LogProperties = FEnvArt_FallenLogProperties();
    
    // Performance settings
    LODDistance = 2000.0f;
    CurrentLODLevel = 1.0f;
    bIsInitialized = false;
    LastLODUpdateTime = 0.0f;

    // Set default collision
    LogMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    LogMeshComponent->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
    LogMeshComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
    LogMeshComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Block);
}

void AEnvArt_CretaceousFallenLogSystem::BeginPlay()
{
    Super::BeginPlay();
    
    if (!bIsInitialized)
    {
        InitializeFallenLog(LogProperties);
    }
}

void AEnvArt_CretaceousFallenLogSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update LOD based on distance to player
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastLODUpdateTime >= LODUpdateInterval)
    {
        APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
        if (PlayerPawn)
        {
            float DistanceToPlayer = FVector::Dist(GetActorLocation(), PlayerPawn->GetActorLocation());
            UpdateLOD(DistanceToPlayer);
        }
        LastLODUpdateTime = CurrentTime;
    }
}

void AEnvArt_CretaceousFallenLogSystem::InitializeFallenLog(const FEnvArt_FallenLogProperties& InLogProperties)
{
    LogProperties = InLogProperties;
    
    // Try to load appropriate mesh based on tree species
    UStaticMesh* LogMesh = nullptr;
    
    // Attempt to load species-specific meshes
    switch (LogProperties.TreeSpecies)
    {
        case EEnvArt_CretaceousTreeSpecies::Araucaria:
            LogMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder"));
            break;
        case EEnvArt_CretaceousTreeSpecies::Ginkgo:
            LogMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder"));
            break;
        case EEnvArt_CretaceousTreeSpecies::Cycad:
            LogMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder"));
            break;
        case EEnvArt_CretaceousTreeSpecies::Fern:
            LogMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder"));
            break;
        case EEnvArt_CretaceousTreeSpecies::Sequoia:
            LogMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder"));
            break;
        default:
            LogMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder"));
            break;
    }

    if (LogMesh && LogMeshComponent)
    {
        LogMeshComponent->SetStaticMesh(LogMesh);
        AdjustLogScale();
        ApplyMaterialBasedOnDecay();
        
        if (LogProperties.bSinkIntoTerrain)
        {
            SinkIntoTerrain(LogProperties.TerrainSinkDepth);
        }
        
        SetupCollision();
        bIsInitialized = true;
        
        UE_LOG(LogTemp, Log, TEXT("Fallen Log System initialized: %s"), *GetLogDescription());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to load log mesh for species: %d"), (int32)LogProperties.TreeSpecies);
    }
}

void AEnvArt_CretaceousFallenLogSystem::UpdateLogAppearance()
{
    if (bIsInitialized)
    {
        ApplyMaterialBasedOnDecay();
        AdjustLogScale();
    }
}

void AEnvArt_CretaceousFallenLogSystem::SetDecayState(EEnvArt_LogDecayState NewDecayState)
{
    LogProperties.DecayState = NewDecayState;
    UpdateLogAppearance();
}

void AEnvArt_CretaceousFallenLogSystem::SetTreeSpecies(EEnvArt_CretaceousTreeSpecies NewSpecies)
{
    LogProperties.TreeSpecies = NewSpecies;
    // Reinitialize with new species
    InitializeFallenLog(LogProperties);
}

void AEnvArt_CretaceousFallenLogSystem::ApplyEnvironmentalWeathering(float WeatheringAmount)
{
    LogProperties.WeatheringIntensity = FMath::Clamp(WeatheringAmount, 0.0f, 1.0f);
    
    // Increase moss based on weathering
    LogProperties.MossIntensity = FMath::Clamp(LogProperties.MossIntensity + (WeatheringAmount * 0.3f), 0.0f, 1.0f);
    
    UpdateLogAppearance();
}

void AEnvArt_CretaceousFallenLogSystem::SinkIntoTerrain(float SinkDepth)
{
    FVector CurrentLocation = GetActorLocation();
    FVector NewLocation = CurrentLocation;
    NewLocation.Z -= (LogProperties.LogDiameter * 100.0f * SinkDepth); // Convert to UE5 units
    
    SetActorLocation(NewLocation);
    LogProperties.TerrainSinkDepth = SinkDepth;
}

void AEnvArt_CretaceousFallenLogSystem::RandomizeLogProperties()
{
    // Randomize tree species
    int32 SpeciesCount = (int32)EEnvArt_CretaceousTreeSpecies::Sequoia + 1;
    LogProperties.TreeSpecies = (EEnvArt_CretaceousTreeSpecies)FMath::RandRange(0, SpeciesCount - 1);
    
    // Randomize decay state
    int32 DecayCount = (int32)EEnvArt_LogDecayState::Fossilized + 1;
    LogProperties.DecayState = (EEnvArt_LogDecayState)FMath::RandRange(0, DecayCount - 1);
    
    // Randomize physical properties
    LogProperties.LogLength = FMath::RandRange(1.5f, 4.0f);
    LogProperties.LogDiameter = FMath::RandRange(0.2f, 0.8f);
    LogProperties.MossIntensity = FMath::RandRange(0.0f, 1.0f);
    LogProperties.WeatheringIntensity = FMath::RandRange(0.3f, 1.0f);
    LogProperties.TerrainSinkDepth = FMath::RandRange(0.05f, 0.3f);
    
    // Apply randomized properties
    InitializeFallenLog(LogProperties);
    
    UE_LOG(LogTemp, Log, TEXT("Randomized fallen log: %s"), *GetLogDescription());
}

void AEnvArt_CretaceousFallenLogSystem::UpdateLOD(float DistanceToPlayer)
{
    if (DistanceToPlayer > LODDistance)
    {
        // Far LOD - reduce detail
        CurrentLODLevel = 0.5f;
        LogMeshComponent->SetVisibility(false);
    }
    else if (DistanceToPlayer > LODDistance * 0.5f)
    {
        // Medium LOD
        CurrentLODLevel = 0.75f;
        LogMeshComponent->SetVisibility(true);
    }
    else
    {
        // Near LOD - full detail
        CurrentLODLevel = 1.0f;
        LogMeshComponent->SetVisibility(true);
    }
}

FString AEnvArt_CretaceousFallenLogSystem::GetLogDescription() const
{
    FString SpeciesName;
    switch (LogProperties.TreeSpecies)
    {
        case EEnvArt_CretaceousTreeSpecies::Araucaria: SpeciesName = TEXT("Araucaria"); break;
        case EEnvArt_CretaceousTreeSpecies::Ginkgo: SpeciesName = TEXT("Ginkgo"); break;
        case EEnvArt_CretaceousTreeSpecies::Cycad: SpeciesName = TEXT("Cycad"); break;
        case EEnvArt_CretaceousTreeSpecies::Fern: SpeciesName = TEXT("Tree Fern"); break;
        case EEnvArt_CretaceousTreeSpecies::Sequoia: SpeciesName = TEXT("Dawn Redwood"); break;
        default: SpeciesName = TEXT("Unknown"); break;
    }
    
    FString DecayName;
    switch (LogProperties.DecayState)
    {
        case EEnvArt_LogDecayState::Fresh: DecayName = TEXT("Fresh"); break;
        case EEnvArt_LogDecayState::Weathered: DecayName = TEXT("Weathered"); break;
        case EEnvArt_LogDecayState::MossyCovered: DecayName = TEXT("Moss-Covered"); break;
        case EEnvArt_LogDecayState::Decomposing: DecayName = TEXT("Decomposing"); break;
        case EEnvArt_LogDecayState::Fossilized: DecayName = TEXT("Fossilized"); break;
        default: DecayName = TEXT("Unknown"); break;
    }
    
    return FString::Printf(TEXT("%s fallen log (%.1fm x %.1fm) - %s state, %.0f%% moss coverage"), 
        *SpeciesName, 
        LogProperties.LogLength, 
        LogProperties.LogDiameter, 
        *DecayName,
        LogProperties.MossIntensity * 100.0f);
}

void AEnvArt_CretaceousFallenLogSystem::ApplyMaterialBasedOnDecay()
{
    if (!LogMeshComponent)
        return;

    // Create dynamic material instance based on decay state
    UMaterialInterface* BaseMaterial = nullptr;
    
    // Try to load appropriate materials, fallback to default if not found
    switch (LogProperties.DecayState)
    {
        case EEnvArt_LogDecayState::Fresh:
            BaseMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial"));
            break;
        case EEnvArt_LogDecayState::Weathered:
        case EEnvArt_LogDecayState::MossyCovered:
        case EEnvArt_LogDecayState::Decomposing:
        case EEnvArt_LogDecayState::Fossilized:
            BaseMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial"));
            break;
        default:
            BaseMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial"));
            break;
    }

    if (BaseMaterial)
    {
        UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, this);
        if (DynamicMaterial)
        {
            // Set color based on decay state
            FLinearColor LogColor;
            switch (LogProperties.DecayState)
            {
                case EEnvArt_LogDecayState::Fresh:
                    LogColor = FLinearColor(0.6f, 0.4f, 0.2f, 1.0f); // Brown wood
                    break;
                case EEnvArt_LogDecayState::Weathered:
                    LogColor = FLinearColor(0.4f, 0.3f, 0.2f, 1.0f); // Darker brown
                    break;
                case EEnvArt_LogDecayState::MossyCovered:
                    LogColor = FLinearColor(0.2f, 0.4f, 0.2f, 1.0f); // Green-brown
                    break;
                case EEnvArt_LogDecayState::Decomposing:
                    LogColor = FLinearColor(0.3f, 0.2f, 0.1f, 1.0f); // Dark brown
                    break;
                case EEnvArt_LogDecayState::Fossilized:
                    LogColor = FLinearColor(0.5f, 0.5f, 0.4f, 1.0f); // Gray-brown
                    break;
            }
            
            // Apply color parameter if it exists
            DynamicMaterial->SetVectorParameterValue(FName("BaseColor"), LogColor);
            LogMeshComponent->SetMaterial(0, DynamicMaterial);
        }
    }
}

void AEnvArt_CretaceousFallenLogSystem::AdjustLogScale()
{
    if (LogMeshComponent)
    {
        // Scale the cylinder to match log dimensions
        // UE5 cylinder default is 1m diameter, 1m height
        FVector LogScale;
        LogScale.X = LogProperties.LogDiameter; // Diameter
        LogScale.Y = LogProperties.LogLength;   // Length (along Y-axis when rotated)
        LogScale.Z = LogProperties.LogDiameter; // Diameter
        
        LogMeshComponent->SetRelativeScale3D(LogScale);
        
        // Rotate to lie horizontally (fallen log)
        FRotator LogRotation = FRotator(0.0f, 0.0f, 90.0f); // Rotate 90 degrees around Z
        LogMeshComponent->SetRelativeRotation(LogRotation);
    }
}

void AEnvArt_CretaceousFallenLogSystem::SetupCollision()
{
    if (LogMeshComponent)
    {
        // Setup collision based on log size
        LogMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        LogMeshComponent->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
        LogMeshComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
        
        // Allow pawns to walk over small logs
        if (LogProperties.LogDiameter < 0.3f)
        {
            LogMeshComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
        }
    }
}