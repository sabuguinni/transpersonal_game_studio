#include "Arch_ShelterManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/Character.h"

AArch_ShelterManager::AArch_ShelterManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Create shelter mesh component
    ShelterMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ShelterMesh"));
    ShelterMesh->SetupAttachment(RootComponent);
    ShelterMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    ShelterMesh->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);

    // Create protection zone
    ProtectionZone = CreateDefaultSubobject<UBoxComponent>(TEXT("ProtectionZone"));
    ProtectionZone->SetupAttachment(RootComponent);
    ProtectionZone->SetBoxExtent(FVector(500.0f, 500.0f, 300.0f));
    ProtectionZone->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    ProtectionZone->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
    ProtectionZone->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);

    // Create fire pit mesh
    FirePitMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FirePitMesh"));
    FirePitMesh->SetupAttachment(RootComponent);
    FirePitMesh->SetRelativeLocation(FVector(0.0f, 0.0f, -50.0f));
    FirePitMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

    // Create storage area mesh
    StorageAreaMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StorageAreaMesh"));
    StorageAreaMesh->SetupAttachment(RootComponent);
    StorageAreaMesh->SetRelativeLocation(FVector(-200.0f, 200.0f, 0.0f));
    StorageAreaMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

    // Initialize default values
    bIsOccupied = false;
    CurrentTemperature = 20.0f;
    bFireLit = false;
    FireIntensity = 0.0f;
    ShelterAge = 0.0f;
    LastMaintenanceTime = 0.0f;
    bNeedsRepair = false;

    // Bind overlap events
    ProtectionZone->OnComponentBeginOverlap.AddDynamic(this, &AArch_ShelterManager::OnProtectionZoneBeginOverlap);
    ProtectionZone->OnComponentEndOverlap.AddDynamic(this, &AArch_ShelterManager::OnProtectionZoneEndOverlap);
}

void AArch_ShelterManager::BeginPlay()
{
    Super::BeginPlay();

    // Initialize shelter with default configuration
    InitializeShelter(ShelterConfig);

    // Start update timers
    GetWorld()->GetTimerManager().SetTimer(
        ConditionUpdateTimer,
        [this]() { UpdateShelterConditions(1.0f); },
        1.0f,
        true
    );

    GetWorld()->GetTimerManager().SetTimer(
        FireUpdateTimer,
        [this]() { UpdateFireEffects(0.1f); },
        0.1f,
        true
    );
}

void AArch_ShelterManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    ShelterAge += DeltaTime;
    UpdateOccupancyState();
}

void AArch_ShelterManager::InitializeShelter(const FArch_ShelterConfig& Config)
{
    ShelterConfig = Config;

    // Update protection zone size
    ProtectionZone->SetBoxExtent(FVector(Config.ProtectionRadius, Config.ProtectionRadius, 300.0f));

    // Setup shelter mesh based on type
    SetupShelterMesh();

    // Setup interior elements
    SetupInteriorElements();

    UE_LOG(LogTemp, Log, TEXT("Shelter initialized: Type=%d, Protection=%.2f"), 
           (int32)Config.ShelterType, Config.WeatherProtection);
}

void AArch_ShelterManager::SetupShelterMesh()
{
    if (ShelterMeshVariants.Num() > 0)
    {
        int32 MeshIndex = FMath::Clamp((int32)ShelterConfig.ShelterType, 0, ShelterMeshVariants.Num() - 1);
        if (ShelterMeshVariants[MeshIndex])
        {
            ShelterMesh->SetStaticMesh(ShelterMeshVariants[MeshIndex]);
        }
    }

    if (ShelterMaterials.Num() > 0)
    {
        int32 MaterialIndex = FMath::RandRange(0, ShelterMaterials.Num() - 1);
        if (ShelterMaterials[MaterialIndex])
        {
            ShelterMesh->SetMaterial(0, ShelterMaterials[MaterialIndex]);
        }
    }
}

void AArch_ShelterManager::SetupInteriorElements()
{
    // Show/hide fire pit based on configuration
    FirePitMesh->SetVisibility(ShelterConfig.bHasFirePit);
    FirePitMesh->SetCollisionEnabled(ShelterConfig.bHasFirePit ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);

    // Show/hide storage area based on configuration
    StorageAreaMesh->SetVisibility(ShelterConfig.bHasStorageArea);
    StorageAreaMesh->SetCollisionEnabled(ShelterConfig.bHasStorageArea ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
}

bool AArch_ShelterManager::CanEnterShelter(AActor* Actor)
{
    if (!Actor || !IsValid(Actor))
    {
        return false;
    }

    if (CurrentOccupants.Num() >= ShelterConfig.MaxOccupants)
    {
        return false;
    }

    if (CurrentOccupants.Contains(Actor))
    {
        return false;
    }

    return IsLocationProtected(Actor->GetActorLocation());
}

void AArch_ShelterManager::EnterShelter(AActor* Actor)
{
    if (CanEnterShelter(Actor))
    {
        CurrentOccupants.AddUnique(Actor);
        UpdateOccupancyState();
        
        UE_LOG(LogTemp, Log, TEXT("Actor %s entered shelter. Occupants: %d/%d"), 
               *Actor->GetName(), CurrentOccupants.Num(), ShelterConfig.MaxOccupants);
    }
}

void AArch_ShelterManager::ExitShelter(AActor* Actor)
{
    if (CurrentOccupants.Contains(Actor))
    {
        CurrentOccupants.Remove(Actor);
        UpdateOccupancyState();
        
        UE_LOG(LogTemp, Log, TEXT("Actor %s exited shelter. Occupants: %d/%d"), 
               *Actor->GetName(), CurrentOccupants.Num(), ShelterConfig.MaxOccupants);
    }
}

void AArch_ShelterManager::LightFire()
{
    if (ShelterConfig.bHasFirePit && !bFireLit)
    {
        bFireLit = true;
        FireIntensity = 1.0f;
        
        UE_LOG(LogTemp, Log, TEXT("Fire lit in shelter"));
    }
}

void AArch_ShelterManager::ExtinguishFire()
{
    if (bFireLit)
    {
        bFireLit = false;
        FireIntensity = 0.0f;
        
        UE_LOG(LogTemp, Log, TEXT("Fire extinguished in shelter"));
    }
}

float AArch_ShelterManager::GetWeatherProtectionAt(const FVector& Location)
{
    if (!IsLocationProtected(Location))
    {
        return 0.0f;
    }

    float Distance = FVector::Dist(GetActorLocation(), Location);
    float MaxDistance = ShelterConfig.ProtectionRadius;
    float ProtectionFactor = FMath::Clamp(1.0f - (Distance / MaxDistance), 0.0f, 1.0f);

    return ShelterConfig.WeatherProtection * ProtectionFactor;
}

float AArch_ShelterManager::GetTemperatureModifierAt(const FVector& Location)
{
    if (!IsLocationProtected(Location))
    {
        return 0.0f;
    }

    float BaseModifier = ShelterConfig.TemperatureModifier;
    
    if (bFireLit)
    {
        float Distance = FVector::Dist(FirePitMesh->GetComponentLocation(), Location);
        float FireRange = 300.0f;
        float FireEffect = FMath::Clamp(1.0f - (Distance / FireRange), 0.0f, 1.0f);
        BaseModifier += FireEffect * FireIntensity * 10.0f;
    }

    return BaseModifier;
}

void AArch_ShelterManager::UpdateShelterConditions(float DeltaTime)
{
    // Update current temperature based on environment and fire
    float BaseTemp = 20.0f; // Base environmental temperature
    CurrentTemperature = BaseTemp + ShelterConfig.TemperatureModifier;

    if (bFireLit)
    {
        CurrentTemperature += FireIntensity * 15.0f;
    }

    // Check for maintenance needs
    if (ShelterAge > 86400.0f) // 24 hours
    {
        float TimeSinceLastMaintenance = ShelterAge - LastMaintenanceTime;
        if (TimeSinceLastMaintenance > 172800.0f) // 48 hours
        {
            bNeedsRepair = true;
        }
    }
}

void AArch_ShelterManager::ApplyWeatherEffects(float RainIntensity, float WindStrength)
{
    // Reduce fire intensity during rain
    if (bFireLit && RainIntensity > 0.1f)
    {
        float RainEffect = RainIntensity * 0.5f;
        FireIntensity = FMath::Clamp(FireIntensity - RainEffect, 0.0f, 1.0f);
        
        if (FireIntensity <= 0.1f)
        {
            ExtinguishFire();
        }
    }

    // Strong winds can affect shelter integrity
    if (WindStrength > 0.8f && bNeedsRepair)
    {
        // Potential for shelter damage during storms
        UE_LOG(LogTemp, Warning, TEXT("Shelter experiencing stress from strong winds"));
    }
}

void AArch_ShelterManager::UpdateFireEffects(float DeltaTime)
{
    if (bFireLit)
    {
        // Fire naturally diminishes over time
        FireIntensity = FMath::Clamp(FireIntensity - (DeltaTime * 0.01f), 0.0f, 1.0f);
        
        if (FireIntensity <= 0.05f)
        {
            ExtinguishFire();
        }
    }
}

void AArch_ShelterManager::UpdateOccupancyState()
{
    // Clean up invalid actors
    CurrentOccupants.RemoveAll([](AActor* Actor) {
        return !IsValid(Actor);
    });

    bIsOccupied = CurrentOccupants.Num() > 0;
}

bool AArch_ShelterManager::IsLocationProtected(const FVector& Location)
{
    return ProtectionZone->GetCollisionShape().IsInside(ProtectionZone->GetComponentTransform().InverseTransformPosition(Location));
}

void AArch_ShelterManager::OnProtectionZoneBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (OtherActor && OtherActor->IsA<ACharacter>())
    {
        EnterShelter(OtherActor);
    }
}

void AArch_ShelterManager::OnProtectionZoneEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (OtherActor)
    {
        ExitShelter(OtherActor);
    }
}