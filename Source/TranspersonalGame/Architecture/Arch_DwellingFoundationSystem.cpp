#include "Arch_DwellingFoundationSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Materials/MaterialInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"

AArch_DwellingFoundationActor::AArch_DwellingFoundationActor()
{
    PrimaryActorTick.bCanEverTick = false;

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    RootComponent = RootSceneComponent;

    // Create foundation mesh component
    FoundationMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FoundationMesh"));
    FoundationMesh->SetupAttachment(RootComponent);
    FoundationMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    FoundationMesh->SetCollisionResponseToAllChannels(ECR_Block);

    // Create foundation bounds
    FoundationBounds = CreateDefaultSubobject<UBoxComponent>(TEXT("FoundationBounds"));
    FoundationBounds->SetupAttachment(RootComponent);
    FoundationBounds->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    FoundationBounds->SetCollisionResponseToAllChannels(ECR_Ignore);
    FoundationBounds->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    // Initialize default properties
    FoundationConfig = FArch_FoundationProperties();

    // Set default box extent
    FoundationBounds->SetBoxExtent(FVector(FoundationConfig.Diameter * 0.5f, FoundationConfig.Diameter * 0.5f, FoundationConfig.Height));
}

void AArch_DwellingFoundationActor::BeginPlay()
{
    Super::BeginPlay();

    // Register with subsystem
    if (UArch_DwellingFoundationSubsystem* FoundationSubsystem = GetWorld()->GetSubsystem<UArch_DwellingFoundationSubsystem>())
    {
        FoundationSubsystem->RegisterFoundation(this);
    }

    // Generate foundation if not already done
    if (!FoundationMesh->GetStaticMesh())
    {
        GenerateFoundation();
    }
}

void AArch_DwellingFoundationActor::GenerateFoundation()
{
    // Clear existing components
    for (UStaticMeshComponent* Component : StoneComponents)
    {
        if (IsValid(Component))
        {
            Component->DestroyComponent();
        }
    }
    StoneComponents.Empty();

    // Generate foundation based on type
    switch (FoundationConfig.FoundationType)
    {
        case EArch_FoundationType::CircularStone:
            CreateCircularStoneFoundation();
            break;
        case EArch_FoundationType::RectangularStone:
            CreateRectangularStoneFoundation();
            break;
        case EArch_FoundationType::NaturalRock:
            CreateNaturalRockFoundation();
            break;
        case EArch_FoundationType::EarthenMound:
            CreateEarthenMoundFoundation();
            break;
        case EArch_FoundationType::WoodPlatform:
            CreateWoodPlatformFoundation();
            break;
        case EArch_FoundationType::CaveFloor:
            CreateCaveFloorFoundation();
            break;
    }

    // Apply weathering effects
    ApplyWeathering();
    
    if (FoundationConfig.bHasMossGrowth)
    {
        AddMossGrowth();
    }
    
    if (FoundationConfig.bPartiallyBuried)
    {
        ApplyBurialEffect();
    }

    // Update bounds
    FoundationBounds->SetBoxExtent(FVector(FoundationConfig.Diameter * 0.5f, FoundationConfig.Diameter * 0.5f, FoundationConfig.Height));
}

void AArch_DwellingFoundationActor::CreateCircularStoneFoundation()
{
    if (StoneBlockMeshes.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("No stone block meshes available for foundation"));
        return;
    }

    float Radius = FoundationConfig.Diameter * 0.5f;
    float AngleStep = 360.0f / FoundationConfig.StoneBlockCount;

    for (int32 i = 0; i < FoundationConfig.StoneBlockCount; i++)
    {
        UStaticMeshComponent* StoneComponent = CreateDefaultSubobject<UStaticMeshComponent>(*FString::Printf(TEXT("StoneBlock_%d"), i));
        StoneComponent->SetupAttachment(RootComponent);
        
        // Set random stone mesh
        int32 MeshIndex = FMath::RandRange(0, StoneBlockMeshes.Num() - 1);
        StoneComponent->SetStaticMesh(StoneBlockMeshes[MeshIndex]);

        // Calculate position
        float Angle = FMath::DegreesToRadians(AngleStep * i);
        FVector Position = FVector(
            FMath::Cos(Angle) * Radius,
            FMath::Sin(Angle) * Radius,
            0.0f
        );

        // Add some randomness
        Position += FVector(
            FMath::RandRange(-20.0f, 20.0f),
            FMath::RandRange(-20.0f, 20.0f),
            FMath::RandRange(-10.0f, 10.0f)
        );

        StoneComponent->SetRelativeLocation(Position);

        // Random rotation
        FRotator Rotation = FRotator(
            FMath::RandRange(-15.0f, 15.0f),
            FMath::RandRange(0.0f, 360.0f),
            FMath::RandRange(-10.0f, 10.0f)
        );
        StoneComponent->SetRelativeRotation(Rotation);

        // Random scale
        float Scale = FMath::RandRange(0.8f, 1.2f);
        StoneComponent->SetRelativeScale3D(FVector(Scale));

        StoneComponents.Add(StoneComponent);
    }
}

void AArch_DwellingFoundationActor::CreateRectangularStoneFoundation()
{
    if (StoneBlockMeshes.Num() == 0) return;

    float Width = FoundationConfig.Diameter;
    float Length = FoundationConfig.Diameter * 1.5f;
    int32 BlocksPerSide = FoundationConfig.StoneBlockCount / 4;

    // Create perimeter stones
    for (int32 Side = 0; Side < 4; Side++)
    {
        for (int32 Block = 0; Block < BlocksPerSide; Block++)
        {
            UStaticMeshComponent* StoneComponent = CreateDefaultSubobject<UStaticMeshComponent>(*FString::Printf(TEXT("RectStone_%d_%d"), Side, Block));
            StoneComponent->SetupAttachment(RootComponent);
            
            int32 MeshIndex = FMath::RandRange(0, StoneBlockMeshes.Num() - 1);
            StoneComponent->SetStaticMesh(StoneBlockMeshes[MeshIndex]);

            FVector Position;
            float Progress = (float)Block / (float)BlocksPerSide;

            switch (Side)
            {
                case 0: // Front
                    Position = FVector(Length * 0.5f, FMath::Lerp(-Width * 0.5f, Width * 0.5f, Progress), 0.0f);
                    break;
                case 1: // Right
                    Position = FVector(FMath::Lerp(Length * 0.5f, -Length * 0.5f, Progress), Width * 0.5f, 0.0f);
                    break;
                case 2: // Back
                    Position = FVector(-Length * 0.5f, FMath::Lerp(Width * 0.5f, -Width * 0.5f, Progress), 0.0f);
                    break;
                case 3: // Left
                    Position = FVector(FMath::Lerp(-Length * 0.5f, Length * 0.5f, Progress), -Width * 0.5f, 0.0f);
                    break;
            }

            // Add randomness
            Position += FVector(
                FMath::RandRange(-15.0f, 15.0f),
                FMath::RandRange(-15.0f, 15.0f),
                FMath::RandRange(-8.0f, 8.0f)
            );

            StoneComponent->SetRelativeLocation(Position);
            
            FRotator Rotation = FRotator(
                FMath::RandRange(-10.0f, 10.0f),
                FMath::RandRange(0.0f, 360.0f),
                FMath::RandRange(-8.0f, 8.0f)
            );
            StoneComponent->SetRelativeRotation(Rotation);

            float Scale = FMath::RandRange(0.9f, 1.1f);
            StoneComponent->SetRelativeScale3D(FVector(Scale));

            StoneComponents.Add(StoneComponent);
        }
    }
}

void AArch_DwellingFoundationActor::CreateNaturalRockFoundation()
{
    // Use larger, more irregular stones for natural look
    if (StoneBlockMeshes.Num() == 0) return;

    int32 RockCount = FMath::Max(3, FoundationConfig.StoneBlockCount / 3);
    float Radius = FoundationConfig.Diameter * 0.4f;

    for (int32 i = 0; i < RockCount; i++)
    {
        UStaticMeshComponent* RockComponent = CreateDefaultSubobject<UStaticMeshComponent>(*FString::Printf(TEXT("NaturalRock_%d"), i));
        RockComponent->SetupAttachment(RootComponent);
        
        int32 MeshIndex = FMath::RandRange(0, StoneBlockMeshes.Num() - 1);
        RockComponent->SetStaticMesh(StoneBlockMeshes[MeshIndex]);

        // Random position within foundation area
        FVector Position = FVector(
            FMath::RandRange(-Radius, Radius),
            FMath::RandRange(-Radius, Radius),
            FMath::RandRange(-20.0f, 5.0f)
        );

        RockComponent->SetRelativeLocation(Position);

        // More dramatic rotation for natural look
        FRotator Rotation = FRotator(
            FMath::RandRange(-30.0f, 30.0f),
            FMath::RandRange(0.0f, 360.0f),
            FMath::RandRange(-25.0f, 25.0f)
        );
        RockComponent->SetRelativeRotation(Rotation);

        // Varied scale for natural irregularity
        FVector Scale = FVector(
            FMath::RandRange(1.2f, 2.0f),
            FMath::RandRange(1.2f, 2.0f),
            FMath::RandRange(0.8f, 1.5f)
        );
        RockComponent->SetRelativeScale3D(Scale);

        StoneComponents.Add(RockComponent);
    }
}

void AArch_DwellingFoundationActor::CreateEarthenMoundFoundation()
{
    // Create a simple earthen mound using the foundation mesh
    if (FoundationMesh && StoneBlockMeshes.Num() > 0)
    {
        FoundationMesh->SetStaticMesh(StoneBlockMeshes[0]); // Use first mesh as base
        FoundationMesh->SetRelativeScale3D(FVector(FoundationConfig.Diameter / 100.0f, FoundationConfig.Diameter / 100.0f, FoundationConfig.Height / 50.0f));
    }
}

void AArch_DwellingFoundationActor::CreateWoodPlatformFoundation()
{
    // Similar to rectangular but with wood materials
    CreateRectangularStoneFoundation();
    
    // Apply wood materials if available
    for (UStaticMeshComponent* Component : StoneComponents)
    {
        if (WeatheredMaterials.Num() > 0)
        {
            int32 MaterialIndex = FMath::RandRange(0, WeatheredMaterials.Num() - 1);
            Component->SetMaterial(0, WeatheredMaterials[MaterialIndex]);
        }
    }
}

void AArch_DwellingFoundationActor::CreateCaveFloorFoundation()
{
    // Create a flattened area with minimal stone work
    CreateNaturalRockFoundation();
    
    // Reduce height variation for cave floor
    for (UStaticMeshComponent* Component : StoneComponents)
    {
        FVector Location = Component->GetRelativeLocation();
        Location.Z = FMath::Max(Location.Z, -5.0f); // Keep stones near ground level
        Component->SetRelativeLocation(Location);
    }
}

void AArch_DwellingFoundationActor::ApplyWeathering()
{
    if (WeatheredMaterials.Num() == 0) return;

    for (UStaticMeshComponent* Component : StoneComponents)
    {
        if (FMath::RandRange(0.0f, 1.0f) < FoundationConfig.WeatheringLevel)
        {
            int32 MaterialIndex = FMath::RandRange(0, WeatheredMaterials.Num() - 1);
            Component->SetMaterial(0, WeatheredMaterials[MaterialIndex]);
        }
    }
}

void AArch_DwellingFoundationActor::AddMossGrowth()
{
    // Moss growth effect - could be implemented with decals or material parameters
    for (UStaticMeshComponent* Component : StoneComponents)
    {
        if (FMath::RandRange(0.0f, 1.0f) < 0.6f) // 60% chance for moss
        {
            // Apply moss material or modify existing material parameters
            // This would require specific moss materials or material instances
        }
    }
}

void AArch_DwellingFoundationActor::ApplyBurialEffect()
{
    for (UStaticMeshComponent* Component : StoneComponents)
    {
        FVector Location = Component->GetRelativeLocation();
        Location.Z -= FMath::RandRange(5.0f, 25.0f); // Partially bury stones
        Component->SetRelativeLocation(Location);
    }
}

void AArch_DwellingFoundationActor::SetFoundationType(EArch_FoundationType NewType)
{
    FoundationConfig.FoundationType = NewType;
    GenerateFoundation();
}

void AArch_DwellingFoundationActor::SetWeatheringLevel(float NewLevel)
{
    FoundationConfig.WeatheringLevel = FMath::Clamp(NewLevel, 0.0f, 1.0f);
    ApplyWeathering();
}

bool AArch_DwellingFoundationActor::IsLocationSuitableForDwelling(const FVector& Location) const
{
    float Distance = FVector::Dist(GetActorLocation(), Location);
    return Distance <= GetFoundationRadius();
}

FVector AArch_DwellingFoundationActor::GetFoundationCenter() const
{
    return GetActorLocation();
}

// Subsystem Implementation
void UArch_DwellingFoundationSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    ActiveFoundations.Empty();
}

void UArch_DwellingFoundationSubsystem::Deinitialize()
{
    ActiveFoundations.Empty();
    Super::Deinitialize();
}

AArch_DwellingFoundationActor* UArch_DwellingFoundationSubsystem::CreateFoundationAtLocation(const FVector& Location, EArch_FoundationType Type)
{
    if (!ValidateFoundationPlacement(Location, 200.0f))
    {
        return nullptr;
    }

    UWorld* World = GetWorld();
    if (!World) return nullptr;

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    AArch_DwellingFoundationActor* NewFoundation = World->SpawnActor<AArch_DwellingFoundationActor>(AArch_DwellingFoundationActor::StaticClass(), Location, FRotator::ZeroRotator, SpawnParams);
    
    if (NewFoundation)
    {
        NewFoundation->SetFoundationType(Type);
        RegisterFoundation(NewFoundation);
    }

    return NewFoundation;
}

TArray<AArch_DwellingFoundationActor*> UArch_DwellingFoundationSubsystem::FindFoundationsInRadius(const FVector& Center, float Radius)
{
    TArray<AArch_DwellingFoundationActor*> FoundationsInRadius;

    for (AArch_DwellingFoundationActor* Foundation : ActiveFoundations)
    {
        if (IsValid(Foundation))
        {
            float Distance = FVector::Dist(Foundation->GetActorLocation(), Center);
            if (Distance <= Radius)
            {
                FoundationsInRadius.Add(Foundation);
            }
        }
    }

    return FoundationsInRadius;
}

bool UArch_DwellingFoundationSubsystem::ValidateFoundationPlacement(const FVector& Location, float Radius)
{
    // Check minimum distance from other foundations
    TArray<AArch_DwellingFoundationActor*> NearbyFoundations = FindFoundationsInRadius(Location, MinimumFoundationDistance);
    if (NearbyFoundations.Num() >= MaxFoundationsPerArea)
    {
        return false;
    }

    // Check terrain suitability
    return CheckTerrainSuitability(Location, Radius);
}

void UArch_DwellingFoundationSubsystem::RegisterFoundation(AArch_DwellingFoundationActor* Foundation)
{
    if (Foundation && !ActiveFoundations.Contains(Foundation))
    {
        ActiveFoundations.Add(Foundation);
    }
}

void UArch_DwellingFoundationSubsystem::UnregisterFoundation(AArch_DwellingFoundationActor* Foundation)
{
    ActiveFoundations.Remove(Foundation);
}

void UArch_DwellingFoundationSubsystem::CleanupInvalidFoundations()
{
    ActiveFoundations.RemoveAll([](AArch_DwellingFoundationActor* Foundation) {
        return !IsValid(Foundation);
    });
}

bool UArch_DwellingFoundationSubsystem::CheckTerrainSuitability(const FVector& Location, float Radius)
{
    // Basic terrain check - could be expanded with landscape queries
    UWorld* World = GetWorld();
    if (!World) return false;

    // Simple ground trace
    FHitResult HitResult;
    FVector TraceStart = Location + FVector(0, 0, 1000);
    FVector TraceEnd = Location - FVector(0, 0, 1000);

    bool bHit = World->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_WorldStatic);
    
    if (bHit)
    {
        // Check if surface is relatively flat
        float SlopeAngle = FMath::Acos(FVector::DotProduct(HitResult.Normal, FVector::UpVector));
        return FMath::RadiansToDegrees(SlopeAngle) < 30.0f; // Max 30 degree slope
    }

    return false;
}