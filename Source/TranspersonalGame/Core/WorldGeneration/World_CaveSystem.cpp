#include "World_CaveSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Components/AudioComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Sound/SoundCue.h"
#include "Kismet/GameplayStatics.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/CollisionProfile.h"

AWorld_CaveSystem::AWorld_CaveSystem()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    
    // Create cave entrance mesh
    CaveEntranceMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CaveEntranceMesh"));
    CaveEntranceMesh->SetupAttachment(RootComponent);
    CaveEntranceMesh->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
    
    // Create cave interior mesh
    CaveInteriorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CaveInteriorMesh"));
    CaveInteriorMesh->SetupAttachment(RootComponent);
    CaveInteriorMesh->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
    
    // Create trigger volume for cave entrance
    CaveEntranceTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("CaveEntranceTrigger"));
    CaveEntranceTrigger->SetupAttachment(RootComponent);
    CaveEntranceTrigger->SetCollisionProfileName(UCollisionProfile::Trigger_ProfileName);
    CaveEntranceTrigger->SetBoxExtent(FVector(200.0f, 200.0f, 300.0f));
    
    // Create ambient audio component
    CaveAmbientAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("CaveAmbientAudio"));
    CaveAmbientAudio->SetupAttachment(RootComponent);
    CaveAmbientAudio->bAutoActivate = false;
    
    // Create dripping audio component
    CaveDrippingAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("CaveDrippingAudio"));
    CaveDrippingAudio->SetupAttachment(RootComponent);
    CaveDrippingAudio->bAutoActivate = false;
    
    // Create stalactite instances
    StalactiteInstances = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("StalactiteInstances"));
    StalactiteInstances->SetupAttachment(RootComponent);
    StalactiteInstances->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
    
    // Create stalagmite instances
    StalagmiteInstances = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("StalagmiteInstances"));
    StalagmiteInstances->SetupAttachment(RootComponent);
    StalagmiteInstances->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
    
    // Initialize default values
    CaveType = EWorld_CaveType::Natural;
    CaveSize = EWorld_CaveSize::Medium;
    CaveDepth = 500.0f;
    CaveWidth = 800.0f;
    CaveHeight = 400.0f;
    bHasWater = false;
    bHasStalactites = true;
    bHasStalagmites = true;
    bHasEcho = true;
    bHasBats = false;
    bHasTreasure = false;
    bIsExplored = false;
    bIsAccessible = true;
    
    WaterLevel = 0.0f;
    LightLevel = 0.1f;
    Temperature = 18.0f;
    Humidity = 0.8f;
    AirQuality = 0.9f;
    
    StalactiteCount = 15;
    StalagmiteCount = 12;
    
    // Set up trigger events
    CaveEntranceTrigger->OnComponentBeginOverlap.AddDynamic(this, &AWorld_CaveSystem::OnCaveEntranceBeginOverlap);
    CaveEntranceTrigger->OnComponentEndOverlap.AddDynamic(this, &AWorld_CaveSystem::OnCaveEntranceEndOverlap);
}

void AWorld_CaveSystem::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeCave();
    GenerateCaveGeometry();
    SetupCaveAudio();
    SpawnCaveFormations();
    
    if (bHasBats)
    {
        SpawnBats();
    }
    
    if (bHasTreasure)
    {
        SpawnTreasure();
    }
}

void AWorld_CaveSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    UpdateCaveAmbient(DeltaTime);
    UpdateWaterLevel(DeltaTime);
    
    if (bHasBats)
    {
        UpdateBatBehavior(DeltaTime);
    }
}

void AWorld_CaveSystem::InitializeCave()
{
    // Set cave properties based on type and size
    switch (CaveType)
    {
        case EWorld_CaveType::Natural:
            Temperature = FMath::RandRange(15.0f, 22.0f);
            Humidity = FMath::RandRange(0.7f, 0.9f);
            bHasStalactites = true;
            bHasStalagmites = true;
            break;
            
        case EWorld_CaveType::Lava:
            Temperature = FMath::RandRange(35.0f, 50.0f);
            Humidity = FMath::RandRange(0.3f, 0.6f);
            LightLevel = 0.3f; // Glowing lava
            bHasWater = false;
            break;
            
        case EWorld_CaveType::Ice:
            Temperature = FMath::RandRange(-5.0f, 5.0f);
            Humidity = FMath::RandRange(0.4f, 0.7f);
            LightLevel = 0.2f; // Ice reflections
            bHasStalactites = false;
            bHasStalagmites = false;
            break;
            
        case EWorld_CaveType::Crystal:
            LightLevel = 0.4f; // Crystal reflections
            bHasStalactites = false;
            bHasStalagmites = false;
            break;
    }
    
    // Adjust properties based on size
    switch (CaveSize)
    {
        case EWorld_CaveSize::Small:
            CaveDepth *= 0.6f;
            CaveWidth *= 0.6f;
            CaveHeight *= 0.6f;
            StalactiteCount = FMath::RandRange(5, 10);
            StalagmiteCount = FMath::RandRange(3, 8);
            break;
            
        case EWorld_CaveSize::Medium:
            StalactiteCount = FMath::RandRange(10, 20);
            StalagmiteCount = FMath::RandRange(8, 15);
            break;
            
        case EWorld_CaveSize::Large:
            CaveDepth *= 1.5f;
            CaveWidth *= 1.5f;
            CaveHeight *= 1.5f;
            StalactiteCount = FMath::RandRange(20, 35);
            StalagmiteCount = FMath::RandRange(15, 25);
            break;
            
        case EWorld_CaveSize::Massive:
            CaveDepth *= 2.5f;
            CaveWidth *= 2.5f;
            CaveHeight *= 2.0f;
            StalactiteCount = FMath::RandRange(30, 50);
            StalagmiteCount = FMath::RandRange(25, 40);
            break;
    }
    
    // Update trigger size
    CaveEntranceTrigger->SetBoxExtent(FVector(CaveWidth * 0.3f, CaveWidth * 0.3f, CaveHeight * 0.8f));
}

void AWorld_CaveSystem::GenerateCaveGeometry()
{
    // This would normally use procedural mesh generation
    // For now, we'll position the existing meshes appropriately
    
    if (CaveEntranceMesh)
    {
        CaveEntranceMesh->SetRelativeLocation(FVector::ZeroVector);
        CaveEntranceMesh->SetRelativeScale3D(FVector(CaveWidth / 400.0f, CaveDepth / 500.0f, CaveHeight / 300.0f));
    }
    
    if (CaveInteriorMesh)
    {
        CaveInteriorMesh->SetRelativeLocation(FVector(-CaveDepth * 0.5f, 0.0f, 0.0f));
        CaveInteriorMesh->SetRelativeScale3D(FVector(CaveDepth / 500.0f, CaveWidth / 400.0f, CaveHeight / 300.0f));
    }
}

void AWorld_CaveSystem::SetupCaveAudio()
{
    if (CaveAmbientAudio)
    {
        CaveAmbientAudio->SetVolumeMultiplier(0.6f);
        CaveAmbientAudio->SetPitchMultiplier(FMath::RandRange(0.8f, 1.2f));
        
        // Adjust audio based on cave type
        switch (CaveType)
        {
            case EWorld_CaveType::Lava:
                // Would load lava cave ambient sound
                break;
            case EWorld_CaveType::Ice:
                // Would load ice cave ambient sound
                break;
            case EWorld_CaveType::Crystal:
                // Would load crystal cave ambient sound
                break;
            default:
                // Natural cave ambient sound
                break;
        }
    }
    
    if (CaveDrippingAudio && bHasWater)
    {
        CaveDrippingAudio->SetVolumeMultiplier(0.3f);
        CaveDrippingAudio->SetPitchMultiplier(FMath::RandRange(0.9f, 1.1f));
    }
}

void AWorld_CaveSystem::SpawnCaveFormations()
{
    if (bHasStalactites && StalactiteInstances)
    {
        for (int32 i = 0; i < StalactiteCount; i++)
        {
            FVector Location = FVector(
                FMath::RandRange(-CaveDepth * 0.8f, -CaveDepth * 0.2f),
                FMath::RandRange(-CaveWidth * 0.4f, CaveWidth * 0.4f),
                CaveHeight * 0.8f
            );
            
            FRotator Rotation = FRotator(0.0f, FMath::RandRange(0.0f, 360.0f), 0.0f);
            FVector Scale = FVector(
                FMath::RandRange(0.5f, 1.5f),
                FMath::RandRange(0.5f, 1.5f),
                FMath::RandRange(0.8f, 2.0f)
            );
            
            FTransform Transform(Rotation, Location, Scale);
            StalactiteInstances->AddInstance(Transform);
        }
    }
    
    if (bHasStalagmites && StalagmiteInstances)
    {
        for (int32 i = 0; i < StalagmiteCount; i++)
        {
            FVector Location = FVector(
                FMath::RandRange(-CaveDepth * 0.8f, -CaveDepth * 0.2f),
                FMath::RandRange(-CaveWidth * 0.4f, CaveWidth * 0.4f),
                -CaveHeight * 0.4f
            );
            
            FRotator Rotation = FRotator(0.0f, FMath::RandRange(0.0f, 360.0f), 0.0f);
            FVector Scale = FVector(
                FMath::RandRange(0.6f, 1.4f),
                FMath::RandRange(0.6f, 1.4f),
                FMath::RandRange(0.5f, 1.8f)
            );
            
            FTransform Transform(Rotation, Location, Scale);
            StalagmiteInstances->AddInstance(Transform);
        }
    }
}

void AWorld_CaveSystem::SpawnBats()
{
    // This would spawn bat actors in the cave
    // For now, we'll just mark that bats are present
    UE_LOG(LogTemp, Log, TEXT("Cave System: Spawning %d bats in cave"), FMath::RandRange(5, 15));
}

void AWorld_CaveSystem::SpawnTreasure()
{
    // This would spawn treasure/resource actors in the cave
    UE_LOG(LogTemp, Log, TEXT("Cave System: Spawning treasure in cave"));
}

void AWorld_CaveSystem::UpdateCaveAmbient(float DeltaTime)
{
    // Update ambient lighting based on time of day and cave properties
    // This is a simplified version - would normally interact with lighting system
}

void AWorld_CaveSystem::UpdateWaterLevel(float DeltaTime)
{
    if (bHasWater)
    {
        // Simulate water level changes (seasonal, weather-based)
        float TargetWaterLevel = FMath::Sin(GetWorld()->GetTimeSeconds() * 0.1f) * 10.0f;
        WaterLevel = FMath::FInterpTo(WaterLevel, TargetWaterLevel, DeltaTime, 0.5f);
    }
}

void AWorld_CaveSystem::UpdateBatBehavior(float DeltaTime)
{
    // Update bat AI behavior - would normally control bat actors
    // This is a placeholder for bat behavior logic
}

void AWorld_CaveSystem::OnCaveEntranceBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (OtherActor && OtherActor->IsA<APawn>())
    {
        UE_LOG(LogTemp, Log, TEXT("Cave System: Player entered cave area"));
        
        // Start cave ambient audio
        if (CaveAmbientAudio && !CaveAmbientAudio->IsPlaying())
        {
            CaveAmbientAudio->Play();
        }
        
        if (CaveDrippingAudio && bHasWater && !CaveDrippingAudio->IsPlaying())
        {
            CaveDrippingAudio->Play();
        }
        
        // Mark cave as discovered if not already explored
        if (!bIsExplored)
        {
            bIsExplored = true;
            UE_LOG(LogTemp, Log, TEXT("Cave System: Cave discovered!"));
            
            // Could trigger discovery events here
            OnCaveDiscovered.Broadcast(this);
        }
        
        // Trigger cave entered event
        OnCaveEntered.Broadcast(this, Cast<APawn>(OtherActor));
    }
}

void AWorld_CaveSystem::OnCaveEntranceEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex)
{
    if (OtherActor && OtherActor->IsA<APawn>())
    {
        UE_LOG(LogTemp, Log, TEXT("Cave System: Player left cave area"));
        
        // Stop cave ambient audio with fade out
        if (CaveAmbientAudio && CaveAmbientAudio->IsPlaying())
        {
            CaveAmbientAudio->FadeOut(2.0f, 0.0f);
        }
        
        if (CaveDrippingAudio && CaveDrippingAudio->IsPlaying())
        {
            CaveDrippingAudio->FadeOut(1.5f, 0.0f);
        }
        
        // Trigger cave exited event
        OnCaveExited.Broadcast(this, Cast<APawn>(OtherActor));
    }
}

FString AWorld_CaveSystem::GetCaveDescription() const
{
    FString TypeStr;
    switch (CaveType)
    {
        case EWorld_CaveType::Natural: TypeStr = TEXT("Natural"); break;
        case EWorld_CaveType::Lava: TypeStr = TEXT("Lava"); break;
        case EWorld_CaveType::Ice: TypeStr = TEXT("Ice"); break;
        case EWorld_CaveType::Crystal: TypeStr = TEXT("Crystal"); break;
    }
    
    FString SizeStr;
    switch (CaveSize)
    {
        case EWorld_CaveSize::Small: SizeStr = TEXT("Small"); break;
        case EWorld_CaveSize::Medium: SizeStr = TEXT("Medium"); break;
        case EWorld_CaveSize::Large: SizeStr = TEXT("Large"); break;
        case EWorld_CaveSize::Massive: SizeStr = TEXT("Massive"); break;
    }
    
    return FString::Printf(TEXT("%s %s Cave - Depth: %.0fcm, Width: %.0fcm, Height: %.0fcm, Temp: %.1f°C"),
        *SizeStr, *TypeStr, CaveDepth, CaveWidth, CaveHeight, Temperature);
}

bool AWorld_CaveSystem::CanPlayerEnter(APawn* Player) const
{
    if (!bIsAccessible)
    {
        return false;
    }
    
    // Could add additional checks here (player equipment, cave danger level, etc.)
    return true;
}

void AWorld_CaveSystem::SetCaveAccessibility(bool bAccessible)
{
    bIsAccessible = bAccessible;
    
    if (CaveEntranceTrigger)
    {
        CaveEntranceTrigger->SetCollisionEnabled(bAccessible ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
    }
}

float AWorld_CaveSystem::GetCaveDangerLevel() const
{
    float DangerLevel = 0.0f;
    
    // Base danger from cave type
    switch (CaveType)
    {
        case EWorld_CaveType::Natural: DangerLevel += 0.2f; break;
        case EWorld_CaveType::Lava: DangerLevel += 0.8f; break;
        case EWorld_CaveType::Ice: DangerLevel += 0.4f; break;
        case EWorld_CaveType::Crystal: DangerLevel += 0.1f; break;
    }
    
    // Size factor
    switch (CaveSize)
    {
        case EWorld_CaveSize::Small: DangerLevel += 0.1f; break;
        case EWorld_CaveSize::Medium: DangerLevel += 0.2f; break;
        case EWorld_CaveSize::Large: DangerLevel += 0.3f; break;
        case EWorld_CaveSize::Massive: DangerLevel += 0.5f; break;
    }
    
    // Environmental factors
    if (bHasWater) DangerLevel += 0.1f;
    if (bHasBats) DangerLevel += 0.1f;
    if (Temperature > 40.0f) DangerLevel += 0.3f;
    if (Temperature < 0.0f) DangerLevel += 0.2f;
    if (AirQuality < 0.5f) DangerLevel += 0.4f;
    
    return FMath::Clamp(DangerLevel, 0.0f, 1.0f);
}