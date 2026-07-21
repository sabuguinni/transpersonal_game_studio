#include "Arch_CretaceousAcousticManager.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Engine/World.h"

AArch_CretaceousAcousticManager::AArch_CretaceousAcousticManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root scene component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Create ambient audio component
    AmbientAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AmbientAudioComponent"));
    AmbientAudioComponent->SetupAttachment(RootComponent);
    AmbientAudioComponent->bAutoActivate = true;

    // Initialize default values
    CurrentReverbIntensity = 0.0f;
    CurrentEchoDelay = 0.0f;
    LastPlayerLocation = FVector::ZeroVector;

    // Initialize default acoustic zones
    FArch_AcousticZone CaveZone;
    CaveZone.ZoneName = TEXT("CaveAcoustics");
    CaveZone.ReverbIntensity = 0.8f;
    CaveZone.EchoDelay = 0.5f;
    CaveZone.SoundOcclusion = 0.7f;
    CaveZone.ZoneRadius = 2000.0f;
    AcousticZones.Add(CaveZone);

    FArch_AcousticZone ForestZone;
    ForestZone.ZoneName = TEXT("ForestAcoustics");
    ForestZone.ReverbIntensity = 0.3f;
    ForestZone.EchoDelay = 0.1f;
    ForestZone.SoundOcclusion = 0.4f;
    ForestZone.ZoneRadius = 3000.0f;
    AcousticZones.Add(ForestZone);

    // Initialize structural acoustics
    FArch_StructuralAcoustics CaveAcoustics;
    CaveAcoustics.StructureType = EArch_StructureType::Cave;
    CaveAcoustics.WallThickness = 200.0f;
    CaveAcoustics.MaterialDensity = 2.8f;
    CaveAcoustics.SoundAbsorption = 0.2f;
    CaveAcoustics.ResonanceFrequency = 150.0f;
    StructuralAcoustics.Add(CaveAcoustics);

    FArch_StructuralAcoustics RockAcoustics;
    RockAcoustics.StructureType = EArch_StructureType::RockFormation;
    RockAcoustics.WallThickness = 500.0f;
    RockAcoustics.MaterialDensity = 3.2f;
    RockAcoustics.SoundAbsorption = 0.1f;
    RockAcoustics.ResonanceFrequency = 80.0f;
    StructuralAcoustics.Add(RockAcoustics);
}

void AArch_CretaceousAcousticManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeAcousticZones();
    
    if (AmbientAudioComponent && CaveAmbienceSound)
    {
        AmbientAudioComponent->SetSound(CaveAmbienceSound);
        AmbientAudioComponent->SetVolumeMultiplier(0.5f);
    }
}

void AArch_CretaceousAcousticManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Get player location and update acoustics
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (PlayerPawn)
    {
        FVector PlayerLocation = PlayerPawn->GetActorLocation();
        
        // Only update if player has moved significantly
        if (FVector::Dist(PlayerLocation, LastPlayerLocation) > 100.0f)
        {
            UpdatePlayerAcoustics(PlayerLocation);
            LastPlayerLocation = PlayerLocation;
        }
    }
}

void AArch_CretaceousAcousticManager::InitializeAcousticZones()
{
    // Set up acoustic zones based on world geometry
    for (int32 i = 0; i < AcousticZones.Num(); i++)
    {
        FArch_AcousticZone& Zone = AcousticZones[i];
        
        // Position zones based on terrain features
        if (Zone.ZoneName.Contains(TEXT("Cave")))
        {
            Zone.ZoneCenter = FVector(0.0f, 0.0f, -500.0f); // Underground
        }
        else if (Zone.ZoneName.Contains(TEXT("Forest")))
        {
            Zone.ZoneCenter = FVector(5000.0f, 5000.0f, 100.0f); // Forest area
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("Acoustic Manager: Initialized %d acoustic zones"), AcousticZones.Num());
}

void AArch_CretaceousAcousticManager::UpdatePlayerAcoustics(FVector PlayerLocation)
{
    // Find the current acoustic zone
    FArch_AcousticZone CurrentZone = GetAcousticZoneAtLocation(PlayerLocation);
    
    // Apply reverb settings
    ApplyEnvironmentalReverb(CurrentZone.ReverbIntensity, CurrentZone.EchoDelay);
    
    // Update ambient sounds
    UpdateAmbientSounds(CurrentZone);
    
    // Calculate structural acoustics if near structures
    for (const FArch_StructuralAcoustics& Acoustics : StructuralAcoustics)
    {
        CalculateStructuralResonance(Acoustics);
    }
}

FArch_AcousticZone AArch_CretaceousAcousticManager::GetAcousticZoneAtLocation(FVector Location)
{
    // Find the closest acoustic zone
    FArch_AcousticZone ClosestZone;
    float ClosestDistance = FLT_MAX;
    
    for (const FArch_AcousticZone& Zone : AcousticZones)
    {
        float Distance = FVector::Dist(Location, Zone.ZoneCenter);
        
        if (Distance < Zone.ZoneRadius && Distance < ClosestDistance)
        {
            ClosestDistance = Distance;
            ClosestZone = Zone;
        }
    }
    
    // If no zone found, return default
    if (ClosestDistance == FLT_MAX && AcousticZones.Num() > 0)
    {
        ClosestZone = AcousticZones[0];
    }
    
    return ClosestZone;
}

void AArch_CretaceousAcousticManager::SetStructuralAcoustics(EArch_StructureType StructureType, float Absorption)
{
    // Find and update structural acoustics
    for (FArch_StructuralAcoustics& Acoustics : StructuralAcoustics)
    {
        if (Acoustics.StructureType == StructureType)
        {
            Acoustics.SoundAbsorption = FMath::Clamp(Absorption, 0.0f, 1.0f);
            break;
        }
    }
}

float AArch_CretaceousAcousticManager::CalculateSoundOcclusion(FVector SourceLocation, FVector ListenerLocation)
{
    // Perform line trace to check for occlusion
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.bTraceComplex = true;
    QueryParams.AddIgnoredActor(this);
    
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        SourceLocation,
        ListenerLocation,
        ECC_WorldStatic,
        QueryParams
    );
    
    if (bHit)
    {
        // Calculate occlusion based on material and thickness
        float Distance = FVector::Dist(SourceLocation, ListenerLocation);
        float OcclusionFactor = FMath::Clamp(Distance / 5000.0f, 0.1f, 0.9f);
        return OcclusionFactor;
    }
    
    return 0.0f; // No occlusion
}

void AArch_CretaceousAcousticManager::ApplyEnvironmentalReverb(float ReverbIntensity, float EchoDelay)
{
    CurrentReverbIntensity = ReverbIntensity;
    CurrentEchoDelay = EchoDelay;
    
    if (AmbientAudioComponent)
    {
        // Adjust volume based on reverb intensity
        float VolumeMultiplier = FMath::Lerp(0.3f, 0.8f, ReverbIntensity);
        AmbientAudioComponent->SetVolumeMultiplier(VolumeMultiplier);
        
        // Adjust pitch based on echo delay
        float PitchMultiplier = FMath::Lerp(0.9f, 1.1f, 1.0f - EchoDelay);
        AmbientAudioComponent->SetPitchMultiplier(PitchMultiplier);
    }
}

void AArch_CretaceousAcousticManager::UpdateAmbientSounds(const FArch_AcousticZone& CurrentZone)
{
    if (!AmbientAudioComponent)
        return;
    
    USoundCue* NewSound = nullptr;
    
    // Select appropriate ambient sound based on zone
    if (CurrentZone.ZoneName.Contains(TEXT("Cave")))
    {
        NewSound = CaveAmbienceSound;
    }
    else if (CurrentZone.ZoneName.Contains(TEXT("Forest")))
    {
        NewSound = ForestAmbienceSound;
    }
    else
    {
        NewSound = RockFormationSound;
    }
    
    // Change sound if different from current
    if (NewSound && AmbientAudioComponent->GetSound() != NewSound)
    {
        AmbientAudioComponent->SetSound(NewSound);
        AmbientAudioComponent->Play();
    }
}

void AArch_CretaceousAcousticManager::CalculateStructuralResonance(const FArch_StructuralAcoustics& Acoustics)
{
    // Calculate resonance based on material properties
    float ResonanceAmplitude = (Acoustics.MaterialDensity * Acoustics.WallThickness) / 1000.0f;
    float AbsorptionFactor = 1.0f - Acoustics.SoundAbsorption;
    
    // Apply calculated resonance to audio system
    if (AmbientAudioComponent)
    {
        float ResonanceMultiplier = FMath::Clamp(ResonanceAmplitude * AbsorptionFactor, 0.5f, 2.0f);
        // Additional audio processing could be applied here
    }
}