#include "Arch_PrehistoricArchitecture.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Engine/Engine.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"

AArch_PrehistoricArchitecture::AArch_PrehistoricArchitecture()
{
    PrimaryActorTick.bCanEverTick = true;

    // Initialize root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Initialize interaction trigger
    InteractionTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionTrigger"));
    InteractionTrigger->SetupAttachment(RootComponent);
    InteractionTrigger->SetBoxExtent(FVector(300.0f, 300.0f, 200.0f));
    InteractionTrigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    InteractionTrigger->SetCollisionResponseToAllChannels(ECR_Ignore);
    InteractionTrigger->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    // Initialize default structure properties
    StructureProperties.StructureType = EArch_StructureType::StoneCircle;
    StructureProperties.WeatheringLevel = EArch_WeatheringLevel::Weathered;
    StructureProperties.StructureScale = 1.0f;
    StructureProperties.StoneCount = 8;
    StructureProperties.CircleRadius = 500.0f;
    StructureProperties.bHasMossGrowth = true;
    StructureProperties.bHasLichenGrowth = true;
    StructureProperties.MossIntensity = 0.6f;
    StructureProperties.LichenIntensity = 0.4f;

    // Initialize interaction data
    InteractionData.bIsInteractable = true;
    InteractionData.InteractionPrompt = TEXT("Examine ancient structure");
    InteractionData.InteractionRange = 300.0f;
    InteractionData.bProvidesWeatherShelter = false;
    InteractionData.ShelterRadius = 200.0f;
    InteractionData.WeatherProtection = 0.3f;

    // Initialize state
    bIsPlayerInRange = false;
    LastInteractionTime = 0.0f;
}

void AArch_PrehistoricArchitecture::BeginPlay()
{
    Super::BeginPlay();

    InitializeComponents();
    GenerateStructure();
    UpdateStructureAppearance();
}

void AArch_PrehistoricArchitecture::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update environmental effects periodically
    static float EnvironmentUpdateTimer = 0.0f;
    EnvironmentUpdateTimer += DeltaTime;
    if (EnvironmentUpdateTimer >= 5.0f)
    {
        UpdateEnvironmentalEffects();
        EnvironmentUpdateTimer = 0.0f;
    }
}

void AArch_PrehistoricArchitecture::InitializeComponents()
{
    // Setup interaction trigger size based on interaction range
    if (InteractionTrigger)
    {
        float TriggerSize = InteractionData.InteractionRange;
        InteractionTrigger->SetBoxExtent(FVector(TriggerSize, TriggerSize, TriggerSize * 0.5f));
    }
}

void AArch_PrehistoricArchitecture::SetupInteractionTrigger()
{
    if (InteractionTrigger)
    {
        // Bind overlap events
        InteractionTrigger->OnComponentBeginOverlap.AddDynamic(this, &AArch_PrehistoricArchitecture::OnPlayerEnterRange);
        InteractionTrigger->OnComponentEndOverlap.AddDynamic(this, &AArch_PrehistoricArchitecture::OnPlayerExitRange);
    }
}

void AArch_PrehistoricArchitecture::GenerateStructure()
{
    // Clear existing structure
    ClearStructure();

    // Generate based on structure type
    switch (StructureProperties.StructureType)
    {
        case EArch_StructureType::StoneCircle:
            CreateStoneCircle();
            break;
        case EArch_StructureType::MegalithDolmen:
            CreateMegalithDolmen();
            break;
        case EArch_StructureType::RockFormation:
            CreateRockFormation();
            break;
        case EArch_StructureType::AncientRuins:
            CreateAncientRuins();
            break;
        default:
            CreateStoneCircle();
            break;
    }

    // Apply weathering and environmental effects
    ApplyWeatheringToStones();
    SpawnMossAndLichen();
}

void AArch_PrehistoricArchitecture::ClearStructure()
{
    // Destroy existing stone mesh components
    for (UStaticMeshComponent* StoneMesh : StoneMeshComponents)
    {
        if (StoneMesh)
        {
            StoneMesh->DestroyComponent();
        }
    }
    StoneMeshComponents.Empty();
    StonePositions.Empty();
    StoneRotations.Empty();
}

void AArch_PrehistoricArchitecture::CreateStoneCircle()
{
    int32 StoneCount = FMath::Clamp(StructureProperties.StoneCount, 3, 20);
    float Radius = StructureProperties.CircleRadius * StructureProperties.StructureScale;

    for (int32 i = 0; i < StoneCount; i++)
    {
        // Calculate position around circle
        FVector StonePosition = CalculateStonePosition(i, Radius, StoneCount);
        FRotator StoneRotation = CalculateStoneRotation(i, StonePosition);

        // Create stone mesh component
        UStaticMeshComponent* StoneMesh = CreateDefaultSubobject<UStaticMeshComponent>(*FString::Printf(TEXT("StoneMesh_%d"), i));
        if (StoneMesh)
        {
            StoneMesh->SetupAttachment(RootComponent);
            StoneMesh->SetRelativeLocation(StonePosition);
            StoneMesh->SetRelativeRotation(StoneRotation);
            
            // Set mesh if available
            if (StandingStoneMesh)
            {
                StoneMesh->SetStaticMesh(StandingStoneMesh);
            }

            // Add random scale variation
            float ScaleVariation = FMath::RandRange(0.8f, 1.2f) * StructureProperties.StructureScale;
            StoneMesh->SetRelativeScale3D(FVector(ScaleVariation, ScaleVariation, ScaleVariation));

            StoneMeshComponents.Add(StoneMesh);
            StonePositions.Add(StonePosition);
            StoneRotations.Add(StoneRotation);
        }
    }
}

void AArch_PrehistoricArchitecture::CreateMegalithDolmen()
{
    // Create supporting stones (vertical)
    int32 SupportStoneCount = 3;
    float Radius = StructureProperties.CircleRadius * 0.5f * StructureProperties.StructureScale;

    for (int32 i = 0; i < SupportStoneCount; i++)
    {
        FVector StonePosition = CalculateStonePosition(i, Radius, SupportStoneCount);
        FRotator StoneRotation = FRotator(0.0f, i * (360.0f / SupportStoneCount), 0.0f);

        UStaticMeshComponent* StoneMesh = CreateDefaultSubobject<UStaticMeshComponent>(*FString::Printf(TEXT("SupportStone_%d"), i));
        if (StoneMesh)
        {
            StoneMesh->SetupAttachment(RootComponent);
            StoneMesh->SetRelativeLocation(StonePosition);
            StoneMesh->SetRelativeRotation(StoneRotation);
            
            if (StandingStoneMesh)
            {
                StoneMesh->SetStaticMesh(StandingStoneMesh);
            }

            float ScaleVariation = FMath::RandRange(0.9f, 1.1f) * StructureProperties.StructureScale;
            StoneMesh->SetRelativeScale3D(FVector(ScaleVariation, ScaleVariation, ScaleVariation * 1.5f));

            StoneMeshComponents.Add(StoneMesh);
            StonePositions.Add(StonePosition);
            StoneRotations.Add(StoneRotation);
        }
    }

    // Create capstone (horizontal)
    UStaticMeshComponent* CapMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CapStone"));
    if (CapMesh)
    {
        CapMesh->SetupAttachment(RootComponent);
        CapMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 300.0f * StructureProperties.StructureScale));
        CapMesh->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));
        
        if (DolmenCapMesh)
        {
            CapMesh->SetStaticMesh(DolmenCapMesh);
        }
        else if (StandingStoneMesh)
        {
            CapMesh->SetStaticMesh(StandingStoneMesh);
        }

        CapMesh->SetRelativeScale3D(FVector(2.0f, 1.5f, 0.3f) * StructureProperties.StructureScale);

        StoneMeshComponents.Add(CapMesh);
        StonePositions.Add(FVector(0.0f, 0.0f, 300.0f * StructureProperties.StructureScale));
        StoneRotations.Add(FRotator(0.0f, 0.0f, 0.0f));
    }
}

void AArch_PrehistoricArchitecture::CreateRockFormation()
{
    int32 RockCount = FMath::RandRange(5, 12);
    float FormationRadius = StructureProperties.CircleRadius * 0.7f * StructureProperties.StructureScale;

    for (int32 i = 0; i < RockCount; i++)
    {
        // Random position within formation area
        float Angle = FMath::RandRange(0.0f, 360.0f);
        float Distance = FMath::RandRange(0.0f, FormationRadius);
        FVector StonePosition = FVector(
            FMath::Cos(FMath::DegreesToRadians(Angle)) * Distance,
            FMath::Sin(FMath::DegreesToRadians(Angle)) * Distance,
            FMath::RandRange(-50.0f, 50.0f)
        );

        FRotator StoneRotation = FRotator(
            FMath::RandRange(-15.0f, 15.0f),
            FMath::RandRange(0.0f, 360.0f),
            FMath::RandRange(-10.0f, 10.0f)
        );

        UStaticMeshComponent* StoneMesh = CreateDefaultSubobject<UStaticMeshComponent>(*FString::Printf(TEXT("RockMesh_%d"), i));
        if (StoneMesh)
        {
            StoneMesh->SetupAttachment(RootComponent);
            StoneMesh->SetRelativeLocation(StonePosition);
            StoneMesh->SetRelativeRotation(StoneRotation);
            
            if (StandingStoneMesh)
            {
                StoneMesh->SetStaticMesh(StandingStoneMesh);
            }

            float ScaleVariation = FMath::RandRange(0.3f, 1.5f) * StructureProperties.StructureScale;
            StoneMesh->SetRelativeScale3D(FVector(ScaleVariation, ScaleVariation, ScaleVariation));

            StoneMeshComponents.Add(StoneMesh);
            StonePositions.Add(StonePosition);
            StoneRotations.Add(StoneRotation);
        }
    }
}

void AArch_PrehistoricArchitecture::CreateAncientRuins()
{
    // Create foundation stones
    int32 FoundationCount = 8;
    float RuinSize = StructureProperties.CircleRadius * StructureProperties.StructureScale;

    // Create perimeter foundation
    for (int32 i = 0; i < FoundationCount; i++)
    {
        FVector StonePosition = CalculateStonePosition(i, RuinSize, FoundationCount);
        StonePosition.Z = FMath::RandRange(-30.0f, 10.0f); // Partially buried
        FRotator StoneRotation = CalculateStoneRotation(i, StonePosition);

        UStaticMeshComponent* StoneMesh = CreateDefaultSubobject<UStaticMeshComponent>(*FString::Printf(TEXT("FoundationStone_%d"), i));
        if (StoneMesh)
        {
            StoneMesh->SetupAttachment(RootComponent);
            StoneMesh->SetRelativeLocation(StonePosition);
            StoneMesh->SetRelativeRotation(StoneRotation);
            
            if (RuinBlockMesh)
            {
                StoneMesh->SetStaticMesh(RuinBlockMesh);
            }
            else if (StandingStoneMesh)
            {
                StoneMesh->SetStaticMesh(StandingStoneMesh);
            }

            float ScaleVariation = FMath::RandRange(0.6f, 1.0f) * StructureProperties.StructureScale;
            StoneMesh->SetRelativeScale3D(FVector(ScaleVariation, ScaleVariation, ScaleVariation * 0.5f));

            StoneMeshComponents.Add(StoneMesh);
            StonePositions.Add(StonePosition);
            StoneRotations.Add(StoneRotation);
        }
    }

    // Add scattered ruin blocks
    int32 ScatteredBlockCount = FMath::RandRange(3, 8);
    for (int32 i = 0; i < ScatteredBlockCount; i++)
    {
        FVector BlockPosition = FVector(
            FMath::RandRange(-RuinSize * 0.8f, RuinSize * 0.8f),
            FMath::RandRange(-RuinSize * 0.8f, RuinSize * 0.8f),
            FMath::RandRange(-20.0f, 50.0f)
        );

        FRotator BlockRotation = FRotator(
            FMath::RandRange(-30.0f, 30.0f),
            FMath::RandRange(0.0f, 360.0f),
            FMath::RandRange(-20.0f, 20.0f)
        );

        UStaticMeshComponent* BlockMesh = CreateDefaultSubobject<UStaticMeshComponent>(*FString::Printf(TEXT("RuinBlock_%d"), i));
        if (BlockMesh)
        {
            BlockMesh->SetupAttachment(RootComponent);
            BlockMesh->SetRelativeLocation(BlockPosition);
            BlockMesh->SetRelativeRotation(BlockRotation);
            
            if (RuinBlockMesh)
            {
                BlockMesh->SetStaticMesh(RuinBlockMesh);
            }
            else if (StandingStoneMesh)
            {
                BlockMesh->SetStaticMesh(StandingStoneMesh);
            }

            float ScaleVariation = FMath::RandRange(0.4f, 0.8f) * StructureProperties.StructureScale;
            BlockMesh->SetRelativeScale3D(FVector(ScaleVariation, ScaleVariation, ScaleVariation));

            StoneMeshComponents.Add(BlockMesh);
            StonePositions.Add(BlockPosition);
            StoneRotations.Add(BlockRotation);
        }
    }
}

FVector AArch_PrehistoricArchitecture::CalculateStonePosition(int32 StoneIndex, float Radius, int32 TotalStones)
{
    float Angle = (360.0f / TotalStones) * StoneIndex;
    float RadianAngle = FMath::DegreesToRadians(Angle);
    
    // Add some random variation
    float RadiusVariation = FMath::RandRange(0.9f, 1.1f);
    float AngleVariation = FMath::RandRange(-5.0f, 5.0f);
    
    return FVector(
        FMath::Cos(RadianAngle + FMath::DegreesToRadians(AngleVariation)) * Radius * RadiusVariation,
        FMath::Sin(RadianAngle + FMath::DegreesToRadians(AngleVariation)) * Radius * RadiusVariation,
        FMath::RandRange(-10.0f, 10.0f)
    );
}

FRotator AArch_PrehistoricArchitecture::CalculateStoneRotation(int32 StoneIndex, const FVector& Position)
{
    // Point toward center with some variation
    FVector ToCenter = -Position.GetSafeNormal();
    FRotator BaseRotation = ToCenter.Rotation();
    
    return FRotator(
        BaseRotation.Pitch + FMath::RandRange(-10.0f, 10.0f),
        BaseRotation.Yaw + FMath::RandRange(-15.0f, 15.0f),
        FMath::RandRange(-5.0f, 5.0f)
    );
}

void AArch_PrehistoricArchitecture::UpdateStructureAppearance()
{
    ApplyWeatheringToStones();
    SpawnMossAndLichen();
}

UMaterialInterface* AArch_PrehistoricArchitecture::GetMaterialForWeathering(EArch_WeatheringLevel WeatheringLevel)
{
    switch (WeatheringLevel)
    {
        case EArch_WeatheringLevel::Fresh:
            return StoneMaterial;
        case EArch_WeatheringLevel::Weathered:
        case EArch_WeatheringLevel::HeavilyWeathered:
            return WeatheredStoneMaterial ? WeatheredStoneMaterial : StoneMaterial;
        case EArch_WeatheringLevel::Ancient:
        case EArch_WeatheringLevel::Crumbling:
            return MossyStoneMaterial ? MossyStoneMaterial : WeatheredStoneMaterial ? WeatheredStoneMaterial : StoneMaterial;
        default:
            return StoneMaterial;
    }
}

void AArch_PrehistoricArchitecture::ApplyWeatheringToStones()
{
    UMaterialInterface* SelectedMaterial = GetMaterialForWeathering(StructureProperties.WeatheringLevel);
    
    for (UStaticMeshComponent* StoneMesh : StoneMeshComponents)
    {
        if (StoneMesh && SelectedMaterial)
        {
            StoneMesh->SetMaterial(0, SelectedMaterial);
        }
    }
}

void AArch_PrehistoricArchitecture::SpawnMossAndLichen()
{
    // This would typically spawn foliage or decal components for moss and lichen
    // For now, we'll adjust material parameters if the materials support it
    
    for (UStaticMeshComponent* StoneMesh : StoneMeshComponents)
    {
        if (StoneMesh)
        {
            // Create dynamic material instance if needed
            UMaterialInstanceDynamic* DynamicMaterial = StoneMesh->CreateAndSetMaterialInstanceDynamic(0);
            if (DynamicMaterial)
            {
                if (StructureProperties.bHasMossGrowth)
                {
                    DynamicMaterial->SetScalarParameterValue(TEXT("MossIntensity"), StructureProperties.MossIntensity);
                }
                
                if (StructureProperties.bHasLichenGrowth)
                {
                    DynamicMaterial->SetScalarParameterValue(TEXT("LichenIntensity"), StructureProperties.LichenIntensity);
                }
            }
        }
    }
}

void AArch_PrehistoricArchitecture::OnPlayerEnterRange()
{
    bIsPlayerInRange = true;
    
    if (InteractionData.bIsInteractable)
    {
        // Show interaction prompt
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, 
                FString::Printf(TEXT("Press E to %s"), *InteractionData.InteractionPrompt));
        }
    }
}

void AArch_PrehistoricArchitecture::OnPlayerExitRange()
{
    bIsPlayerInRange = false;
}

void AArch_PrehistoricArchitecture::OnPlayerInteract()
{
    if (!bIsPlayerInRange || !InteractionData.bIsInteractable)
    {
        return;
    }

    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastInteractionTime < 2.0f)
    {
        return; // Prevent spam
    }
    LastInteractionTime = CurrentTime;

    // Provide information about the structure
    FString StructureInfo = GetStructureDescription();
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, StructureInfo);
    }
}

FString AArch_PrehistoricArchitecture::GetStructureDescription()
{
    FString TypeName;
    switch (StructureProperties.StructureType)
    {
        case EArch_StructureType::StoneCircle:
            TypeName = TEXT("Ancient Stone Circle");
            break;
        case EArch_StructureType::MegalithDolmen:
            TypeName = TEXT("Megalithic Dolmen");
            break;
        case EArch_StructureType::RockFormation:
            TypeName = TEXT("Natural Rock Formation");
            break;
        case EArch_StructureType::AncientRuins:
            TypeName = TEXT("Ancient Ruins");
            break;
        default:
            TypeName = TEXT("Mysterious Structure");
            break;
    }

    FString WeatheringDesc;
    switch (StructureProperties.WeatheringLevel)
    {
        case EArch_WeatheringLevel::Fresh:
            WeatheringDesc = TEXT("recently constructed");
            break;
        case EArch_WeatheringLevel::Weathered:
            WeatheringDesc = TEXT("weathered by time");
            break;
        case EArch_WeatheringLevel::HeavilyWeathered:
            WeatheringDesc = TEXT("heavily weathered");
            break;
        case EArch_WeatheringLevel::Ancient:
            WeatheringDesc = TEXT("ancient and worn");
            break;
        case EArch_WeatheringLevel::Crumbling:
            WeatheringDesc = TEXT("crumbling with age");
            break;
    }

    return FString::Printf(TEXT("%s - This structure appears %s. %s"), 
        *TypeName, 
        *WeatheringDesc,
        StructureProperties.bHasMossGrowth ? TEXT("Moss and lichen have claimed parts of the stone.") : TEXT("The stone remains relatively clean.")
    );
}

bool AArch_PrehistoricArchitecture::IsPlayerInShelterRange(const FVector& PlayerLocation) const
{
    if (!InteractionData.bProvidesWeatherShelter)
    {
        return false;
    }

    float Distance = FVector::Dist(GetActorLocation(), PlayerLocation);
    return Distance <= InteractionData.ShelterRadius;
}

float AArch_PrehistoricArchitecture::GetWeatherProtectionAtLocation(const FVector& Location) const
{
    if (!IsPlayerInShelterRange(Location))
    {
        return 0.0f;
    }

    float Distance = FVector::Dist(GetActorLocation(), Location);
    float ProtectionFactor = 1.0f - (Distance / InteractionData.ShelterRadius);
    return InteractionData.WeatherProtection * ProtectionFactor;
}

void AArch_PrehistoricArchitecture::UpdateEnvironmentalEffects()
{
    // Update moss and lichen growth based on environmental conditions
    // This could be expanded to respond to weather, season, etc.
    
    if (StructureProperties.bHasMossGrowth)
    {
        // Moss grows more in humid conditions
        float HumidityFactor = 1.0f; // This would come from weather system
        StructureProperties.MossIntensity = FMath::Clamp(StructureProperties.MossIntensity + (HumidityFactor * 0.001f), 0.0f, 1.0f);
    }

    if (StructureProperties.bHasLichenGrowth)
    {
        // Lichen grows slowly over time
        StructureProperties.LichenIntensity = FMath::Clamp(StructureProperties.LichenIntensity + 0.0005f, 0.0f, 1.0f);
    }

    // Update visual appearance
    SpawnMossAndLichen();
}

void AArch_PrehistoricArchitecture::UpdateSeasonalAppearance(float SeasonProgress)
{
    // Adjust moss and lichen intensity based on season
    float SeasonalMossMultiplier = 0.5f + (0.5f * FMath::Sin(SeasonProgress * 2.0f * PI)); // More moss in wet seasons
    float SeasonalLichenMultiplier = 0.8f + (0.2f * FMath::Cos(SeasonProgress * 2.0f * PI)); // Lichen more stable

    for (UStaticMeshComponent* StoneMesh : StoneMeshComponents)
    {
        if (StoneMesh)
        {
            UMaterialInstanceDynamic* DynamicMaterial = Cast<UMaterialInstanceDynamic>(StoneMesh->GetMaterial(0));
            if (DynamicMaterial)
            {
                DynamicMaterial->SetScalarParameterValue(TEXT("MossIntensity"), 
                    StructureProperties.MossIntensity * SeasonalMossMultiplier);
                DynamicMaterial->SetScalarParameterValue(TEXT("LichenIntensity"), 
                    StructureProperties.LichenIntensity * SeasonalLichenMultiplier);
            }
        }
    }
}