#include "Arch_ShelterManager.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/PrimitiveComponent.h"

UArch_ShelterManager::UArch_ShelterManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f;

    // Initialize default shelter data
    ShelterData.ShelterType = EArch_ShelterType::CaveEntrance;
    ShelterData.ShelterSize = FVector(400.0f, 300.0f, 250.0f);
    ShelterData.ProtectionRadius = 500.0f;
    ShelterData.WeatherProtection = 0.8f;
    ShelterData.TemperatureBonus = 5.0f;
    ShelterData.bHasFirePit = false;
    ShelterData.bIsOccupied = false;
    ShelterData.MaxOccupants = 4;

    CurrentCondition = 1.0f;
    LastMaintenanceTime = 0.0f;

    // Initialize default interior elements
    FArch_InteriorElement FirePit;
    FirePit.ElementName = TEXT("FirePit");
    FirePit.RelativeLocation = FVector(0.0f, 0.0f, -50.0f);
    FirePit.RelativeRotation = FRotator::ZeroRotator;
    FirePit.Scale = FVector(1.0f, 1.0f, 0.5f);
    FirePit.bIsActive = false;
    InteriorElements.Add(FirePit);

    FArch_InteriorElement SleepingArea;
    SleepingArea.ElementName = TEXT("SleepingArea");
    SleepingArea.RelativeLocation = FVector(-150.0f, 100.0f, -30.0f);
    SleepingArea.RelativeRotation = FRotator::ZeroRotator;
    SleepingArea.Scale = FVector(2.0f, 1.5f, 0.3f);
    SleepingArea.bIsActive = true;
    InteriorElements.Add(SleepingArea);

    FArch_InteriorElement ToolStorage;
    ToolStorage.ElementName = TEXT("ToolStorage");
    ToolStorage.RelativeLocation = FVector(120.0f, -80.0f, 20.0f);
    ToolStorage.RelativeRotation = FRotator(0.0f, 45.0f, 0.0f);
    ToolStorage.Scale = FVector(0.8f, 0.8f, 1.2f);
    ToolStorage.bIsActive = true;
    InteriorElements.Add(ToolStorage);
}

void UArch_ShelterManager::BeginPlay()
{
    Super::BeginPlay();
    
    CreateMainStructure();
    CreateInteriorElements();
    ApplyMaterialsBasedOnType();
    SetupCollisionAndPhysics();
    
    UE_LOG(LogTemp, Warning, TEXT("Arch_ShelterManager: Shelter initialized with type %d"), (int32)ShelterData.ShelterType);
}

void UArch_ShelterManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    UpdateShelterCondition(DeltaTime);
}

void UArch_ShelterManager::InitializeShelter(EArch_ShelterType InShelterType, FVector InSize)
{
    ShelterData.ShelterType = InShelterType;
    ShelterData.ShelterSize = InSize;
    
    // Adjust protection values based on shelter type
    switch (InShelterType)
    {
        case EArch_ShelterType::CaveEntrance:
            ShelterData.WeatherProtection = 0.9f;
            ShelterData.TemperatureBonus = 8.0f;
            ShelterData.MaxOccupants = 6;
            break;
        case EArch_ShelterType::RockShelter:
            ShelterData.WeatherProtection = 0.7f;
            ShelterData.TemperatureBonus = 4.0f;
            ShelterData.MaxOccupants = 4;
            break;
        case EArch_ShelterType::TreeHollow:
            ShelterData.WeatherProtection = 0.6f;
            ShelterData.TemperatureBonus = 3.0f;
            ShelterData.MaxOccupants = 2;
            break;
        case EArch_ShelterType::StoneHut:
            ShelterData.WeatherProtection = 0.8f;
            ShelterData.TemperatureBonus = 6.0f;
            ShelterData.MaxOccupants = 5;
            break;
        case EArch_ShelterType::LeanTo:
            ShelterData.WeatherProtection = 0.5f;
            ShelterData.TemperatureBonus = 2.0f;
            ShelterData.MaxOccupants = 3;
            break;
        case EArch_ShelterType::Windbreak:
            ShelterData.WeatherProtection = 0.4f;
            ShelterData.TemperatureBonus = 1.0f;
            ShelterData.MaxOccupants = 2;
            break;
    }
    
    CreateMainStructure();
    ApplyMaterialsBasedOnType();
}

void UArch_ShelterManager::AddInteriorElement(const FArch_InteriorElement& NewElement)
{
    // Check if element with same name already exists
    for (int32 i = 0; i < InteriorElements.Num(); i++)
    {
        if (InteriorElements[i].ElementName == NewElement.ElementName)
        {
            InteriorElements[i] = NewElement;
            CreateInteriorElements();
            return;
        }
    }
    
    InteriorElements.Add(NewElement);
    CreateInteriorElements();
}

void UArch_ShelterManager::RemoveInteriorElement(const FString& ElementName)
{
    for (int32 i = InteriorElements.Num() - 1; i >= 0; i--)
    {
        if (InteriorElements[i].ElementName == ElementName)
        {
            InteriorElements.RemoveAt(i);
            break;
        }
    }
    CreateInteriorElements();
}

bool UArch_ShelterManager::CanProvideProtection(FVector TestLocation) const
{
    if (!GetOwner())
    {
        return false;
    }
    
    FVector ShelterLocation = GetOwner()->GetActorLocation();
    float Distance = FVector::Dist(TestLocation, ShelterLocation);
    
    return Distance <= ShelterData.ProtectionRadius;
}

float UArch_ShelterManager::GetProtectionLevel(FVector TestLocation) const
{
    if (!CanProvideProtection(TestLocation))
    {
        return 0.0f;
    }
    
    FVector ShelterLocation = GetOwner()->GetActorLocation();
    float Distance = FVector::Dist(TestLocation, ShelterLocation);
    float ProtectionFalloff = 1.0f - (Distance / ShelterData.ProtectionRadius);
    
    return ShelterData.WeatherProtection * ProtectionFalloff * CurrentCondition;
}

void UArch_ShelterManager::SetOccupancy(bool bOccupied, int32 NumOccupants)
{
    ShelterData.bIsOccupied = bOccupied;
    
    if (bOccupied && NumOccupants <= ShelterData.MaxOccupants)
    {
        // Enable fire pit if shelter is occupied
        ShelterData.bHasFirePit = true;
        ToggleFirePit(true);
    }
    else if (!bOccupied)
    {
        // Disable fire pit if shelter becomes unoccupied
        ToggleFirePit(false);
        CurrentOccupants.Empty();
    }
}

void UArch_ShelterManager::ToggleFirePit(bool bActive)
{
    ShelterData.bHasFirePit = bActive;
    
    // Update fire pit element
    for (FArch_InteriorElement& Element : InteriorElements)
    {
        if (Element.ElementName == TEXT("FirePit"))
        {
            Element.bIsActive = bActive;
            break;
        }
    }
    
    CreateInteriorElements();
}

void UArch_ShelterManager::GenerateBiomeSpecificShelter(EBiomeType BiomeType)
{
    // Set biome-specific shelter size and type
    ShelterData.ShelterSize = GetBiomeSpecificSize(BiomeType);
    
    // Clear existing interior elements and add biome-specific ones
    InteriorElements.Empty();
    InteriorElements = GetBiomeSpecificInterior(BiomeType);
    
    // Adjust shelter type based on biome
    switch (BiomeType)
    {
        case EBiomeType::TemperateForest:
            ShelterData.ShelterType = EArch_ShelterType::TreeHollow;
            break;
        case EBiomeType::Mountain:
            ShelterData.ShelterType = EArch_ShelterType::CaveEntrance;
            break;
        case EBiomeType::Swampland:
            ShelterData.ShelterType = EArch_ShelterType::LeanTo;
            break;
        case EBiomeType::Desert:
            ShelterData.ShelterType = EArch_ShelterType::RockShelter;
            break;
        default:
            ShelterData.ShelterType = EArch_ShelterType::Windbreak;
            break;
    }
    
    InitializeShelter(ShelterData.ShelterType, ShelterData.ShelterSize);
}

void UArch_ShelterManager::ApplyWeatheringEffects(float WeatheringLevel)
{
    CurrentCondition = FMath::Clamp(1.0f - WeatheringLevel, 0.1f, 1.0f);
    
    // Reduce protection values based on weathering
    float ConditionMultiplier = CurrentCondition;
    ShelterData.WeatherProtection *= ConditionMultiplier;
    ShelterData.TemperatureBonus *= ConditionMultiplier;
}

void UArch_ShelterManager::UpdateShelterCondition(float DeltaTime)
{
    // Gradual deterioration over time
    float DeteriorationRate = 0.00001f; // Very slow deterioration
    CurrentCondition = FMath::Clamp(CurrentCondition - (DeteriorationRate * DeltaTime), 0.1f, 1.0f);
    
    // Faster deterioration if exposed to weather and unoccupied
    if (!ShelterData.bIsOccupied)
    {
        CurrentCondition = FMath::Clamp(CurrentCondition - (DeteriorationRate * 2.0f * DeltaTime), 0.1f, 1.0f);
    }
}

void UArch_ShelterManager::CreateMainStructure()
{
    if (!GetOwner())
    {
        return;
    }
    
    // Find or create main structure mesh component
    MainStructureMesh = GetOwner()->FindComponentByClass<UStaticMeshComponent>();
    if (!MainStructureMesh)
    {
        MainStructureMesh = NewObject<UStaticMeshComponent>(GetOwner(), TEXT("MainStructureMesh"));
        GetOwner()->AddInstanceComponent(MainStructureMesh);
        MainStructureMesh->AttachToComponent(GetOwner()->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
    }
    
    // Set mesh scale based on shelter size
    FVector Scale = ShelterData.ShelterSize / 100.0f; // Normalize to reasonable scale
    MainStructureMesh->SetRelativeScale3D(Scale);
}

void UArch_ShelterManager::CreateInteriorElements()
{
    if (!GetOwner())
    {
        return;
    }
    
    // Clear existing interior meshes
    for (UStaticMeshComponent* Mesh : InteriorMeshes)
    {
        if (Mesh)
        {
            Mesh->DestroyComponent();
        }
    }
    InteriorMeshes.Empty();
    
    // Create new interior element meshes
    for (const FArch_InteriorElement& Element : InteriorElements)
    {
        if (Element.bIsActive)
        {
            UStaticMeshComponent* ElementMesh = NewObject<UStaticMeshComponent>(GetOwner(), FName(*Element.ElementName));
            if (ElementMesh)
            {
                GetOwner()->AddInstanceComponent(ElementMesh);
                ElementMesh->AttachToComponent(GetOwner()->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
                ElementMesh->SetRelativeLocation(Element.RelativeLocation);
                ElementMesh->SetRelativeRotation(Element.RelativeRotation);
                ElementMesh->SetRelativeScale3D(Element.Scale);
                InteriorMeshes.Add(ElementMesh);
            }
        }
    }
}

void UArch_ShelterManager::ApplyMaterialsBasedOnType()
{
    if (!MainStructureMesh)
    {
        return;
    }
    
    // Apply materials based on shelter type
    switch (ShelterData.ShelterType)
    {
        case EArch_ShelterType::CaveEntrance:
        case EArch_ShelterType::RockShelter:
        case EArch_ShelterType::StoneHut:
            if (StoneMaterial)
            {
                MainStructureMesh->SetMaterial(0, StoneMaterial);
            }
            break;
        case EArch_ShelterType::TreeHollow:
        case EArch_ShelterType::LeanTo:
        case EArch_ShelterType::Windbreak:
            if (WoodMaterial)
            {
                MainStructureMesh->SetMaterial(0, WoodMaterial);
            }
            break;
    }
    
    // Apply moss material to some interior elements for realism
    for (UStaticMeshComponent* Mesh : InteriorMeshes)
    {
        if (Mesh && MossMaterial)
        {
            Mesh->SetMaterial(0, MossMaterial);
        }
    }
}

void UArch_ShelterManager::SetupCollisionAndPhysics()
{
    if (!MainStructureMesh)
    {
        return;
    }
    
    // Set up collision for shelter
    MainStructureMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    MainStructureMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
    MainStructureMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
}

FVector UArch_ShelterManager::GetBiomeSpecificSize(EBiomeType BiomeType) const
{
    switch (BiomeType)
    {
        case EBiomeType::TemperateForest:
            return FVector(300.0f, 300.0f, 400.0f); // Tall tree hollow
        case EBiomeType::Mountain:
            return FVector(500.0f, 400.0f, 300.0f); // Wide cave entrance
        case EBiomeType::Swampland:
            return FVector(400.0f, 200.0f, 200.0f); // Low lean-to
        case EBiomeType::Desert:
            return FVector(350.0f, 250.0f, 180.0f); // Rock overhang
        default:
            return FVector(400.0f, 300.0f, 250.0f); // Default size
    }
}

TArray<FArch_InteriorElement> UArch_ShelterManager::GetBiomeSpecificInterior(EBiomeType BiomeType) const
{
    TArray<FArch_InteriorElement> Elements;
    
    // Common fire pit
    FArch_InteriorElement FirePit;
    FirePit.ElementName = TEXT("FirePit");
    FirePit.RelativeLocation = FVector(0.0f, 0.0f, -50.0f);
    FirePit.bIsActive = false;
    Elements.Add(FirePit);
    
    switch (BiomeType)
    {
        case EBiomeType::TemperateForest:
        {
            FArch_InteriorElement LeafBed;
            LeafBed.ElementName = TEXT("LeafBed");
            LeafBed.RelativeLocation = FVector(-100.0f, 80.0f, -40.0f);
            LeafBed.Scale = FVector(1.5f, 1.0f, 0.2f);
            Elements.Add(LeafBed);
            
            FArch_InteriorElement BarkStorage;
            BarkStorage.ElementName = TEXT("BarkStorage");
            BarkStorage.RelativeLocation = FVector(90.0f, -60.0f, 10.0f);
            Elements.Add(BarkStorage);
            break;
        }
        case EBiomeType::Mountain:
        {
            FArch_InteriorElement StoneSeat;
            StoneSeat.ElementName = TEXT("StoneSeat");
            StoneSeat.RelativeLocation = FVector(-150.0f, 0.0f, -30.0f);
            StoneSeat.Scale = FVector(1.0f, 0.5f, 0.5f);
            Elements.Add(StoneSeat);
            
            FArch_InteriorElement ToolRack;
            ToolRack.ElementName = TEXT("ToolRack");
            ToolRack.RelativeLocation = FVector(120.0f, -100.0f, 50.0f);
            Elements.Add(ToolRack);
            break;
        }
        case EBiomeType::Swampland:
        {
            FArch_InteriorElement RaisedPlatform;
            RaisedPlatform.ElementName = TEXT("RaisedPlatform");
            RaisedPlatform.RelativeLocation = FVector(-80.0f, 60.0f, 20.0f);
            RaisedPlatform.Scale = FVector(2.0f, 1.5f, 0.3f);
            Elements.Add(RaisedPlatform);
            break;
        }
        case EBiomeType::Desert:
        {
            FArch_InteriorElement SandBed;
            SandBed.ElementName = TEXT("SandBed");
            SandBed.RelativeLocation = FVector(-120.0f, 70.0f, -45.0f);
            SandBed.Scale = FVector(1.8f, 1.2f, 0.1f);
            Elements.Add(SandBed);
            
            FArch_InteriorElement WaterStorage;
            WaterStorage.ElementName = TEXT("WaterStorage");
            WaterStorage.RelativeLocation = FVector(100.0f, -50.0f, -20.0f);
            Elements.Add(WaterStorage);
            break;
        }
    }
    
    return Elements;
}