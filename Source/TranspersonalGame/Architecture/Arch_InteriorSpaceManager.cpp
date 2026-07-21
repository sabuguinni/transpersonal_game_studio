#include "Arch_InteriorSpaceManager.h"
#include "Engine/Engine.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

AArch_InteriorSpaceManager::AArch_InteriorSpaceManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 2.0f; // Update every 2 seconds

    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Create interior bounds
    InteriorBounds = CreateDefaultSubobject<UBoxComponent>(TEXT("InteriorBounds"));
    InteriorBounds->SetupAttachment(RootComponent);
    InteriorBounds->SetBoxExtent(FVector(300.0f, 300.0f, 200.0f));
    InteriorBounds->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    InteriorBounds->SetCollisionResponseToAllChannels(ECR_Ignore);
    InteriorBounds->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    // Create floor mesh
    FloorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FloorMesh"));
    FloorMesh->SetupAttachment(RootComponent);
    FloorMesh->SetRelativeLocation(FVector(0, 0, -100));
    
    // Initialize default values
    InteriorType = EArch_InteriorType::Dwelling;
    InteriorMood = EArch_InteriorMood::Cozy;
    InteriorTemperature = 20.0f;
    AirQuality = 100.0f;
    bIsOccupied = false;
    MaxOccupants = 4;
    EnvironmentUpdateTimer = 0.0f;
}

void AArch_InteriorSpaceManager::BeginPlay()
{
    Super::BeginPlay();
    
    CreateInteriorLayout();
    SpawnInteriorProps();
    UpdateEnvironmentalEffects();

    // Bind overlap events
    if (InteriorBounds)
    {
        InteriorBounds->OnComponentBeginOverlap.AddDynamic(this, &AArch_InteriorSpaceManager::OnActorEnterInterior);
        InteriorBounds->OnComponentEndOverlap.AddDynamic(this, &AArch_InteriorSpaceManager::OnActorExitInterior);
    }
}

void AArch_InteriorSpaceManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    EnvironmentUpdateTimer += DeltaTime;
    
    if (EnvironmentUpdateTimer >= 2.0f)
    {
        UpdateEnvironmentalEffects();
        EnvironmentUpdateTimer = 0.0f;
    }
}

void AArch_InteriorSpaceManager::SetupInterior(EArch_InteriorType Type, EArch_InteriorMood Mood)
{
    InteriorType = Type;
    InteriorMood = Mood;
    
    CreateInteriorLayout();
    SpawnInteriorProps();
    UpdateEnvironmentalEffects();
}

void AArch_InteriorSpaceManager::SpawnInteriorProps()
{
    // Clear existing props
    for (AActor* Prop : SpawnedProps)
    {
        if (IsValid(Prop))
        {
            Prop->Destroy();
        }
    }
    SpawnedProps.Empty();

    // Spawn props based on interior type
    switch (InteriorType)
    {
        case EArch_InteriorType::Dwelling:
            InteriorProps.bHasFirePit = true;
            InteriorProps.bHasSleepingArea = true;
            InteriorProps.bHasStorage = true;
            break;
        case EArch_InteriorType::Workshop:
            InteriorProps.bHasFirePit = true;
            InteriorProps.bHasStorage = true;
            break;
        case EArch_InteriorType::Storage:
            InteriorProps.bHasStorage = true;
            break;
        case EArch_InteriorType::Shelter:
            InteriorProps.bHasSleepingArea = true;
            break;
        case EArch_InteriorType::Sacred:
            InteriorProps.bHasFirePit = true;
            break;
        case EArch_InteriorType::Communal:
            InteriorProps.bHasFirePit = true;
            InteriorProps.bHasSleepingArea = true;
            break;
    }

    // Spawn fire pits
    if (InteriorProps.bHasFirePit)
    {
        for (const FVector& Location : InteriorProps.FirePitLocations)
        {
            SpawnFirePit(Location);
        }
    }

    // Spawn sleeping areas
    if (InteriorProps.bHasSleepingArea)
    {
        for (const FVector& Location : InteriorProps.SleepingAreaLocations)
        {
            SpawnSleepingArea(Location);
        }
    }

    // Spawn storage
    if (InteriorProps.bHasStorage)
    {
        for (const FVector& Location : InteriorProps.StorageLocations)
        {
            SpawnStorage(Location);
        }
    }

    // Spawn tools
    for (const FVector& Location : InteriorProps.ToolLocations)
    {
        SpawnTools(Location);
    }
}

bool AArch_InteriorSpaceManager::CanEnterInterior(AActor* Actor)
{
    if (!Actor)
        return false;

    return CurrentOccupants.Num() < MaxOccupants;
}

void AArch_InteriorSpaceManager::OnActorEnterInterior(AActor* Actor)
{
    if (!Actor || CurrentOccupants.Contains(Actor))
        return;

    if (CanEnterInterior(Actor))
    {
        CurrentOccupants.Add(Actor);
        bIsOccupied = CurrentOccupants.Num() > 0;
        
        UE_LOG(LogTemp, Log, TEXT("Actor %s entered interior. Occupants: %d"), 
               *Actor->GetName(), CurrentOccupants.Num());
        
        OnInteriorEntered(Actor);
    }
}

void AArch_InteriorSpaceManager::OnActorExitInterior(AActor* Actor)
{
    if (!Actor)
        return;

    CurrentOccupants.Remove(Actor);
    bIsOccupied = CurrentOccupants.Num() > 0;
    
    UE_LOG(LogTemp, Log, TEXT("Actor %s exited interior. Occupants: %d"), 
           *Actor->GetName(), CurrentOccupants.Num());
    
    OnInteriorExited(Actor);
}

FVector AArch_InteriorSpaceManager::GetRandomInteriorLocation() const
{
    if (!InteriorBounds)
        return GetActorLocation();

    FVector BoundsExtent = InteriorBounds->GetScaledBoxExtent();
    FVector RandomOffset = FVector(
        FMath::RandRange(-BoundsExtent.X * 0.8f, BoundsExtent.X * 0.8f),
        FMath::RandRange(-BoundsExtent.Y * 0.8f, BoundsExtent.Y * 0.8f),
        50.0f
    );

    return GetActorLocation() + RandomOffset;
}

TArray<FVector> AArch_InteriorSpaceManager::GetPropLocations(const FString& PropType) const
{
    if (PropType == "FirePit")
        return InteriorProps.FirePitLocations;
    else if (PropType == "Sleeping")
        return InteriorProps.SleepingAreaLocations;
    else if (PropType == "Storage")
        return InteriorProps.StorageLocations;
    else if (PropType == "Tools")
        return InteriorProps.ToolLocations;
    
    return TArray<FVector>();
}

float AArch_InteriorSpaceManager::GetComfortLevel() const
{
    float ComfortLevel = 50.0f; // Base comfort

    // Temperature comfort
    float TempComfort = 100.0f - FMath::Abs(InteriorTemperature - 22.0f) * 2.0f;
    ComfortLevel += TempComfort * 0.3f;

    // Air quality comfort
    ComfortLevel += AirQuality * 0.2f;

    // Props comfort bonus
    if (InteriorProps.bHasFirePit) ComfortLevel += 15.0f;
    if (InteriorProps.bHasSleepingArea) ComfortLevel += 10.0f;
    if (InteriorProps.bHasStorage) ComfortLevel += 5.0f;

    // Mood modifier
    switch (InteriorMood)
    {
        case EArch_InteriorMood::Cozy:
            ComfortLevel += 20.0f;
            break;
        case EArch_InteriorMood::Peaceful:
            ComfortLevel += 15.0f;
            break;
        case EArch_InteriorMood::Active:
            ComfortLevel += 5.0f;
            break;
        case EArch_InteriorMood::Abandoned:
            ComfortLevel -= 30.0f;
            break;
        case EArch_InteriorMood::Dangerous:
            ComfortLevel -= 40.0f;
            break;
    }

    return FMath::Clamp(ComfortLevel, 0.0f, 100.0f);
}

void AArch_InteriorSpaceManager::UpdateEnvironmentalEffects()
{
    UpdateTemperature();
    UpdateLighting();

    // Update air quality based on occupancy and props
    float BaseAirQuality = 80.0f;
    
    if (InteriorProps.bHasFirePit)
    {
        BaseAirQuality -= 10.0f; // Smoke reduces air quality
    }
    
    // Overcrowding reduces air quality
    if (CurrentOccupants.Num() > MaxOccupants / 2)
    {
        BaseAirQuality -= (CurrentOccupants.Num() - MaxOccupants / 2) * 5.0f;
    }
    
    AirQuality = FMath::Clamp(BaseAirQuality, 0.0f, 100.0f);
}

void AArch_InteriorSpaceManager::CreateInteriorLayout()
{
    // Clear existing prop locations
    InteriorProps.FirePitLocations.Empty();
    InteriorProps.SleepingAreaLocations.Empty();
    InteriorProps.StorageLocations.Empty();
    InteriorProps.ToolLocations.Empty();

    FVector Center = GetActorLocation();

    // Create layout based on interior type
    switch (InteriorType)
    {
        case EArch_InteriorType::Dwelling:
            InteriorProps.FirePitLocations.Add(Center + FVector(0, 0, 0));
            InteriorProps.SleepingAreaLocations.Add(Center + FVector(150, 150, 0));
            InteriorProps.SleepingAreaLocations.Add(Center + FVector(-150, 150, 0));
            InteriorProps.StorageLocations.Add(Center + FVector(0, -200, 0));
            InteriorProps.ToolLocations.Add(Center + FVector(200, 0, 0));
            break;
        case EArch_InteriorType::Workshop:
            InteriorProps.FirePitLocations.Add(Center + FVector(-100, 0, 0));
            InteriorProps.StorageLocations.Add(Center + FVector(150, 150, 0));
            InteriorProps.StorageLocations.Add(Center + FVector(150, -150, 0));
            InteriorProps.ToolLocations.Add(Center + FVector(0, 100, 0));
            InteriorProps.ToolLocations.Add(Center + FVector(0, -100, 0));
            break;
        case EArch_InteriorType::Storage:
            for (int32 i = 0; i < 6; i++)
            {
                float Angle = i * 60.0f * PI / 180.0f;
                FVector StoragePos = Center + FVector(FMath::Cos(Angle) * 150.0f, FMath::Sin(Angle) * 150.0f, 0);
                InteriorProps.StorageLocations.Add(StoragePos);
            }
            break;
        default:
            InteriorProps.FirePitLocations.Add(Center);
            break;
    }
}

void AArch_InteriorSpaceManager::SpawnFirePit(const FVector& Location)
{
    // Spawn basic fire pit placeholder
    UWorld* World = GetWorld();
    if (World)
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.Owner = this;
        
        // Use basic static mesh actor as placeholder
        AStaticMeshActor* FirePit = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), Location, FRotator::ZeroRotator, SpawnParams);
        if (FirePit)
        {
            FirePit->SetActorLabel(TEXT("FirePit"));
            SpawnedProps.Add(FirePit);
        }
    }
}

void AArch_InteriorSpaceManager::SpawnSleepingArea(const FVector& Location)
{
    // Spawn sleeping area placeholder
    UWorld* World = GetWorld();
    if (World)
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.Owner = this;
        
        AStaticMeshActor* SleepingArea = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), Location, FRotator::ZeroRotator, SpawnParams);
        if (SleepingArea)
        {
            SleepingArea->SetActorLabel(TEXT("SleepingArea"));
            SpawnedProps.Add(SleepingArea);
        }
    }
}

void AArch_InteriorSpaceManager::SpawnStorage(const FVector& Location)
{
    // Spawn storage placeholder
    UWorld* World = GetWorld();
    if (World)
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.Owner = this;
        
        AStaticMeshActor* Storage = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), Location, FRotator::ZeroRotator, SpawnParams);
        if (Storage)
        {
            Storage->SetActorLabel(TEXT("Storage"));
            SpawnedProps.Add(Storage);
        }
    }
}

void AArch_InteriorSpaceManager::SpawnTools(const FVector& Location)
{
    // Spawn tools placeholder
    UWorld* World = GetWorld();
    if (World)
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.Owner = this;
        
        AStaticMeshActor* Tools = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), Location, FRotator::ZeroRotator, SpawnParams);
        if (Tools)
        {
            Tools->SetActorLabel(TEXT("Tools"));
            SpawnedProps.Add(Tools);
        }
    }
}

void AArch_InteriorSpaceManager::UpdateLighting()
{
    // Update lighting intensity based on props and mood
    float BaseLighting = 0.5f;
    
    if (InteriorProps.bHasFirePit)
    {
        BaseLighting += 0.4f;
    }
    
    switch (InteriorMood)
    {
        case EArch_InteriorMood::Cozy:
            BaseLighting += 0.2f;
            break;
        case EArch_InteriorMood::Mysterious:
            BaseLighting -= 0.3f;
            break;
        case EArch_InteriorMood::Dangerous:
            BaseLighting -= 0.2f;
            break;
    }
    
    InteriorProps.LightingIntensity = FMath::Clamp(BaseLighting, 0.1f, 2.0f);
}

void AArch_InteriorSpaceManager::UpdateTemperature()
{
    // Base temperature influenced by outside conditions and props
    float BaseTemp = 18.0f; // Slightly cool base temperature
    
    if (InteriorProps.bHasFirePit)
    {
        BaseTemp += 8.0f; // Fire provides warmth
    }
    
    // Occupancy adds warmth
    BaseTemp += CurrentOccupants.Num() * 1.5f;
    
    InteriorTemperature = FMath::Clamp(BaseTemp, 5.0f, 35.0f);
}