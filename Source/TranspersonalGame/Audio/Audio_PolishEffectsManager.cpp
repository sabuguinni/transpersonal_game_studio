#include "Audio_PolishEffectsManager.h"
#include "Components/AudioComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Sound/SoundCue.h"
#include "Particles/ParticleSystemComponent.h"
#include "Particles/ParticleSystem.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialParameterCollection.h"
#include "Materials/MaterialParameterCollectionInstance.h"

UAudio_PolishEffectsManager::UAudio_PolishEffectsManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    
    // Screen shake parameters
    ShakeIntensity = 1.0f;
    ShakeDuration = 0.5f;
    ShakeRadius = 2000.0f;
    
    // Damage flash parameters
    DamageFlashDuration = 0.3f;
    DamageFlashIntensity = 0.8f;
    
    // Footstep dust parameters
    FootstepDustScale = 1.0f;
    FootstepDustLifetime = 2.0f;
    
    // Day/night cycle parameters
    DayDuration = 600.0f; // 10 minutes
    CurrentTimeOfDay = 0.5f; // Start at noon
    
    // Initialize audio components
    ProximityAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("ProximityAudio"));
    EnvironmentalAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("EnvironmentalAudio"));
    
    // Initialize particle system component
    FootstepParticleComponent = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("FootstepParticles"));
    
    bIsInitialized = false;
}

void UAudio_PolishEffectsManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeEffectsSystem();
    
    // Start day/night cycle
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(
            DayNightTimerHandle,
            this,
            &UAudio_PolishEffectsManager::UpdateDayNightCycle,
            0.1f,
            true
        );
    }
}

void UAudio_PolishEffectsManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bIsInitialized)
    {
        UpdateProximityDetection();
        UpdateDamageFlashEffect(DeltaTime);
        UpdateFootstepEffects();
    }
}

void UAudio_PolishEffectsManager::InitializeEffectsSystem()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Warning, TEXT("Audio_PolishEffectsManager: World not found during initialization"));
        return;
    }
    
    // Find directional light for day/night cycle
    DirectionalLight = nullptr;
    for (TActorIterator<ADirectionalLight> ActorIterator(World); ActorIterator; ++ActorIterator)
    {
        DirectionalLight = *ActorIterator;
        break;
    }
    
    if (!DirectionalLight)
    {
        UE_LOG(LogTemp, Warning, TEXT("Audio_PolishEffectsManager: No directional light found for day/night cycle"));
    }
    
    // Load proximity alert sound
    ProximityAlertSound = LoadObject<USoundCue>(nullptr, TEXT("/Game/Audio/ProximityAlert_Cue"));
    if (!ProximityAlertSound)
    {
        UE_LOG(LogTemp, Warning, TEXT("Audio_PolishEffectsManager: Failed to load proximity alert sound"));
    }
    
    // Load environmental sound
    EnvironmentalSound = LoadObject<USoundCue>(nullptr, TEXT("/Game/Audio/EnvironmentalSystem_Cue"));
    if (!EnvironmentalSound)
    {
        UE_LOG(LogTemp, Warning, TEXT("Audio_PolishEffectsManager: Failed to load environmental sound"));
    }
    
    // Load footstep particle system
    FootstepParticleSystem = LoadObject<UParticleSystem>(nullptr, TEXT("/Game/VFX/FootstepDust_PS"));
    if (!FootstepParticleSystem)
    {
        UE_LOG(LogTemp, Warning, TEXT("Audio_PolishEffectsManager: Failed to load footstep particle system"));
    }
    
    // Load damage flash material parameter collection
    DamageFlashMPC = LoadObject<UMaterialParameterCollection>(nullptr, TEXT("/Game/Materials/DamageFlash_MPC"));
    if (!DamageFlashMPC)
    {
        UE_LOG(LogTemp, Warning, TEXT("Audio_PolishEffectsManager: Failed to load damage flash material parameter collection"));
    }
    
    bIsInitialized = true;
    UE_LOG(LogTemp, Log, TEXT("Audio_PolishEffectsManager: Effects system initialized successfully"));
}

void UAudio_PolishEffectsManager::TriggerScreenShake(float Intensity, FVector Location)
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    APlayerController* PlayerController = World->GetFirstPlayerController();
    if (!PlayerController) return;
    
    // Calculate distance-based intensity
    APawn* PlayerPawn = PlayerController->GetPawn();
    if (PlayerPawn)
    {
        float Distance = FVector::Dist(PlayerPawn->GetActorLocation(), Location);
        float DistanceRatio = FMath::Clamp(1.0f - (Distance / ShakeRadius), 0.0f, 1.0f);
        float AdjustedIntensity = Intensity * DistanceRatio * ShakeIntensity;
        
        if (AdjustedIntensity > 0.1f)
        {
            // Create camera shake
            if (PlayerController->PlayerCameraManager)
            {
                PlayerController->PlayerCameraManager->StartCameraShake(
                    UCameraShakeBase::StaticClass(),
                    AdjustedIntensity
                );
            }
            
            UE_LOG(LogTemp, Log, TEXT("Audio_PolishEffectsManager: Screen shake triggered with intensity %f at distance %f"), 
                   AdjustedIntensity, Distance);
        }
    }
}

void UAudio_PolishEffectsManager::TriggerDamageFlash(float Intensity)
{
    if (DamageFlashMPC)
    {
        UMaterialParameterCollectionInstance* MPCInstance = GetWorld()->GetParameterCollectionInstance(DamageFlashMPC);
        if (MPCInstance)
        {
            DamageFlashCurrentIntensity = Intensity * DamageFlashIntensity;
            DamageFlashTimer = DamageFlashDuration;
            
            MPCInstance->SetScalarParameterValue(TEXT("DamageFlashIntensity"), DamageFlashCurrentIntensity);
            
            UE_LOG(LogTemp, Log, TEXT("Audio_PolishEffectsManager: Damage flash triggered with intensity %f"), 
                   DamageFlashCurrentIntensity);
        }
    }
}

void UAudio_PolishEffectsManager::SpawnFootstepDust(FVector Location, float Scale)
{
    if (FootstepParticleSystem && GetWorld())
    {
        FVector SpawnLocation = Location;
        SpawnLocation.Z += 5.0f; // Slightly above ground
        
        UGameplayStatics::SpawnEmitterAtLocation(
            GetWorld(),
            FootstepParticleSystem,
            SpawnLocation,
            FRotator::ZeroRotator,
            FVector(Scale * FootstepDustScale),
            true
        );
        
        UE_LOG(LogTemp, VeryVerbose, TEXT("Audio_PolishEffectsManager: Footstep dust spawned at location %s"), 
               *SpawnLocation.ToString());
    }
}

void UAudio_PolishEffectsManager::PlayProximityAlert(FVector ThreatLocation, float ThreatSize)
{
    if (ProximityAlertSound && ProximityAudioComponent)
    {
        ProximityAudioComponent->SetSound(ProximityAlertSound);
        ProximityAudioComponent->SetVolumeMultiplier(FMath::Clamp(ThreatSize / 8.0f, 0.5f, 2.0f));
        ProximityAudioComponent->Play();
        
        // Also trigger screen shake for large threats
        if (ThreatSize > 5.0f)
        {
            TriggerScreenShake(ThreatSize / 10.0f, ThreatLocation);
        }
        
        UE_LOG(LogTemp, Log, TEXT("Audio_PolishEffectsManager: Proximity alert played for threat size %f"), ThreatSize);
    }
}

void UAudio_PolishEffectsManager::PlayEnvironmentalAudio(const FString& BiomeType)
{
    if (EnvironmentalSound && EnvironmentalAudioComponent)
    {
        EnvironmentalAudioComponent->SetSound(EnvironmentalSound);
        
        // Adjust volume based on biome type
        float VolumeMultiplier = 1.0f;
        if (BiomeType == TEXT("Forest"))
        {
            VolumeMultiplier = 0.8f;
        }
        else if (BiomeType == TEXT("Desert"))
        {
            VolumeMultiplier = 0.6f;
        }
        else if (BiomeType == TEXT("Swamp"))
        {
            VolumeMultiplier = 1.2f;
        }
        
        EnvironmentalAudioComponent->SetVolumeMultiplier(VolumeMultiplier);
        EnvironmentalAudioComponent->Play();
        
        UE_LOG(LogTemp, Log, TEXT("Audio_PolishEffectsManager: Environmental audio started for biome %s"), *BiomeType);
    }
}

void UAudio_PolishEffectsManager::UpdateProximityDetection()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    APlayerController* PlayerController = World->GetFirstPlayerController();
    if (!PlayerController) return;
    
    APawn* PlayerPawn = PlayerController->GetPawn();
    if (!PlayerPawn) return;
    
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    
    // Check for nearby large actors (potential threats)
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(World, APawn::StaticClass(), FoundActors);
    
    for (AActor* Actor : FoundActors)
    {
        if (Actor == PlayerPawn) continue;
        
        float Distance = FVector::Dist(PlayerLocation, Actor->GetActorLocation());
        if (Distance < 1000.0f) // Within 10 meters
        {
            // Check if this is a large actor (potential dinosaur)
            UStaticMeshComponent* MeshComp = Actor->FindComponentByClass<UStaticMeshComponent>();
            if (MeshComp && MeshComp->GetStaticMesh())
            {
                FVector BoundsSize = MeshComp->GetStaticMesh()->GetBounds().BoxExtent;
                float ActorSize = BoundsSize.Size();
                
                if (ActorSize > 200.0f) // Large actor detected
                {
                    // Trigger proximity alert if not recently triggered
                    if (GetWorld()->GetTimeSeconds() - LastProximityAlertTime > 5.0f)
                    {
                        PlayProximityAlert(Actor->GetActorLocation(), ActorSize / 100.0f);
                        LastProximityAlertTime = GetWorld()->GetTimeSeconds();
                    }
                }
            }
        }
    }
}

void UAudio_PolishEffectsManager::UpdateDamageFlashEffect(float DeltaTime)
{
    if (DamageFlashTimer > 0.0f)
    {
        DamageFlashTimer -= DeltaTime;
        
        if (DamageFlashMPC)
        {
            UMaterialParameterCollectionInstance* MPCInstance = GetWorld()->GetParameterCollectionInstance(DamageFlashMPC);
            if (MPCInstance)
            {
                float Alpha = DamageFlashTimer / DamageFlashDuration;
                float CurrentIntensity = DamageFlashCurrentIntensity * Alpha;
                
                MPCInstance->SetScalarParameterValue(TEXT("DamageFlashIntensity"), CurrentIntensity);
                
                if (DamageFlashTimer <= 0.0f)
                {
                    MPCInstance->SetScalarParameterValue(TEXT("DamageFlashIntensity"), 0.0f);
                }
            }
        }
    }
}

void UAudio_PolishEffectsManager::UpdateFootstepEffects()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    APlayerController* PlayerController = World->GetFirstPlayerController();
    if (!PlayerController) return;
    
    APawn* PlayerPawn = PlayerController->GetPawn();
    if (!PlayerPawn) return;
    
    // Check if player is moving
    FVector CurrentLocation = PlayerPawn->GetActorLocation();
    float MovementDistance = FVector::Dist(CurrentLocation, LastPlayerLocation);
    
    if (MovementDistance > 100.0f) // Player moved at least 1 meter
    {
        // Spawn footstep dust
        SpawnFootstepDust(CurrentLocation, 1.0f);
        LastPlayerLocation = CurrentLocation;
    }
    
    // Also check for dinosaur movement and spawn dust for them
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(World, APawn::StaticClass(), FoundActors);
    
    for (AActor* Actor : FoundActors)
    {
        if (Actor == PlayerPawn) continue;
        
        // Check if this is a large moving actor
        UStaticMeshComponent* MeshComp = Actor->FindComponentByClass<UStaticMeshComponent>();
        if (MeshComp && MeshComp->GetStaticMesh())
        {
            FVector BoundsSize = MeshComp->GetStaticMesh()->GetBounds().BoxExtent;
            float ActorSize = BoundsSize.Size();
            
            if (ActorSize > 200.0f) // Large actor
            {
                // Spawn occasional footstep dust for large actors
                if (FMath::RandRange(0.0f, 1.0f) < 0.1f) // 10% chance per tick
                {
                    SpawnFootstepDust(Actor->GetActorLocation(), ActorSize / 200.0f);
                }
            }
        }
    }
}

void UAudio_PolishEffectsManager::UpdateDayNightCycle()
{
    if (!DirectionalLight) return;
    
    // Update time of day
    CurrentTimeOfDay += (1.0f / DayDuration) * 0.1f; // 0.1f is the timer interval
    if (CurrentTimeOfDay > 1.0f)
    {
        CurrentTimeOfDay = 0.0f;
    }
    
    // Calculate sun angle (0 = midnight, 0.5 = noon)
    float SunAngle = (CurrentTimeOfDay - 0.5f) * 180.0f; // -90 to +90 degrees
    
    // Update directional light rotation
    FRotator LightRotation = FRotator(SunAngle, 0.0f, 0.0f);
    DirectionalLight->SetActorRotation(LightRotation);
    
    // Update light intensity based on time of day
    float LightIntensity = FMath::Clamp(FMath::Cos(FMath::DegreesToRadians(SunAngle)), 0.1f, 1.0f);
    
    ULightComponent* LightComponent = DirectionalLight->FindComponentByClass<ULightComponent>();
    if (LightComponent)
    {
        LightComponent->SetIntensity(LightIntensity * 3.0f); // Base intensity of 3
    }
    
    // Update light color (warmer at sunrise/sunset)
    float ColorTemp = 6500.0f; // Default daylight temperature
    if (CurrentTimeOfDay < 0.25f || CurrentTimeOfDay > 0.75f)
    {
        ColorTemp = 3000.0f; // Warmer for sunrise/sunset
    }
    
    if (LightComponent)
    {
        LightComponent->SetTemperature(ColorTemp);
    }
}

void UAudio_PolishEffectsManager::SetDayNightSpeed(float NewDayDuration)
{
    DayDuration = FMath::Max(NewDayDuration, 60.0f); // Minimum 1 minute day
    UE_LOG(LogTemp, Log, TEXT("Audio_PolishEffectsManager: Day duration set to %f seconds"), DayDuration);
}

void UAudio_PolishEffectsManager::SetTimeOfDay(float NewTimeOfDay)
{
    CurrentTimeOfDay = FMath::Clamp(NewTimeOfDay, 0.0f, 1.0f);
    UE_LOG(LogTemp, Log, TEXT("Audio_PolishEffectsManager: Time of day set to %f"), CurrentTimeOfDay);
}

float UAudio_PolishEffectsManager::GetTimeOfDay() const
{
    return CurrentTimeOfDay;
}