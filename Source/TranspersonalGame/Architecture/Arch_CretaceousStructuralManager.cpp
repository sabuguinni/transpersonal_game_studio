#include "Arch_CretaceousStructuralManager.h"
#include "Engine/Engine.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "GameFramework/Character.h"

AArch_CretaceousStructuralManager::AArch_CretaceousStructuralManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root scene component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Create primary structure mesh
    PrimaryStructureMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PrimaryStructureMesh"));
    PrimaryStructureMesh->SetupAttachment(RootComponent);

    // Create interior trigger volume
    InteriorTriggerVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("InteriorTriggerVolume"));
    InteriorTriggerVolume->SetupAttachment(RootComponent);
    InteriorTriggerVolume->SetBoxExtent(FVector(250, 250, 150));
    InteriorTriggerVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    InteriorTriggerVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
    InteriorTriggerVolume->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    // Initialize default values
    WeatheringIntensity = 0.6f;
    VegetationOvergrowth = 0.4f;
    bPlayerInInterior = false;
    StructuralIntegrity = 0.75f;
    LastWeatheringUpdate = 0.0f;
    ArchaeologicalNotes = TEXT("Cretaceous period stone construction showing primitive architectural techniques");

    // Initialize primary structure
    PrimaryStructure.ElementName = TEXT("Ancient Stone Archway");
    PrimaryStructure.StructureType = EArch_StructureType::Archway;
    PrimaryStructure.Dimensions = FVector(400, 100, 500);
    PrimaryStructure.DurabilityPercent = 75.0f;
    PrimaryStructure.bHasMossGrowth = true;
    PrimaryStructure.bIsPartiallyRuined = true;

    // Initialize interior space
    InteriorSpace.SpaceName = TEXT("Primitive Dwelling");
    InteriorSpace.InteriorType = EArch_InteriorType::Dwelling;
    InteriorSpace.SpaceBounds = FVector(400, 400, 250);
    InteriorSpace.AmbientLightLevel = 0.25f;
    InteriorSpace.bHasFirePit = true;
    InteriorSpace.bShowsHabitationSigns = true;
    InteriorSpace.ContainedArtifacts.Add(TEXT("Stone Tools"));
    InteriorSpace.ContainedArtifacts.Add(TEXT("Clay Pottery Fragments"));
    InteriorSpace.ContainedArtifacts.Add(TEXT("Animal Hide Remains"));
}

void AArch_CretaceousStructuralManager::BeginPlay()
{
    Super::BeginPlay();

    // Bind overlap events
    if (InteriorTriggerVolume)
    {
        InteriorTriggerVolume->OnComponentBeginOverlap.AddDynamic(this, &AArch_CretaceousStructuralManager::OnInteriorTriggerBeginOverlap);
        InteriorTriggerVolume->OnComponentEndOverlap.AddDynamic(this, &AArch_CretaceousStructuralManager::OnInteriorTriggerEndOverlap);
    }

    InitializeStructuralElements();
    PlaceInteriorArtifacts();
    UpdateStructuralMaterials();
}

void AArch_CretaceousStructuralManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    LastWeatheringUpdate += DeltaTime;
    
    // Apply gradual weathering every 30 seconds
    if (LastWeatheringUpdate >= 30.0f)
    {
        ApplyWeatheringEffects(0.001f); // Very gradual weathering
        LastWeatheringUpdate = 0.0f;
    }

    // Update vegetation growth
    if (VegetationOvergrowth < 1.0f)
    {
        UpdateVegetationGrowth(DeltaTime * 0.0001f); // Slow vegetation growth
    }
}

void AArch_CretaceousStructuralManager::InitializeStructuralElements()
{
    // Clear existing secondary elements
    SecondaryElements.Empty();

    // Add supporting structural elements
    FArch_StructuralElement SupportPillar;
    SupportPillar.ElementName = TEXT("Stone Support Pillar");
    SupportPillar.StructureType = EArch_StructureType::Pillar;
    SupportPillar.Dimensions = FVector(80, 80, 300);
    SupportPillar.DurabilityPercent = 80.0f;
    SupportPillar.bHasMossGrowth = true;
    SupportPillar.bIsPartiallyRuined = false;
    SecondaryElements.Add(SupportPillar);

    FArch_StructuralElement WallSection;
    WallSection.ElementName = TEXT("Weathered Stone Wall");
    WallSection.StructureType = EArch_StructureType::Wall;
    WallSection.Dimensions = FVector(300, 50, 200);
    WallSection.DurabilityPercent = 60.0f;
    WallSection.bHasMossGrowth = true;
    WallSection.bIsPartiallyRuined = true;
    SecondaryElements.Add(WallSection);

    CalculateStructuralIntegrity();

    UE_LOG(LogTemp, Log, TEXT("Structural Manager: Initialized %d structural elements"), SecondaryElements.Num() + 1);
}

void AArch_CretaceousStructuralManager::ApplyWeatheringEffects(float WeatheringAmount)
{
    // Apply weathering to primary structure
    PrimaryStructure.DurabilityPercent = FMath::Clamp(
        PrimaryStructure.DurabilityPercent - WeatheringAmount * 100.0f, 
        0.0f, 
        100.0f
    );

    // Apply weathering to secondary elements
    for (FArch_StructuralElement& Element : SecondaryElements)
    {
        Element.DurabilityPercent = FMath::Clamp(
            Element.DurabilityPercent - WeatheringAmount * 100.0f,
            0.0f,
            100.0f
        );
    }

    // Update weathering intensity
    WeatheringIntensity = FMath::Clamp(WeatheringIntensity + WeatheringAmount, 0.0f, 1.0f);

    // Recalculate structural integrity
    CalculateStructuralIntegrity();

    // Update visual materials
    UpdateStructuralMaterials();

    UE_LOG(LogTemp, Log, TEXT("Structural Manager: Applied weathering, integrity now %.2f"), StructuralIntegrity);
}

void AArch_CretaceousStructuralManager::SetupInteriorSpace(const FArch_InteriorSpace& NewInteriorConfig)
{
    InteriorSpace = NewInteriorConfig;

    // Update trigger volume size based on new space bounds
    if (InteriorTriggerVolume)
    {
        FVector NewExtent = InteriorSpace.SpaceBounds * 0.5f;
        InteriorTriggerVolume->SetBoxExtent(NewExtent);
    }

    // Place new artifacts
    PlaceInteriorArtifacts();

    UE_LOG(LogTemp, Log, TEXT("Structural Manager: Interior space configured - %s"), *InteriorSpace.SpaceName);
}

void AArch_CretaceousStructuralManager::UpdateVegetationGrowth(float GrowthRate)
{
    VegetationOvergrowth = FMath::Clamp(VegetationOvergrowth + GrowthRate, 0.0f, 1.0f);

    // Update moss growth on structures
    PrimaryStructure.bHasMossGrowth = VegetationOvergrowth > 0.2f;
    
    for (FArch_StructuralElement& Element : SecondaryElements)
    {
        Element.bHasMossGrowth = VegetationOvergrowth > 0.15f;
    }

    UpdateStructuralMaterials();
}

void AArch_CretaceousStructuralManager::OnPlayerEnterInterior()
{
    bPlayerInInterior = true;
    OnInteriorSpaceActivated();
    
    UE_LOG(LogTemp, Log, TEXT("Structural Manager: Player entered interior space - %s"), *InteriorSpace.SpaceName);
}

void AArch_CretaceousStructuralManager::OnPlayerExitInterior()
{
    bPlayerInInterior = false;
    
    UE_LOG(LogTemp, Log, TEXT("Structural Manager: Player exited interior space"));
}

void AArch_CretaceousStructuralManager::OnInteriorTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (ACharacter* Character = Cast<ACharacter>(OtherActor))
    {
        OnPlayerEnterInterior();
    }
}

void AArch_CretaceousStructuralManager::OnInteriorTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex)
{
    if (ACharacter* Character = Cast<ACharacter>(OtherActor))
    {
        OnPlayerExitInterior();
    }
}

void AArch_CretaceousStructuralManager::UpdateStructuralMaterials()
{
    // Update material parameters based on weathering and vegetation
    if (PrimaryStructureMesh && PrimaryStructureMesh->GetMaterial(0))
    {
        // Create dynamic material instance if needed
        UMaterialInstanceDynamic* DynamicMaterial = PrimaryStructureMesh->CreateAndSetMaterialInstanceDynamic(0);
        if (DynamicMaterial)
        {
            DynamicMaterial->SetScalarParameterValue(TEXT("WeatheringIntensity"), WeatheringIntensity);
            DynamicMaterial->SetScalarParameterValue(TEXT("MossGrowth"), VegetationOvergrowth);
            DynamicMaterial->SetScalarParameterValue(TEXT("StructuralIntegrity"), StructuralIntegrity);
        }
    }
}

void AArch_CretaceousStructuralManager::PlaceInteriorArtifacts()
{
    // Log artifact placement for debugging
    for (const FString& Artifact : InteriorSpace.ContainedArtifacts)
    {
        UE_LOG(LogTemp, Log, TEXT("Structural Manager: Artifact present - %s"), *Artifact);
    }

    // In a full implementation, this would spawn actual artifact actors
    // For now, we log the presence of artifacts for archaeological storytelling
}

void AArch_CretaceousStructuralManager::CalculateStructuralIntegrity()
{
    float TotalIntegrity = PrimaryStructure.DurabilityPercent;
    int32 ElementCount = 1;

    for (const FArch_StructuralElement& Element : SecondaryElements)
    {
        TotalIntegrity += Element.DurabilityPercent;
        ElementCount++;
    }

    StructuralIntegrity = TotalIntegrity / (ElementCount * 100.0f);
    StructuralIntegrity = FMath::Clamp(StructuralIntegrity, 0.0f, 1.0f);
}