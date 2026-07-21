#include "World_LandscapeAudioIntegration.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/SceneComponent.h"
#include "Components/AudioComponent.h"
#include "Landscape/Landscape.h"
#include "Landscape/LandscapeComponent.h"
#include "Sound/SoundBase.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

DEFINE_LOG_CATEGORY(LogLandscapeAudio);

AWorld_LandscapeAudioIntegration::AWorld_LandscapeAudioIntegration()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f;

    // Create root scene component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Create primary audio component
    PrimaryAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("PrimaryAudioComponent"));
    PrimaryAudioComponent->SetupAttachment(RootComponent);
    PrimaryAudioComponent->bAutoActivate = false;

    // Create secondary audio component for blending
    SecondaryAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("SecondaryAudioComponent"));
    SecondaryAudioComponent->SetupAttachment(RootComponent);
    SecondaryAudioComponent->bAutoActivate = false;

    // Initialize default values
    AudioUpdateInterval = 0.5f;
    TerrainSampleRadius = 500.0f;
    MaxAudioZones = 10;
    HighAltitudeThreshold = 1000.0f;
    LowAltitudeThreshold = 50.0f;
    
    CurrentBiome = EWorld_BiomeType::Forest;
    CurrentTerrainHeight = 0.0f;
    CurrentAudioBlendWeight = 1.0f;
    bIsNearWater = false;
    bIsHighAltitude = false;
    LastAudioUpdateTime = 0.0f;

    // Initialize default biome volumes and pitches
    BiomeAudioVolumes.Add(EWorld_BiomeType::Forest, 0.8f);
    BiomeAudioVolumes.Add(EWorld_BiomeType::Desert, 0.6f);
    BiomeAudioVolumes.Add(EWorld_BiomeType::Tundra, 0.7f);
    BiomeAudioVolumes.Add(EWorld_BiomeType::Swamp, 0.9f);
    BiomeAudioVolumes.Add(EWorld_BiomeType::Volcanic, 1.0f);

    BiomeAudioPitches.Add(EWorld_BiomeType::Forest, 1.0f);
    BiomeAudioPitches.Add(EWorld_BiomeType::Desert, 0.9f);
    BiomeAudioPitches.Add(EWorld_BiomeType::Tundra, 0.8f);
    BiomeAudioPitches.Add(EWorld_BiomeType::Swamp, 1.1f);
    BiomeAudioPitches.Add(EWorld_BiomeType::Volcanic, 1.2f);
}

void AWorld_LandscapeAudioIntegration::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogLandscapeAudio, Log, TEXT("LandscapeAudioIntegration: BeginPlay started"));
    
    // Find landscape if not set
    if (!TargetLandscape)
    {
        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), ALandscape::StaticClass(), FoundActors);
        
        if (FoundActors.Num() > 0)
        {
            TargetLandscape = Cast<ALandscape>(FoundActors[0]);
            UE_LOG(LogLandscapeAudio, Log, TEXT("Found landscape: %s"), *TargetLandscape->GetName());
        }
    }
    
    InitializeDefaultBiomeSounds();
    UpdateAudioComponentSettings();
}

void AWorld_LandscapeAudioIntegration::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastAudioUpdateTime >= AudioUpdateInterval)
    {
        UpdateAudioBasedOnTerrain(GetActorLocation());
        LastAudioUpdateTime = CurrentTime;
    }
}

void AWorld_LandscapeAudioIntegration::InitializeLandscapeAudio(ALandscape* InLandscape)
{
    TargetLandscape = InLandscape;
    
    if (TargetLandscape)
    {
        UE_LOG(LogLandscapeAudio, Log, TEXT("Initialized landscape audio for: %s"), *TargetLandscape->GetName());
        UpdateAudioBasedOnTerrain(GetActorLocation());
    }
}

void AWorld_LandscapeAudioIntegration::UpdateAudioBasedOnTerrain(const FVector& WorldLocation)
{
    if (!TargetLandscape)
    {
        return;
    }
    
    // Get terrain height
    CurrentTerrainHeight = GetTerrainHeightAtLocation(WorldLocation);
    
    // Determine biome
    EWorld_BiomeType NewBiome = GetBiomeAtLocation(WorldLocation);
    
    // Check for biome transition
    if (NewBiome != CurrentBiome)
    {
        BlendBiomeAudio(CurrentBiome, NewBiome, 0.5f);
        CurrentBiome = NewBiome;
    }
    
    // Update height-based audio
    UpdateHeightBasedAudio(CurrentTerrainHeight);
    
    // Check for water proximity
    bIsNearWater = DetectWaterNearby(WorldLocation, TerrainSampleRadius);
    UpdateWaterAudio(bIsNearWater);
}

EWorld_BiomeType AWorld_LandscapeAudioIntegration::GetBiomeAtLocation(const FVector& WorldLocation)
{
    // Simple biome determination based on location and height
    // This would be replaced with actual biome data lookup
    
    float Height = CurrentTerrainHeight;
    float X = WorldLocation.X;
    float Y = WorldLocation.Y;
    
    // High altitude = Tundra
    if (Height > HighAltitudeThreshold)
    {
        return EWorld_BiomeType::Tundra;
    }
    
    // Low altitude + specific regions = Swamp
    if (Height < LowAltitudeThreshold && FMath::Abs(X) < 2000.0f && Y > 1000.0f)
    {
        return EWorld_BiomeType::Swamp;
    }
    
    // Volcanic region (example coordinates)
    if (X < -1500.0f && Y > 1500.0f)
    {
        return EWorld_BiomeType::Volcanic;
    }
    
    // Desert region (example coordinates)
    if (X > 2000.0f && Y < -1000.0f)
    {
        return EWorld_BiomeType::Desert;
    }
    
    // Default to forest
    return EWorld_BiomeType::Forest;
}

float AWorld_LandscapeAudioIntegration::GetTerrainHeightAtLocation(const FVector& WorldLocation)
{
    if (!TargetLandscape)
    {
        return 0.0f;
    }
    
    // Use landscape's GetHeightAtLocation if available
    FVector LandscapeLocation = TargetLandscape->GetActorLocation();
    FVector RelativeLocation = WorldLocation - LandscapeLocation;
    
    // Simple height calculation - would use actual landscape height sampling
    float BaseHeight = 100.0f;
    float HeightVariation = FMath::Sin(RelativeLocation.X * 0.001f) * FMath::Cos(RelativeLocation.Y * 0.001f) * 200.0f;
    
    return BaseHeight + HeightVariation;
}

void AWorld_LandscapeAudioIntegration::SetBiomeAudio(EWorld_BiomeType BiomeType, float BlendWeight)
{
    if (!PrimaryAudioComponent)
    {
        return;
    }
    
    USoundBase** FoundSound = BiomeAmbientSounds.Find(BiomeType);
    if (FoundSound && *FoundSound)
    {
        PrimaryAudioComponent->SetSound(*FoundSound);
        
        float* FoundVolume = BiomeAudioVolumes.Find(BiomeType);
        float* FoundPitch = BiomeAudioPitches.Find(BiomeType);
        
        float Volume = FoundVolume ? *FoundVolume * BlendWeight : 0.8f * BlendWeight;
        float Pitch = FoundPitch ? *FoundPitch : 1.0f;
        
        PrimaryAudioComponent->SetVolumeMultiplier(Volume);
        PrimaryAudioComponent->SetPitchMultiplier(Pitch);
        
        if (!PrimaryAudioComponent->IsPlaying())
        {
            PrimaryAudioComponent->Play();
        }
        
        UE_LOG(LogLandscapeAudio, Log, TEXT("Set biome audio: %d, Volume: %f, Pitch: %f"), 
               (int32)BiomeType, Volume, Pitch);
    }
}

void AWorld_LandscapeAudioIntegration::BlendBiomeAudio(EWorld_BiomeType FromBiome, EWorld_BiomeType ToBiome, float BlendAlpha)
{
    if (!PrimaryAudioComponent || !SecondaryAudioComponent)
    {
        return;
    }
    
    // Set up primary component with current biome
    SetBiomeAudio(FromBiome, 1.0f - BlendAlpha);
    
    // Set up secondary component with target biome
    USoundBase** FoundSound = BiomeAmbientSounds.Find(ToBiome);
    if (FoundSound && *FoundSound)
    {
        SecondaryAudioComponent->SetSound(*FoundSound);
        
        float* FoundVolume = BiomeAudioVolumes.Find(ToBiome);
        float* FoundPitch = BiomeAudioPitches.Find(ToBiome);
        
        float Volume = FoundVolume ? *FoundVolume * BlendAlpha : 0.8f * BlendAlpha;
        float Pitch = FoundPitch ? *FoundPitch : 1.0f;
        
        SecondaryAudioComponent->SetVolumeMultiplier(Volume);
        SecondaryAudioComponent->SetPitchMultiplier(Pitch);
        
        if (!SecondaryAudioComponent->IsPlaying())
        {
            SecondaryAudioComponent->Play();
        }
    }
    
    CurrentAudioBlendWeight = BlendAlpha;
    
    UE_LOG(LogLandscapeAudio, Log, TEXT("Blending biome audio: %d -> %d, Alpha: %f"), 
           (int32)FromBiome, (int32)ToBiome, BlendAlpha);
}

void AWorld_LandscapeAudioIntegration::UpdateHeightBasedAudio(float TerrainHeight)
{
    bool bShouldBeHighAltitude = TerrainHeight > HighAltitudeThreshold;
    
    if (bShouldBeHighAltitude != bIsHighAltitude)
    {
        bIsHighAltitude = bShouldBeHighAltitude;
        SetHighAltitudeAudio(bIsHighAltitude);
    }
}

void AWorld_LandscapeAudioIntegration::SetHighAltitudeAudio(bool bEnable, float Volume)
{
    if (HighAltitudeWindSound && SecondaryAudioComponent)
    {
        if (bEnable)
        {
            SecondaryAudioComponent->SetSound(HighAltitudeWindSound);
            SecondaryAudioComponent->SetVolumeMultiplier(Volume);
            if (!SecondaryAudioComponent->IsPlaying())
            {
                SecondaryAudioComponent->Play();
            }
        }
        else
        {
            SecondaryAudioComponent->Stop();
        }
        
        UE_LOG(LogLandscapeAudio, Log, TEXT("High altitude audio: %s"), bEnable ? TEXT("Enabled") : TEXT("Disabled"));
    }
}

void AWorld_LandscapeAudioIntegration::SetLowAltitudeAudio(bool bEnable, float Volume)
{
    if (LowAltitudeWaterSound && SecondaryAudioComponent)
    {
        if (bEnable)
        {
            SecondaryAudioComponent->SetSound(LowAltitudeWaterSound);
            SecondaryAudioComponent->SetVolumeMultiplier(Volume);
            if (!SecondaryAudioComponent->IsPlaying())
            {
                SecondaryAudioComponent->Play();
            }
        }
        else
        {
            SecondaryAudioComponent->Stop();
        }
        
        UE_LOG(LogLandscapeAudio, Log, TEXT("Low altitude audio: %s"), bEnable ? TEXT("Enabled") : TEXT("Disabled"));
    }
}

bool AWorld_LandscapeAudioIntegration::DetectWaterNearby(const FVector& WorldLocation, float SearchRadius)
{
    // Simple water detection - would use actual water volume detection
    // For now, assume water exists at low elevations in specific areas
    
    float Height = GetTerrainHeightAtLocation(WorldLocation);
    if (Height < LowAltitudeThreshold)
    {
        // Check if we're in a potential water area
        float X = WorldLocation.X;
        float Y = WorldLocation.Y;
        
        if ((FMath::Abs(X) < 1000.0f && FMath::Abs(Y) < 1000.0f) || // Central area
            (X > -2000.0f && X < -1000.0f && Y > 500.0f && Y < 1500.0f)) // Swamp area
        {
            return true;
        }
    }
    
    return false;
}

void AWorld_LandscapeAudioIntegration::UpdateWaterAudio(bool bNearWater, float WaterDistance)
{
    if (bNearWater && LowAltitudeWaterSound)
    {
        SetLowAltitudeAudio(true, 0.6f);
    }
    else if (!bNearWater && bIsNearWater) // Was near water, now not
    {
        SetLowAltitudeAudio(false);
    }
}

void AWorld_LandscapeAudioIntegration::CreateAudioZoneAtLocation(const FVector& Location, EWorld_BiomeType BiomeType, float Radius)
{
    if (ActiveAudioZones.Num() >= MaxAudioZones)
    {
        OptimizeAudioZones();
    }
    
    ActiveAudioZones.Add(Location);
    
    UE_LOG(LogLandscapeAudio, Log, TEXT("Created audio zone at: %s, Biome: %d, Radius: %f"), 
           *Location.ToString(), (int32)BiomeType, Radius);
}

void AWorld_LandscapeAudioIntegration::RemoveAudioZonesInRadius(const FVector& Location, float Radius)
{
    ActiveAudioZones.RemoveAll([Location, Radius](const FVector& ZoneLocation)
    {
        return FVector::Dist(Location, ZoneLocation) <= Radius;
    });
    
    UE_LOG(LogLandscapeAudio, Log, TEXT("Removed audio zones near: %s, Radius: %f"), 
           *Location.ToString(), Radius);
}

void AWorld_LandscapeAudioIntegration::OptimizeAudioZones()
{
    // Remove oldest zones if we have too many
    if (ActiveAudioZones.Num() > MaxAudioZones)
    {
        int32 ZonesToRemove = ActiveAudioZones.Num() - MaxAudioZones;
        ActiveAudioZones.RemoveAt(0, ZonesToRemove);
    }
    
    UE_LOG(LogLandscapeAudio, Log, TEXT("Optimized audio zones, remaining: %d"), ActiveAudioZones.Num());
}

void AWorld_LandscapeAudioIntegration::DebugShowAudioZones()
{
    if (!GetWorld())
    {
        return;
    }
    
    for (const FVector& ZoneLocation : ActiveAudioZones)
    {
        DrawDebugSphere(GetWorld(), ZoneLocation, 100.0f, 12, FColor::Green, false, 5.0f);
    }
    
    // Show current biome info
    FVector CurrentLocation = GetActorLocation();
    FString BiomeInfo = FString::Printf(TEXT("Biome: %d, Height: %.1f"), 
                                       (int32)CurrentBiome, CurrentTerrainHeight);
    
    DrawDebugString(GetWorld(), CurrentLocation + FVector(0, 0, 200), BiomeInfo, 
                   nullptr, FColor::White, 5.0f);
}

void AWorld_LandscapeAudioIntegration::TestBiomeAudioTransition()
{
    // Cycle through biomes for testing
    static int32 TestBiomeIndex = 0;
    EWorld_BiomeType TestBiomes[] = {
        EWorld_BiomeType::Forest,
        EWorld_BiomeType::Desert,
        EWorld_BiomeType::Tundra,
        EWorld_BiomeType::Swamp,
        EWorld_BiomeType::Volcanic
    };
    
    EWorld_BiomeType TestBiome = TestBiomes[TestBiomeIndex % 5];
    SetBiomeAudio(TestBiome);
    
    TestBiomeIndex++;
    
    UE_LOG(LogLandscapeAudio, Warning, TEXT("Testing biome audio: %d"), (int32)TestBiome);
}

void AWorld_LandscapeAudioIntegration::ValidateLandscapeAudioSetup()
{
    bool bIsValid = true;
    
    if (!TargetLandscape)
    {
        UE_LOG(LogLandscapeAudio, Error, TEXT("No target landscape assigned"));
        bIsValid = false;
    }
    
    if (!PrimaryAudioComponent)
    {
        UE_LOG(LogLandscapeAudio, Error, TEXT("No primary audio component"));
        bIsValid = false;
    }
    
    if (BiomeAmbientSounds.Num() == 0)
    {
        UE_LOG(LogLandscapeAudio, Warning, TEXT("No biome ambient sounds configured"));
    }
    
    UE_LOG(LogLandscapeAudio, Log, TEXT("Landscape audio validation: %s"), 
           bIsValid ? TEXT("PASSED") : TEXT("FAILED"));
}

void AWorld_LandscapeAudioIntegration::InitializeDefaultBiomeSounds()
{
    // This would load actual sound assets
    // For now, just log that initialization happened
    UE_LOG(LogLandscapeAudio, Log, TEXT("Initialized default biome sounds"));
}

void AWorld_LandscapeAudioIntegration::UpdateAudioComponentSettings()
{
    if (PrimaryAudioComponent)
    {
        PrimaryAudioComponent->bAutoActivate = false;
        PrimaryAudioComponent->SetVolumeMultiplier(0.8f);
    }
    
    if (SecondaryAudioComponent)
    {
        SecondaryAudioComponent->bAutoActivate = false;
        SecondaryAudioComponent->SetVolumeMultiplier(0.5f);
    }
}

float AWorld_LandscapeAudioIntegration::CalculateBiomeBlendWeight(const FVector& Location, EWorld_BiomeType BiomeType)
{
    // Simple distance-based blend calculation
    // Would be replaced with proper biome boundary detection
    return 1.0f;
}

void AWorld_LandscapeAudioIntegration::CleanupInactiveAudioZones()
{
    // Remove zones that are too far from current location
    FVector CurrentLocation = GetActorLocation();
    float MaxDistance = TerrainSampleRadius * 3.0f;
    
    ActiveAudioZones.RemoveAll([CurrentLocation, MaxDistance](const FVector& ZoneLocation)
    {
        return FVector::Dist(CurrentLocation, ZoneLocation) > MaxDistance;
    });
}