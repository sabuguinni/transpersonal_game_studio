#include "Arch_SwampArchitectureSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/StaticMesh.h"

// AArch_SwampPlatformActor Implementation
AArch_SwampPlatformActor::AArch_SwampPlatformActor()
{
    PrimaryActorTick.bCanEverTick = false;

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Create platform mesh component
    PlatformMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlatformMesh"));
    PlatformMesh->SetupAttachment(RootComponent);
    
    // Create support pillars component
    SupportPillars = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SupportPillars"));
    SupportPillars->SetupAttachment(RootComponent);

    // Initialize default platform data
    PlatformData = FArch_SwampPlatformData();
}

void AArch_SwampPlatformActor::BeginPlay()
{
    Super::BeginPlay();
    
    // Apply initial platform configuration
    InitializePlatform(PlatformData);
}

void AArch_SwampPlatformActor::InitializePlatform(const FArch_SwampPlatformData& InPlatformData)
{
    PlatformData = InPlatformData;
    
    // Set platform location
    SetActorLocation(PlatformData.PlatformLocation);
    
    // Configure platform mesh scale based on data
    if (PlatformMesh)
    {
        FVector PlatformScale = FVector(1.0f, 1.0f, PlatformData.PlatformHeight / 100.0f);
        PlatformMesh->SetWorldScale3D(PlatformScale);
    }
    
    // Position support pillars based on water level
    if (SupportPillars)
    {
        FVector PillarOffset = FVector(0.0f, 0.0f, -(PlatformData.PlatformHeight - PlatformData.WaterLevel) * 0.5f);
        SupportPillars->SetRelativeLocation(PillarOffset);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Swamp platform initialized at location: %s"), *PlatformData.PlatformLocation.ToString());
}

void AArch_SwampPlatformActor::UpdateWaterLevel(float NewWaterLevel)
{
    PlatformData.WaterLevel = NewWaterLevel;
    
    // Adjust support pillar positioning
    if (SupportPillars)
    {
        FVector PillarOffset = FVector(0.0f, 0.0f, -(PlatformData.PlatformHeight - PlatformData.WaterLevel) * 0.5f);
        SupportPillars->SetRelativeLocation(PillarOffset);
    }
}

void AArch_SwampPlatformActor::ApplyWeathering(float WeatheringAmount)
{
    // Reduce structural integrity based on weathering
    PlatformData.bHasMossGrowth = WeatheringAmount > 0.3f;
    
    // Apply visual weathering effects to materials
    if (PlatformMesh && PlatformMesh->GetMaterial(0))
    {
        // Create dynamic material instance for weathering effects
        UMaterialInstanceDynamic* DynamicMaterial = PlatformMesh->CreateAndSetMaterialInstanceDynamic(0);
        if (DynamicMaterial)
        {
            DynamicMaterial->SetScalarParameterValue(TEXT("WeatheringAmount"), WeatheringAmount);
            DynamicMaterial->SetScalarParameterValue(TEXT("MossGrowth"), PlatformData.bHasMossGrowth ? 1.0f : 0.0f);
        }
    }
}

bool AArch_SwampPlatformActor::IsAboveWaterLevel() const
{
    float CurrentHeight = GetActorLocation().Z;
    return CurrentHeight > PlatformData.WaterLevel;
}

// AArch_SwampDwellingActor Implementation
AArch_SwampDwellingActor::AArch_SwampDwellingActor()
{
    PrimaryActorTick.bCanEverTick = false;

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Create dwelling structure component
    DwellingStructure = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DwellingStructure"));
    DwellingStructure->SetupAttachment(RootComponent);
    
    // Create interior furnishing component
    InteriorFurnishing = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("InteriorFurnishing"));
    InteriorFurnishing->SetupAttachment(RootComponent);

    // Initialize default dwelling data
    DwellingData = FArch_SwampDwellingData();
}

void AArch_SwampDwellingActor::BeginPlay()
{
    Super::BeginPlay();
    
    // Apply initial dwelling configuration
    InitializeDwelling(DwellingData);
}

void AArch_SwampDwellingActor::InitializeDwelling(const FArch_SwampDwellingData& InDwellingData)
{
    DwellingData = InDwellingData;
    
    // Set dwelling location
    SetActorLocation(DwellingData.DwellingLocation);
    
    // Configure dwelling structure scale based on floor area
    if (DwellingStructure)
    {
        float AreaScale = FMath::Sqrt(DwellingData.FloorArea / 16.0f); // Normalized to 4x4 base
        FVector StructureScale = FVector(AreaScale, AreaScale, 1.0f);
        DwellingStructure->SetWorldScale3D(StructureScale);
    }
    
    // Configure interior elements
    if (InteriorFurnishing)
    {
        // Scale interior based on dwelling size
        float InteriorScale = FMath::Sqrt(DwellingData.FloorArea / 16.0f);
        InteriorFurnishing->SetWorldScale3D(FVector(InteriorScale));
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Swamp dwelling initialized with %d sleeping areas and %.1f structural integrity"), 
           DwellingData.SleepingAreaCount, DwellingData.StructuralIntegrity);
}

void AArch_SwampDwellingActor::AddInteriorElement(const FString& ElementType)
{
    if (ElementType == TEXT("FirePit"))
    {
        DwellingData.bHasFirePit = true;
    }
    else if (ElementType == TEXT("StorageBaskets"))
    {
        DwellingData.bHasStorageBaskets = true;
    }
    else if (ElementType == TEXT("SleepingArea"))
    {
        DwellingData.SleepingAreaCount++;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Added interior element: %s to swamp dwelling"), *ElementType);
}

void AArch_SwampDwellingActor::UpdateStructuralIntegrity(float IntegrityChange)
{
    DwellingData.StructuralIntegrity = FMath::Clamp(DwellingData.StructuralIntegrity + IntegrityChange, 0.0f, 1.0f);
    
    // Apply visual effects based on structural integrity
    if (DwellingStructure && DwellingStructure->GetMaterial(0))
    {
        UMaterialInstanceDynamic* DynamicMaterial = DwellingStructure->CreateAndSetMaterialInstanceDynamic(0);
        if (DynamicMaterial)
        {
            DynamicMaterial->SetScalarParameterValue(TEXT("StructuralIntegrity"), DwellingData.StructuralIntegrity);
            DynamicMaterial->SetScalarParameterValue(TEXT("DamageLevel"), 1.0f - DwellingData.StructuralIntegrity);
        }
    }
}

bool AArch_SwampDwellingActor::CanSupportOccupants(int32 OccupantCount) const
{
    // Check if structural integrity and sleeping areas can support occupants
    bool bStructurallySound = DwellingData.StructuralIntegrity > 0.3f;
    bool bHasEnoughSpace = DwellingData.SleepingAreaCount >= OccupantCount;
    
    return bStructurallySound && bHasEnoughSpace;
}

// UArch_SwampArchitectureSubsystem Implementation
void UArch_SwampArchitectureSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    ActivePlatforms.Empty();
    ActiveDwellings.Empty();
    
    UE_LOG(LogTemp, Warning, TEXT("Swamp Architecture Subsystem initialized"));
}

void UArch_SwampArchitectureSubsystem::Deinitialize()
{
    // Clean up all tracked structures
    ActivePlatforms.Empty();
    ActiveDwellings.Empty();
    
    Super::Deinitialize();
}

AArch_SwampPlatformActor* UArch_SwampArchitectureSubsystem::CreateSwampPlatform(const FVector& Location, const FArch_SwampPlatformData& PlatformData)
{
    if (UWorld* World = GetWorld())
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
        
        AArch_SwampPlatformActor* NewPlatform = World->SpawnActor<AArch_SwampPlatformActor>(Location, FRotator::ZeroRotator, SpawnParams);
        
        if (NewPlatform)
        {
            NewPlatform->InitializePlatform(PlatformData);
            ActivePlatforms.Add(NewPlatform);
            
            UE_LOG(LogTemp, Warning, TEXT("Created swamp platform at location: %s"), *Location.ToString());
            return NewPlatform;
        }
    }
    
    return nullptr;
}

AArch_SwampDwellingActor* UArch_SwampArchitectureSubsystem::CreateSwampDwelling(const FVector& Location, const FArch_SwampDwellingData& DwellingData)
{
    if (UWorld* World = GetWorld())
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
        
        AArch_SwampDwellingActor* NewDwelling = World->SpawnActor<AArch_SwampDwellingActor>(Location, FRotator::ZeroRotator, SpawnParams);
        
        if (NewDwelling)
        {
            NewDwelling->InitializeDwelling(DwellingData);
            ActiveDwellings.Add(NewDwelling);
            
            UE_LOG(LogTemp, Warning, TEXT("Created swamp dwelling at location: %s"), *Location.ToString());
            return NewDwelling;
        }
    }
    
    return nullptr;
}

void UArch_SwampArchitectureSubsystem::UpdateAllStructuresForWaterLevel(float NewWaterLevel)
{
    // Update all platforms for new water level
    for (AArch_SwampPlatformActor* Platform : ActivePlatforms)
    {
        if (IsValid(Platform))
        {
            Platform->UpdateWaterLevel(NewWaterLevel);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Updated %d swamp platforms for water level: %.1f"), ActivePlatforms.Num(), NewWaterLevel);
}

void UArch_SwampArchitectureSubsystem::ApplyEnvironmentalWeathering(float DeltaTime)
{
    float WeatheringRate = 0.001f; // Slow weathering over time
    float WeatheringAmount = WeatheringRate * DeltaTime;
    
    // Apply weathering to all platforms
    for (AArch_SwampPlatformActor* Platform : ActivePlatforms)
    {
        if (IsValid(Platform))
        {
            Platform->ApplyWeathering(WeatheringAmount);
        }
    }
    
    // Apply structural degradation to dwellings
    for (AArch_SwampDwellingActor* Dwelling : ActiveDwellings)
    {
        if (IsValid(Dwelling))
        {
            Dwelling->UpdateStructuralIntegrity(-WeatheringAmount * 0.5f);
        }
    }
}

int32 UArch_SwampArchitectureSubsystem::GetActiveStructureCount() const
{
    return ActivePlatforms.Num() + ActiveDwellings.Num();
}

void UArch_SwampArchitectureSubsystem::CleanupDestroyedStructures()
{
    // Remove null or destroyed platforms
    ActivePlatforms.RemoveAll([](AArch_SwampPlatformActor* Platform)
    {
        return !IsValid(Platform);
    });
    
    // Remove null or destroyed dwellings
    ActiveDwellings.RemoveAll([](AArch_SwampDwellingActor* Dwelling)
    {
        return !IsValid(Dwelling);
    });
    
    UE_LOG(LogTemp, Log, TEXT("Cleaned up destroyed swamp structures. Active: %d platforms, %d dwellings"), 
           ActivePlatforms.Num(), ActiveDwellings.Num());
}