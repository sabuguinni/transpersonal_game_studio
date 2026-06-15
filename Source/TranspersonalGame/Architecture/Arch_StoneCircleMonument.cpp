#include "Arch_StoneCircleMonument.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/Engine.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/Pawn.h"

AArch_StoneCircleMonument::AArch_StoneCircleMonument()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root scene component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Create interaction sphere
    InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
    InteractionSphere->SetupAttachment(RootComponent);
    InteractionSphere->SetSphereRadius(1200.0f);
    InteractionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    InteractionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    InteractionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    // Bind overlap events
    InteractionSphere->OnComponentBeginOverlap.AddDynamic(this, &AArch_StoneCircleMonument::OnInteractionSphereBeginOverlap);
    InteractionSphere->OnComponentEndOverlap.AddDynamic(this, &AArch_StoneCircleMonument::OnInteractionSphereEndOverlap);

    // Initialize default values
    CircleType = EArch_StoneCircleType::Ancient_Megalith;
    NumberOfStones = 8;
    CircleRadius = 1000.0f;
    StoneHeight = 400.0f;
    bEnableWindEffects = true;
    bEnableWeatherEffects = true;
    AmbientTemperatureModifier = -2.0f;
    HumidityLevel = 0.6f;
    bCanBeExamined = true;
    bProvidesShade = true;
    ShadeRadius = 800.0f;
    bIsGenerated = false;
    LastWeatherUpdate = 0.0f;

    // Initialize stone properties
    StoneProperties.WeatheringLevel = 0.7f;
    StoneProperties.MossGrowth = 0.5f;
    StoneProperties.StructuralIntegrity = 0.8f;
    StoneProperties.bHasAncientCarvings = true;
    StoneProperties.AgeInYears = 10000.0f;
}

void AArch_StoneCircleMonument::BeginPlay()
{
    Super::BeginPlay();

    // Generate the stone circle on begin play
    GenerateStoneCircle();
    
    // Apply initial weathering effects
    ApplyWeatheringEffects();
}

void AArch_StoneCircleMonument::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update environmental effects periodically
    LastWeatherUpdate += DeltaTime;
    if (LastWeatherUpdate >= 5.0f) // Update every 5 seconds
    {
        UpdateEnvironmentalEffects();
        LastWeatherUpdate = 0.0f;
    }
}

void AArch_StoneCircleMonument::GenerateStoneCircle()
{
    if (bIsGenerated)
    {
        // Clear existing stones
        for (UStaticMeshComponent* StoneMesh : StoneMeshes)
        {
            if (StoneMesh)
            {
                StoneMesh->DestroyComponent();
            }
        }
        StoneMeshes.Empty();
        StonePositions.Empty();
        StoneRotations.Empty();
    }

    // Calculate stone positions around the circle
    float AngleStep = 360.0f / NumberOfStones;
    
    for (int32 i = 0; i < NumberOfStones; i++)
    {
        float Angle = i * AngleStep;
        float RadianAngle = FMath::DegreesToRadians(Angle);
        
        // Calculate position with slight randomization
        float RandomRadiusOffset = FMath::RandRange(-100.0f, 100.0f);
        float ActualRadius = CircleRadius + RandomRadiusOffset;
        
        FVector StonePosition = FVector(
            ActualRadius * FMath::Cos(RadianAngle),
            ActualRadius * FMath::Sin(RadianAngle),
            0.0f
        );
        
        // Add slight height variation
        StonePosition.Z += FMath::RandRange(-50.0f, 50.0f);
        
        // Calculate rotation with slight randomization
        FRotator StoneRotation = FRotator(
            FMath::RandRange(-5.0f, 5.0f),  // Pitch
            Angle + FMath::RandRange(-15.0f, 15.0f),  // Yaw
            FMath::RandRange(-3.0f, 3.0f)   // Roll
        );
        
        StonePositions.Add(StonePosition);
        StoneRotations.Add(StoneRotation);
        
        CreateStoneAtPosition(StonePosition, StoneRotation, i);
    }
    
    bIsGenerated = true;
    
    UE_LOG(LogTemp, Log, TEXT("Stone Circle Generated: %d stones at radius %f"), NumberOfStones, CircleRadius);
}

void AArch_StoneCircleMonument::CreateStoneAtPosition(const FVector& Position, const FRotator& Rotation, int32 StoneIndex)
{
    // Create stone mesh component
    FString ComponentName = FString::Printf(TEXT("StoneMesh_%d"), StoneIndex);
    UStaticMeshComponent* StoneMesh = CreateDefaultSubobject<UStaticMeshComponent>(*ComponentName);
    
    if (StoneMesh)
    {
        StoneMesh->SetupAttachment(RootComponent);
        StoneMesh->SetRelativeLocation(Position);
        StoneMesh->SetRelativeRotation(Rotation);
        
        // Set collision properties
        StoneMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        StoneMesh->SetCollisionResponseToAllChannels(ECR_Block);
        
        // Scale based on stone height and add variation
        float ScaleVariation = FMath::RandRange(0.8f, 1.2f);
        float HeightScale = (StoneHeight / 200.0f) * ScaleVariation; // Assuming base mesh is ~200 units tall
        StoneMesh->SetRelativeScale3D(FVector(ScaleVariation, ScaleVariation, HeightScale));
        
        StoneMeshes.Add(StoneMesh);
        
        UE_LOG(LogTemp, Log, TEXT("Created stone %d at position %s"), StoneIndex, *Position.ToString());
    }
}

void AArch_StoneCircleMonument::SetCircleType(EArch_StoneCircleType NewType)
{
    CircleType = NewType;
    
    // Update properties based on type
    switch (CircleType)
    {
        case EArch_StoneCircleType::Ancient_Megalith:
            StoneProperties.WeatheringLevel = 0.8f;
            StoneProperties.MossGrowth = 0.7f;
            StoneProperties.AgeInYears = 15000.0f;
            break;
            
        case EArch_StoneCircleType::Weathered_Sandstone:
            StoneProperties.WeatheringLevel = 0.9f;
            StoneProperties.MossGrowth = 0.3f;
            StoneProperties.AgeInYears = 8000.0f;
            break;
            
        case EArch_StoneCircleType::Volcanic_Basalt:
            StoneProperties.WeatheringLevel = 0.4f;
            StoneProperties.MossGrowth = 0.2f;
            StoneProperties.AgeInYears = 12000.0f;
            break;
            
        case EArch_StoneCircleType::Limestone_Carved:
            StoneProperties.WeatheringLevel = 0.6f;
            StoneProperties.MossGrowth = 0.5f;
            StoneProperties.bHasAncientCarvings = true;
            StoneProperties.AgeInYears = 10000.0f;
            break;
            
        case EArch_StoneCircleType::Granite_Monolith:
            StoneProperties.WeatheringLevel = 0.3f;
            StoneProperties.MossGrowth = 0.4f;
            StoneProperties.AgeInYears = 20000.0f;
            break;
    }
    
    ApplyWeatheringEffects();
}

void AArch_StoneCircleMonument::UpdateStoneProperties(const FArch_StoneProperties& NewProperties)
{
    StoneProperties = NewProperties;
    ApplyWeatheringEffects();
}

float AArch_StoneCircleMonument::GetTemperatureModifier() const
{
    float BaseModifier = AmbientTemperatureModifier;
    
    // Adjust based on stone type
    switch (CircleType)
    {
        case EArch_StoneCircleType::Volcanic_Basalt:
            BaseModifier += 1.0f; // Basalt retains heat
            break;
        case EArch_StoneCircleType::Limestone_Carved:
            BaseModifier -= 1.0f; // Limestone stays cool
            break;
        default:
            break;
    }
    
    return BaseModifier;
}

bool AArch_StoneCircleMonument::IsPlayerInShade(const FVector& PlayerLocation) const
{
    if (!bProvidesShade)
        return false;
        
    float DistanceToCenter = FVector::Dist2D(GetActorLocation(), PlayerLocation);
    return DistanceToCenter <= ShadeRadius;
}

FString AArch_StoneCircleMonument::GetMonumentDescription() const
{
    FString TypeName;
    switch (CircleType)
    {
        case EArch_StoneCircleType::Ancient_Megalith:
            TypeName = TEXT("Ancient Megalithic");
            break;
        case EArch_StoneCircleType::Weathered_Sandstone:
            TypeName = TEXT("Weathered Sandstone");
            break;
        case EArch_StoneCircleType::Volcanic_Basalt:
            TypeName = TEXT("Volcanic Basalt");
            break;
        case EArch_StoneCircleType::Limestone_Carved:
            TypeName = TEXT("Carved Limestone");
            break;
        case EArch_StoneCircleType::Granite_Monolith:
            TypeName = TEXT("Granite Monolithic");
            break;
    }
    
    return FString::Printf(TEXT("%s Stone Circle - %d stones, %.0f years old, %.1f%% weathered"),
        *TypeName, NumberOfStones, StoneProperties.AgeInYears, StoneProperties.WeatheringLevel * 100.0f);
}

void AArch_StoneCircleMonument::ApplyWeatheringEffects()
{
    // Apply weathering effects to all stone meshes
    for (UStaticMeshComponent* StoneMesh : StoneMeshes)
    {
        if (StoneMesh)
        {
            // Here you would apply material parameters based on weathering level
            // This is a placeholder for material parameter setting
            UE_LOG(LogTemp, Log, TEXT("Applied weathering level %.2f to stone"), StoneProperties.WeatheringLevel);
        }
    }
}

void AArch_StoneCircleMonument::UpdateEnvironmentalEffects()
{
    if (bEnableWeatherEffects)
    {
        // Update moss growth based on humidity
        float NewMossGrowth = FMath::Clamp(StoneProperties.MossGrowth + (HumidityLevel - 0.5f) * 0.01f, 0.0f, 1.0f);
        StoneProperties.MossGrowth = NewMossGrowth;
        
        // Update weathering based on age
        float WeatheringIncrease = 0.0001f; // Very slow weathering
        StoneProperties.WeatheringLevel = FMath::Clamp(StoneProperties.WeatheringLevel + WeatheringIncrease, 0.0f, 1.0f);
    }
}

void AArch_StoneCircleMonument::OnInteractionSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (APawn* PlayerPawn = Cast<APawn>(OtherActor))
    {
        OnPlayerEnterCircle(PlayerPawn);
        UE_LOG(LogTemp, Log, TEXT("Player entered stone circle: %s"), *GetMonumentDescription());
    }
}

void AArch_StoneCircleMonument::OnInteractionSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex)
{
    if (APawn* PlayerPawn = Cast<APawn>(OtherActor))
    {
        OnPlayerExitCircle(PlayerPawn);
        UE_LOG(LogTemp, Log, TEXT("Player exited stone circle"));
    }
}